#pragma once
#include <vector>
#include "Chunk.h"
#include "Ast.h"
#include "Value.h"
#include "Config.h"

enum class SymbolScope
{
    GLOBAL,
    LOCAL,
    BUILTIN,
};

struct Symbol
{
    Symbol()
        : scope(SymbolScope::GLOBAL), index(0), scopeDepth(0), isInUpScope(0), isStructSymbol(false)
    {
    }

    Symbol(const std::string &name, const SymbolScope &scope, int32_t index, int32_t scopeDepth = 0, bool isStructSymbol = false)
        : name(name), scope(scope), index(index), isStructSymbol(isStructSymbol), scopeDepth(scopeDepth), isInUpScope(0)
    {
    }

    std::string name;
    bool isStructSymbol;
    SymbolScope scope;
    int32_t index;
    int32_t scopeDepth;
    int32_t isInUpScope;
};

struct SymbolTable
{
    SymbolTable()
        : enclosing(nullptr), definitionCount(0), scopeDepth(0)
    {
    }

    SymbolTable(SymbolTable *enclosing)
        : enclosing(enclosing), definitionCount(0)
    {
        scopeDepth = enclosing->scopeDepth + 1;
    }

    ~SymbolTable()
    {
        auto p = enclosing;
        while (p)
        {
            auto q = p->enclosing;
            SAFE_DELETE(p);
            p = q;
        }
    }

    Symbol Define(const std::string &name, bool isStructSymbol = false)
    {
        auto symbol = Symbol(name, SymbolScope::GLOBAL, definitionCount, scopeDepth, isStructSymbol);

        if (!enclosing)
            symbol.scope = SymbolScope::GLOBAL;
        else
            symbol.scope = SymbolScope::LOCAL;

        if (symbolMaps.find(name) != symbolMaps.end())
            ASSERT("Redefined variable:(%s) in current context.", name.data());

        symbolMaps[name] = symbol;
        definitionCount++;
        return symbol;
    }

    Symbol DefineBuiltin(const std::string &name, int32_t index)
    {
        auto symbol = Symbol(name, SymbolScope::BUILTIN, index, scopeDepth);
        symbolMaps[name] = symbol;
        return symbol;
    }

    bool Resolve(const std::string &name, Symbol &symbol)
    {
        auto iter = symbolMaps.find(name);
        if (iter != symbolMaps.end())
        {
            symbol = iter->second;
            return true;
        }
        else if (enclosing)
        {
            bool isFound = enclosing->Resolve(name, symbol);
            if (!isFound)
                return false;
            if (symbol.scope == SymbolScope::GLOBAL || symbol.scope == SymbolScope::BUILTIN)
                return true;

            symbol.isInUpScope = 1;

            symbolMaps[symbol.name] = symbol;
            return true;
        }

        return false;
    }

    SymbolTable *enclosing;

    std::unordered_map<std::string, Symbol> symbolMaps;
    int32_t definitionCount;

    int32_t scopeDepth;
};

class COMPUTE_DUCK_API Compiler
{

public:
    Compiler();
    ~Compiler();

    Chunk *Compile(const std::vector<Stmt *> &stmts, bool isLineInterpret = false);

    void ResetStatus();

private:
    enum class RWState //read write state
    {
        READ,
        WRITE,
    };

    void CompileStmt(Stmt *stmt);
    void CompileExprStmt(ExprStmt *stmt);
    void CompileIfStmt(IfStmt *stmt);
    void CompileScopeStmt(ScopeStmt *stmt);
    void CompileWhileStmt(WhileStmt *stmt);
    void CompileReturnStmt(ReturnStmt *stmt);
    void CompileStructStmt(StructStmt *stmt);

    void CompileExpr(Expr *expr, const RWState &state = RWState::READ);
    void CompileInfixExpr(InfixExpr *expr);
    void CompileNumExpr(NumExpr *expr);
    void CompileBoolExpr(BoolExpr *expr);
    void CompilePrefixExpr(PrefixExpr *expr);
    void CompileStrExpr(StrExpr *expr);
    void CompileNilExpr(NilExpr *expr);
    void CompileGroupExpr(GroupExpr *expr);
    void CompileArrayExpr(ArrayExpr *expr);
    void CompileIndexExpr(IndexExpr *expr);
    void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state);
    void CompileFunctionExpr(FunctionExpr *expr);
    void CompileFunctionCallExpr(FunctionCallExpr *expr);
    void CompileStructCallExpr(StructCallExpr *expr, const RWState &state = RWState::READ);
    void CompileRefExpr(RefExpr *expr);
    void CompileAnonyStructExpr(AnonyStructExpr *expr);
    void CompileDllImportExpr(DllImportExpr *expr);

    void EnterScope();
    void ExitScope();

    OpCodes &CurOpCodes();

    uint32_t Emit(int32_t opcode);
    uint32_t EmitConstant(uint32_t pos);

    void ModifyOpCode(uint32_t pos, int32_t opcode);

    uint32_t AddConstant(const Value &value);

    void LoadSymbol(const Symbol &symbol);

    std::vector<Value> m_Constants;

    std::vector<OpCodes> m_Scopes;

    SymbolTable *m_SymbolTable;

    //record builtin object names,for finding new added symbol names
    std::vector<std::string> m_BuiltinNames;
};