#include "Compiler.h"
#include <limits>
#include "Allocator.h"
#include "Object.h"
#include "BuiltinManager.h"
constexpr int16_t INVALID_OPCODE = std::numeric_limits<int16_t>::max();

Compiler::~Compiler()
{
    SAFE_DELETE(m_SymbolTable);
}

FunctionObject *Compiler::Compile(const std::vector<Stmt *> &stmts)
{
    ResetStatus();

    for (const auto &stmt : stmts)
        CompileStmt(stmt);

    auto mainFn = ALLOCATE_OBJECT(FunctionObject, CurChunk(), m_SymbolTable->GetLocalVarCount());

    SAFE_DELETE(m_SymbolTable);
    return mainFn;
}

void Compiler::ResetStatus()
{
    std::vector<Chunk>().swap(m_ScopeChunks);
    m_ScopeChunks.emplace_back(Chunk()); // set a default opCodeList

    SAFE_DELETE(m_SymbolTable);
    m_SymbolTable = new SymbolTable();

    DefineBuiltin();
}

void Compiler::CompileStmt(Stmt *stmt)
{
    switch (stmt->type)
    {
    case AstType::EXPR:
        CompileExprStmt((ExprStmt *)stmt);
        break;
    case AstType::IF:
        CompileIfStmt((IfStmt *)stmt);
        break;
    case AstType::RETURN:
        CompileReturnStmt((ReturnStmt *)stmt);
        break;
    case AstType::WHILE:
        CompileWhileStmt((WhileStmt *)stmt);
        break;
    case AstType::SCOPE:
        CompileScopeStmt((ScopeStmt *)stmt);
        break;
    // ++ 新增内容
    case AstType::STRUCT:
        CompileStructStmt((StructStmt *)stmt);
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

void Compiler::CompileScopeStmt(ScopeStmt *stmt)
{
    m_SymbolTable->EnterScope();
    for (const auto &s : stmt->stmts)
        CompileStmt(s);
    m_SymbolTable->ExitScope();
}

void Compiler::CompileIfStmt(IfStmt *stmt)
{
    CompileExpr(stmt->condition);
    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(INVALID_OPCODE);

    CompileStmt(stmt->thenBranch);

    uint32_t jumpAddress = INVALID_OPCODE;

    if (stmt->elseBranch)
    {
        Emit(OP_JUMP);
        jumpAddress = Emit(INVALID_OPCODE);
    }

    ModifyOpCode(jumpIfFalseAddress, (int16_t)CurChunk().opCodeList.size());

    if (stmt->elseBranch)
    {
        CompileStmt(stmt->elseBranch);
        ModifyOpCode(jumpAddress, (int16_t)CurChunk().opCodeList.size());
    }
}

void Compiler::CompileWhileStmt(WhileStmt *stmt)
{
    auto jumpAddress = (int32_t)CurChunk().opCodeList.size();
    CompileExpr(stmt->condition);

    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(INVALID_OPCODE);

    CompileStmt(stmt->body);

    Emit(OP_JUMP);
    Emit(jumpAddress);

    ModifyOpCode(jumpIfFalseAddress, (int16_t)CurChunk().opCodeList.size());
}

void Compiler::CompileReturnStmt(ReturnStmt *stmt)
{
    if (stmt->expr)
    {
        CompileExpr(stmt->expr);
        Emit(OP_RETURN);
        Emit(1);
    }
    else
    {
        Emit(OP_RETURN);
        Emit(0);
    }
}

// ++ 新增内容
void Compiler::CompileStructStmt(StructStmt *stmt)
{
    auto symbol = m_SymbolTable->Define(stmt->name, true);

    m_ScopeChunks.emplace_back(Chunk());

    CompileStructExpr(stmt->body);

    auto chunk = m_ScopeChunks.back();
    m_ScopeChunks.pop_back();

    chunk.opCodeList.emplace_back(OP_RETURN);
    chunk.opCodeList.emplace_back(1);

    auto fn = ALLOCATE_OBJECT(FunctionObject, chunk);

    EmitClosure(fn);

    StoreSymbol(symbol);
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
    case AstType::FUNCTION_CALL:
        return CompileFunctionCallExpr((FunctionCallExpr *)expr);
    case AstType::FUNCTION:
        return CompileFunctionExpr((FunctionExpr *)expr);
    // ++ 新增内容
    case AstType::STRUCT_CALL:
        return CompileStructCallExpr((StructCallExpr *)expr, state);
    case AstType::STRUCT:
        return CompileStructExpr((StructExpr *)expr);
    // -- 新增内容
    default:
        ASSERT("Unknown expr.");
    }
}

void Compiler::CompileBinaryExpr(BinaryExpr *expr)
{
    if (expr->op == "=")
    {
        if (expr->left->type == AstType::IDENTIFIER && expr->right->type == AstType::FUNCTION)
            m_SymbolTable->Define(((IdentifierExpr *)expr->left)->literal);

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
    EmitConstant(ALLOCATE_OBJECT(StrObject, expr->value.c_str()));
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

void Compiler::CompileFunctionExpr(FunctionExpr *expr)
{
    m_SymbolTable = new SymbolTable(m_SymbolTable);

    m_ScopeChunks.emplace_back(Chunk());

    for (const auto &param : expr->parameters)
        m_SymbolTable->Define(param->literal);

    for (const auto &s : expr->body->stmts)
        CompileStmt(s);

    auto localVarCount = m_SymbolTable->GetLocalVarCount();
    auto parameterCount = static_cast<uint8_t>(expr->parameters.size());

    auto chunk = m_ScopeChunks.back();
    m_ScopeChunks.pop_back();

    // for non return  or empty stmt in function scope:add a return to return nothing
    if (chunk.opCodeList.empty() || chunk.opCodeList[chunk.opCodeList.size() - 2] != OP_RETURN)
    {
        chunk.opCodeList.emplace_back(OP_RETURN);
        chunk.opCodeList.emplace_back(0);
    }

    auto fn = ALLOCATE_OBJECT(FunctionObject, chunk, localVarCount, parameterCount);

    EmitClosure(fn);

    auto tmpTable = m_SymbolTable;
    m_SymbolTable = m_SymbolTable->GetUpper();
    SAFE_DELETE(tmpTable);
}

void Compiler::CompileFunctionCallExpr(FunctionCallExpr *expr)
{
    CompileExpr(expr->name);

    for (const auto &argu : expr->arguments)
        CompileExpr(argu);

    Emit(OP_FUNCTION_CALL);
    Emit(static_cast<int16_t>(expr->arguments.size()));
}

// ++ 新增内容
void Compiler::CompileStructExpr(StructExpr *expr)
{
    for (const auto &[k, v] : expr->members)
    {
        CompileExpr(v);
        EmitConstant(ALLOCATE_OBJECT(StrObject, k->literal.c_str()));
    }

    Emit(OP_STRUCT);
    Emit(static_cast<int16_t>(expr->members.size()));
}

void Compiler::CompileStructCallExpr(StructCallExpr *expr, const RWState &state)
{
    if (expr->callMember->type == AstType::FUNCTION_CALL && state == RWState::WRITE)
        ASSERT("Cannot assign to a struct's function call expr");

    CompileExpr(expr->callee);

    if (expr->callMember->type == AstType::IDENTIFIER)
        EmitConstant(ALLOCATE_OBJECT(StrObject, ((IdentifierExpr *)expr->callMember)->literal.c_str()));
    else if (expr->callMember->type == AstType::FUNCTION_CALL)
        EmitConstant(ALLOCATE_OBJECT(StrObject, ((IdentifierExpr *)((FunctionCallExpr *)expr->callMember)->name)->literal.data()));

    if (state == RWState::READ)
    {
        Emit(OP_GET_STRUCT);

        if (expr->callMember->type == AstType::FUNCTION_CALL)
        {
            auto funcCall = (FunctionCallExpr *)expr->callMember;
            for (const auto &argu : funcCall->arguments)
                CompileExpr(argu);

            Emit(OP_FUNCTION_CALL);
            Emit(static_cast<int16_t>(funcCall->arguments.size()));
        }
    }
    else
        Emit(OP_SET_STRUCT);
}
// -- 新增内容

Chunk &Compiler::CurChunk()
{
    return m_ScopeChunks.back();
}

uint32_t Compiler::Emit(int16_t opcode)
{
    CurChunk().opCodeList.emplace_back(opcode);
    return static_cast<uint32_t>(CurChunk().opCodeList.size() - 1);
}

uint32_t Compiler::EmitConstant(const Value &value)
{
    auto pos = AddConstant(value);

    Emit(OP_CONSTANT);
    Emit(pos);
    return static_cast<uint32_t>(CurChunk().opCodeList.size() - 1);
}

uint16_t Compiler::AddConstant(const Value &value)
{
    CurChunk().constants.emplace_back(value);
    auto pos = static_cast<int16_t>(CurChunk().constants.size() - 1);
    return pos;
}

uint32_t Compiler::EmitClosure(FunctionObject *fn)
{
    auto upvalueCount = m_SymbolTable->GetUpvalueCount();

    uint32_t pos = AddConstant(fn);
    Emit(OP_CLOSURE);
    Emit(pos);
    Emit(upvalueCount);

    for (uint8_t i = 0; i < upvalueCount; ++i)
    {
        auto upvalue = m_SymbolTable->GetUpvalueList()[i];
        Emit(upvalue.index);
        Emit(upvalue.scopeDepth);
    }

    return static_cast<uint32_t>(CurChunk().opCodeList.size() - 1);
}

void Compiler::DefineSymbol(const Symbol &symbol)
{
    switch (symbol.scope)
    {
    case SymbolScope::GLOBAL:
        Emit(OP_DEF_GLOBAL);
        Emit(symbol.index);
        break;
    case SymbolScope::LOCAL:
        Emit(OP_DEF_LOCAL);
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
    case SymbolScope::LOCAL:
        Emit(OP_GET_LOCAL);
        Emit(symbol.index);
        break;
    case SymbolScope::UPVALUE:
        Emit(OP_GET_UPVALUE);
        Emit(symbol.upvalueIndex);
        break;
    case SymbolScope::BUILTIN:
    {
        CurChunk().constants.emplace_back(ALLOCATE_OBJECT(StrObject, symbol.name.data()));
        auto pos = static_cast<int16_t>(CurChunk().constants.size() - 1);
        Emit(OP_GET_BUILTIN);
        Emit(pos);
        break;
    }
    default:
        break;
    }

    // ++ 新增内容
    if (symbol.isStructSymbol)
    {
        Emit(OP_FUNCTION_CALL);
        Emit(0);
    }
    // -- 新增内容
}

void Compiler::StoreSymbol(const Symbol &symbol)
{
    switch (symbol.scope)
    {
    case SymbolScope::GLOBAL:
        Emit(OP_SET_GLOBAL);
        Emit(symbol.index);
        break;
    case SymbolScope::LOCAL:
        Emit(OP_SET_LOCAL);
        Emit(symbol.index);
        break;
    case SymbolScope::UPVALUE:
        Emit(OP_SET_UPVALUE);
        Emit(symbol.upvalueIndex);
        break;
    default:
        break;
    }
}

void Compiler::ModifyOpCode(uint32_t pos, int16_t opcode)
{
    CurChunk().opCodeList[pos] = opcode;
}

void Compiler::DefineBuiltin()
{
    HashTable &builtinTable = BuiltinManager::GetInstance()->GetBuiltinObjectTable();
    for (size_t i = 0; i < builtinTable.GetCapacity(); ++i)
    {
        if (builtinTable.IsValid(i))
        {
            auto key = builtinTable.GetEntries()[i].key;
            m_SymbolTable->DefineBuiltin(key->value);
        }
    }
}