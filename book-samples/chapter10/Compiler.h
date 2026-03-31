#pragma once
#include <vector>
#include "Chunk.h"
#include "Ast.h"
#include "Value.h"
#include "Object.h"
#include "SymbolTable.h"

class COMPUTEDUCK_API Compiler
{
public:
    Compiler() = default;
    ~Compiler();
    // ++ 修改内容
    // const Chunk &Compile(const std::vector<Stmt *> &stmts);
    FunctionObject *Compile(const std::vector<Stmt *> &stmts);
    // -- 修改内容
private:
    enum class RWState
    {
        READ,
        WRITE,
    };

    void ResetStatus();

    void CompileStmt(Stmt *stmt);
    void CompileExprStmt(ExprStmt *stmt);
    void CompileScopeStmt(ScopeStmt *stmt);
    void CompilePrintStmt(PrintStmt *stmt);

    void CompileIfStmt(IfStmt *stmt);
    // ++ 新增内容
    void CompileReturnStmt(ReturnStmt *stmt);
    // -- 新增内容

    void CompileWhileStmt(WhileStmt *stmt);

    void CompileExpr(Expr *expr, const RWState &state = RWState::READ);
    void CompileBinaryExpr(BinaryExpr *expr);
    void CompileNumExpr(NumExpr *expr);
    void CompileBoolExpr(BoolExpr *expr);
    void CompileUnaryExpr(UnaryExpr *expr);
    void CompileStrExpr(StrExpr *expr);
    void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state);
    void CompileNilExpr(NilExpr *expr);
    void CompileGroupExpr(GroupExpr *expr);
    void CompileArrayExpr(ArrayExpr *expr);
    void CompileIndexExpr(IndexExpr *expr, const RWState &state);
    // ++ 新增内容
    void CompileFunctionExpr(FunctionExpr *expr);
    void CompileFunctionCallExpr(FunctionCallExpr *expr);
    // -- 新增内容

    Chunk &CurChunk();

    uint32_t Emit(int16_t opcode);
    uint32_t EmitConstant(const Value &value);

    void DefineSymbol(const Symbol &symbol);
    void LoadSymbol(const Symbol &symbol);
    void StoreSymbol(const Symbol &symbol);

    void ModifyOpCode(uint32_t pos, int16_t opcode);

    // ++ 修改内容
    // Chunk m_Chunk;
    std::vector<Chunk> m_ScopeChunks;
    // -- 修改内容

    SymbolTable *m_SymbolTable{nullptr};
};