#include "Compiler.h"
#include <limits>
#include "Allocator.h"
#include "Object.h"

Compiler::~Compiler()
{
    SAFE_DELETE(m_SymbolTable);
}

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

    SAFE_DELETE(m_SymbolTable);

    m_SymbolTable = new SymbolTable();
}

void Compiler::CompileStmt(Stmt *stmt)
{
    switch (stmt->type)
    {
    case AstType::EXPR:
        CompileExprStmt((ExprStmt *)stmt);
        break;
        // ++ 新增内容
    case AstType::PRINT:
        CompilePrintStmt((PrintStmt *)stmt);
        break;
        // -- 新增内容
    default:
        ASSERT("Unknown stmt")
    }
}

void Compiler::CompileExprStmt(ExprStmt *stmt)
{
    CompileExpr(stmt->expr);
}

// ++ 新增内容
void Compiler::CompilePrintStmt(PrintStmt *stmt)
{
    CompileExpr(stmt->expr);
    Emit(OP_PRINT);
}
// -- 新增内容

void Compiler::CompileExpr(Expr *expr, const RWState &state)
{
    switch (expr->type)
    {
    case AstType::NUM:
        return CompileNumExpr((NumExpr *)expr);
    case AstType::STR:
        return CompileStrExpr((StrExpr *)expr);
    case AstType::BOOL:
        return CompileBoolExpr((BoolExpr *)expr);
    case AstType::NIL:
        return CompileNilExpr((NilExpr *)expr);
    case AstType::IDENTIFIER:
        return CompileIdentifierExpr((IdentifierExpr *)expr, state);
    case AstType::GROUP:
        return CompileGroupExpr((GroupExpr *)expr);
    case AstType::ARRAY:
        return CompileArrayExpr((ArrayExpr *)expr);
    case AstType::INDEX:
        return CompileIndexExpr((IndexExpr *)expr, state);
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
    if (expr->op == "=")
    {
        CompileExpr(expr->right);
        CompileExpr(expr->left, RWState::WRITE);
    }
    else
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
        else if (expr->op == ">")
            Emit(OP_GREATER);
        else if (expr->op == "<")
            Emit(OP_LESS);
        else if (expr->op == "&")
            Emit(OP_BIT_AND);
        else if (expr->op == "|")
            Emit(OP_BIT_OR);
        else if (expr->op == "^")
            Emit(OP_BIT_XOR);
        else if (expr->op == ">=")
        {
            Emit(OP_LESS);
            Emit(OP_NOT);
        }
        else if (expr->op == "<=")
        {
            Emit(OP_GREATER);
            Emit(OP_NOT);
        }
        else if (expr->op == "==")
            Emit(OP_EQUAL);
        else if (expr->op == "!=")
        {
            Emit(OP_EQUAL);
            Emit(OP_NOT);
        }
        else if (expr->op == "and")
            Emit(OP_AND);
        else if (expr->op == "or")
            Emit(OP_OR);
    }
}

void Compiler::CompileNumExpr(NumExpr *expr)
{
    EmitConstant(expr->value);
}

void Compiler::CompileBoolExpr(BoolExpr *expr)
{
    if (expr->value)
        EmitConstant(true);
    else
        EmitConstant(false);
}

void Compiler::CompileUnaryExpr(UnaryExpr *expr)
{
    CompileExpr(expr->right);

    if (expr->op == "-")
        Emit(OP_MINUS);
    else if (expr->op == "~")
        Emit(OP_BIT_NOT);
    else if (expr->op == "not")
        Emit(OP_NOT);
    else
        ASSERT("Unrecognized unary type.");
}

void Compiler::CompileStrExpr(StrExpr *expr)
{
    EmitConstant(Allocator::GetInstance()->CreateObject<StrObject>(expr->value.c_str()));
}

void Compiler::CompileNilExpr(NilExpr *expr)
{
    EmitConstant(Value());
}

void Compiler::CompileGroupExpr(GroupExpr *expr)
{
    return CompileExpr(expr->expr);
}

void Compiler::CompileArrayExpr(ArrayExpr *expr)
{
    for (const auto &e : expr->elements)
        CompileExpr(e);

    Emit(OP_ARRAY);
    Emit(static_cast<int16_t>(expr->elements.size()));
}

void Compiler::CompileIndexExpr(IndexExpr *expr, const RWState &state)
{
    CompileExpr(expr->ds);
    CompileExpr(expr->index);
    if (state == RWState::WRITE)
        Emit(OP_SET_INDEX);
    else
        Emit(OP_GET_INDEX);
}

void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state)
{
    Symbol symbol;
    bool isFound = m_SymbolTable->Resolve(expr->literal, symbol);
    if (state == RWState::READ)
    {
        if (!isFound)
            ASSERT("Undefined variable:%s", expr->Stringify().c_str());
        LoadSymbol(symbol);
    }
    else
    {
        if (!isFound)
        {
            symbol = m_SymbolTable->Define(expr->literal);
            DefineSymbol(symbol);
        }
        else
            StoreSymbol(symbol);
    }
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

void Compiler::DefineSymbol(const Symbol &symbol)
{
    switch (symbol.scope)
    {
    case SymbolScope::GLOBAL:
        Emit(OP_DEF_GLOBAL);
        Emit(symbol.index);
        break;
    default:
        break;
    }
}

void Compiler::LoadSymbol(const Symbol &symbol)
{
    switch (symbol.scope)
    {
    case SymbolScope::GLOBAL:
        Emit(OP_GET_GLOBAL);
        Emit(symbol.index);
        break;
    default:
        break;
    }
}

void Compiler::StoreSymbol(const Symbol &symbol)
{
    switch (symbol.scope)
    {
    case SymbolScope::GLOBAL:
        Emit(OP_SET_GLOBAL);
        Emit(symbol.index);
        break;
    default:
        break;
    }
}