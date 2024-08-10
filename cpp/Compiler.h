#pragma once
#include <vector>
#include "Chunk.h"
#include "Ast.h"
#include "Value.h"
#include "Object.h"
#include "SymbolTable.h"

class COMPUTE_DUCK_API Compiler
{

public:
    Compiler() = default;
    ~Compiler();

    Value Compile(const std::vector<Stmt *> &stmts);

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
    void CompileIndexExpr(IndexExpr *expr, const RWState& state);
    void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state);
    void CompileFunctionExpr(FunctionExpr *expr);
    void CompileFunctionCallExpr(FunctionCallExpr *expr);
    void CompileStructCallExpr(StructCallExpr *expr, const RWState &state);
    void CompileRefExpr(RefExpr *expr);
    void CompileStructExpr(StructExpr *expr);
    void CompileDllImportExpr(DllImportExpr *expr);

    void EnterScope();
    void ExitScope();

    Chunk &CurChunk();

    uint32_t Emit(int16_t opcode);
    uint32_t EmitConstant(const Value &value);

    void ModifyOpCode(uint32_t pos, int16_t opcode);

    void LoadSymbol(const Symbol &symbol);
    void StoreSymbol(const Symbol &symbol);

    std::vector<Chunk> m_ScopeChunks;

    SymbolTable *m_SymbolTable{nullptr};
};