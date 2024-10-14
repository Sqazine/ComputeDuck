#include "Compiler.h"
#include <limits>

const Chunk &Compiler::Compile(const std::vector<Stmt *> &stmts)
{
    ResetStatus();

    for (const auto &stmt : stmts)
        CompileStmt(stmt);

    return CurChunk();
}

void Compiler::ResetStatus()
{
    m_Chunk = Chunk();
}

void Compiler::CompileStmt(Stmt *stmt)
{
    switch (stmt->type)
    {
    case AstType::EXPR:
        CompileExprStmt((ExprStmt *)stmt);
        break;
    default:
        ASSERT("Unknown stmt")
    }
}

void Compiler::CompileExprStmt(ExprStmt *stmt)
{
    CompileExpr(stmt->expr);
}

void Compiler::CompileExpr(Expr *expr)
{
    switch (expr->type)
    {
    case AstType::NUM:
        return CompileNumExpr((NumExpr *)expr);
    case AstType::GROUP:
        return CompileGroupExpr((GroupExpr *)expr);
    case AstType::UNARY:
        return CompileUnaryExpr((UnaryExpr *)expr);
    case AstType::BINARY:
        return CompileBinaryExpr((BinaryExpr *)expr);
    default:
        ASSERT("Unknown expr.");
    }
}

void Compiler::CompileBinaryExpr(BinaryExpr *expr)
{

    CompileExpr(expr->right);
    CompileExpr(expr->left);

    if (expr->op == "+")
        Emit(OP_ADD);
    else if (expr->op == "-")
        Emit(OP_SUB);
    else if (expr->op == "*")
        Emit(OP_MUL);
    else if (expr->op == "/")
        Emit(OP_DIV);
    else if (expr->op == "&")
        Emit(OP_BIT_AND);
    else if (expr->op == "|")
        Emit(OP_BIT_OR);
    else if (expr->op == "^")
        Emit(OP_BIT_XOR);
}

void Compiler::CompileNumExpr(NumExpr *expr)
{
    EmitConstant(expr->value);
}

void Compiler::CompileUnaryExpr(UnaryExpr *expr)
{
    CompileExpr(expr->right);

    if (expr->op == "-")
        Emit(OP_MINUS);
    else if (expr->op == "~")
        Emit(OP_BIT_NOT);
    else
        ASSERT("Unrecognized unary type.");
}

void Compiler::CompileGroupExpr(GroupExpr *expr)
{
    return CompileExpr(expr->expr);
}

Chunk &Compiler::CurChunk()
{
    return m_Chunk;
}

uint32_t Compiler::Emit(int16_t opcode)
{
    CurChunk().opCodes.emplace_back(opcode);
    return static_cast<uint32_t>(CurChunk().opCodes.size() - 1);
}

uint32_t Compiler::EmitConstant(const Value &value)
{
    CurChunk().constants.emplace_back(value);
    auto pos = static_cast<int16_t>(CurChunk().constants.size() - 1);

    Emit(OP_CONSTANT);
    Emit(pos);
    return static_cast<uint32_t>(CurChunk().opCodes.size() - 1);
}
