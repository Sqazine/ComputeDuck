#pragma once
#include <vector>
#include "Chunk.h"
#include "Ast.h"
#include "Value.h"

class COMPUTEDUCK_API Compiler
{
public:
    Compiler() = default;
    ~Compiler() = default;

    const Chunk &Compile(const std::vector<Stmt *> &stmts);

private:
    void ResetStatus();

    void CompileStmt(Stmt *stmt);
    void CompileExprStmt(ExprStmt *stmt);

    void CompileExpr(Expr *expr);
    void CompileBinaryExpr(BinaryExpr *expr);
    void CompileNumExpr(NumExpr *expr);
    void CompileUnaryExpr(UnaryExpr *expr);
    void CompileGroupExpr(GroupExpr *expr);

    Chunk &CurChunk();

    uint32_t Emit(int16_t opcode);
    uint32_t EmitConstant(const Value &value);

    Chunk m_Chunk;
};