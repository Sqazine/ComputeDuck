#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APFixedPoint.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "Config.h"
#include "Ast.h"
#include "Config.h"
#include "Utils.h"

enum class LLVMSymbolScope
{
    GLOBAL,
    LOCAL,
    BUILTIN,
};

struct LLVMSymbol
{
    LLVMSymbol()
        : scope(LLVMSymbolScope::GLOBAL), alloc(nullptr), scopeDepth(0), isInUpScope(0)
    {
    }

    LLVMSymbol(const std::string &name, const LLVMSymbolScope &scope, llvm::AllocaInst * alloc, int32_t scopeDepth = 0)
        : name(name), scope(scope), alloc(alloc), scopeDepth(scopeDepth), isInUpScope(0)
    {
    }

    std::string name;
    LLVMSymbolScope scope;
    llvm::AllocaInst *alloc;
    int32_t scopeDepth;
    int32_t isInUpScope;
};

struct LLVMSymbolTable
{
    LLVMSymbolTable()
        : enclosing(nullptr), scopeDepth(0)
    {
    }

    LLVMSymbolTable(LLVMSymbolTable *enclosing)
        : enclosing(enclosing)
    {
        scopeDepth = enclosing->scopeDepth + 1;
    }

    ~LLVMSymbolTable()
    {
        auto p = enclosing;
        while (p)
        {
            auto q = p->enclosing;
            SAFE_DELETE(p);
            p = q;
        }
    }

    LLVMSymbol Define(const std::string &name, llvm::AllocaInst *a = nullptr)
    {
        auto symbol = LLVMSymbol(name, LLVMSymbolScope::GLOBAL, a, scopeDepth);

        if (!enclosing)
            symbol.scope = LLVMSymbolScope::GLOBAL;
        else
            symbol.scope = LLVMSymbolScope::LOCAL;

        if (symbolMaps.find(name) != symbolMaps.end())
            ASSERT("Redefined variable:(%s) in current context.", name.data());

        symbolMaps[name] = symbol;
        return symbol;
    }

	LLVMSymbol DefineBuiltin(const std::string& name)
	{
		auto symbol = LLVMSymbol(name, LLVMSymbolScope::BUILTIN,nullptr, scopeDepth);
		symbolMaps[name] = symbol;
		return symbol;
	}


    void Set(const std::string& name, llvm::AllocaInst* a)
    {
		symbolMaps[name].alloc = a;
    }

    bool Resolve(const std::string &name, LLVMSymbol &symbol)
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
            if (symbol.scope == LLVMSymbolScope::GLOBAL)
                return true;

            symbol.isInUpScope = 1;

            symbolMaps[symbol.name] = symbol;
            return true;
        }

        return false;
    }

    LLVMSymbolTable *enclosing;
    std::unordered_map<std::string, LLVMSymbol> symbolMaps;
    int32_t scopeDepth;
};

class COMPUTE_DUCK_API LLVMCompiler
{
public:
    LLVMCompiler();
    ~LLVMCompiler();

    llvm::Function *Compile(const std::vector<Stmt *> &stmts);

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

    llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function*  fn,llvm::StringRef name,llvm::Type* type);

    void Push(llvm::Value *v);
    llvm::Value *Peek(int32_t distance);
    llvm::Value *Pop();

    llvm::Function* GetCurFunction();
    void AddFunction(llvm::Function* fn);
    llvm::Function* PeekFunction(int32_t distance);
    llvm::Function* PopFunction();

    void EnterScope();
    void ExitScope();

    std::vector<llvm::Value *> m_ValueStack;
    std::vector<llvm::Function*> m_FunctionStack;

    LLVMSymbolTable *m_SymbolTable;

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;
};