#pragma once
#include <vector>
#include "Chunk.h"
#include "Ast.h"
#include "Value.h"
#include "SymbolTable.h"

class COMPUTE_DUCK_API OpCodeCompilerImpl
{

public:
    OpCodeCompilerImpl();
    ~OpCodeCompilerImpl();

    Chunk *Compile(const std::vector<Stmt *> &stmts);
private:
    void ResetStatus();

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
    void CompileStructExpr(StructExpr *expr);
    void CompileDllImportExpr(DllImportExpr *expr);

    void EnterScope();
    void ExitScope();

    OpCodes &CurOpCodes();

    uint32_t Emit(int16_t opcode);
    uint32_t EmitConstant(uint32_t pos);

    void ModifyOpCode(uint32_t pos, int16_t opcode);

    uint32_t AddConstant(const Value &value);

    void LoadSymbol(const Symbol &symbol);
    void StoreSymbol(const Symbol &symbol);

    std::vector<Value> m_Constants;

    std::vector<OpCodes> m_Scopes;

    SymbolTable *m_SymbolTable;

    // record builtin object names,for finding new added symbol names
    std::vector<std::string> m_BuiltinNames;
};