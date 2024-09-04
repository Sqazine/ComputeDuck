#include "Compiler.h"
#include <limits>
#include "Object.h"
#include "BuiltinManager.h"
#include "Allocator.h"

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

    auto mainFn= Allocator::GetInstance()->CreateObject<FunctionObject>(CurChunk());
    PUSH(mainFn);

    Allocator::GetInstance()->ResetStack();

    return mainFn;
}

void Compiler::ResetStatus()
{
    std::vector<Chunk>().swap(m_ScopeChunks);
    m_ScopeChunks.emplace_back(Chunk()); // set a default opcodes

    SAFE_DELETE(m_SymbolTable);

    m_SymbolTable = new SymbolTable();

    for (const auto &[k, v] : BuiltinManager::GetInstance()->GetBuiltinObjectList())
        m_SymbolTable->DefineBuiltin(k);
}

void Compiler::CompileStmt(Stmt *stmt)
{
    switch (stmt->type)
    {
    case AstType::RETURN:
        CompileReturnStmt((ReturnStmt *)stmt);
        break;
    case AstType::EXPR:
        CompileExprStmt((ExprStmt *)stmt);
        break;
    case AstType::SCOPE:
        CompileScopeStmt((ScopeStmt *)stmt);
        break;
    case AstType::IF:
        CompileIfStmt((IfStmt *)stmt);
        break;
    case AstType::WHILE:
        CompileWhileStmt((WhileStmt *)stmt);
        break;
    case AstType::STRUCT:
        CompileStructStmt((StructStmt *)stmt);
        break;
    default:
        ASSERT("Unknown stmt")
    }
}

void Compiler::CompileExprStmt(ExprStmt *stmt)
{
    CompileExpr(stmt->expr);
}

void Compiler::CompileIfStmt(IfStmt *stmt)
{
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Emit(OP_JUMP_START);
    Emit(JumpMode::IF);
#endif

    CompileExpr(stmt->condition);
    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(INVALID_OPCODE);
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Emit(JumpMode::IF);
#endif

    CompileStmt(stmt->thenBranch);

    Emit(OP_JUMP);
    auto jumpAddress = Emit(INVALID_OPCODE);
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Emit(JumpMode::IF);
#endif

    ModifyOpCode(jumpIfFalseAddress, (int16_t)CurChunk().opCodes.size());

    if (stmt->elseBranch)
        CompileStmt(stmt->elseBranch);

    ModifyOpCode(jumpAddress, (int16_t)CurChunk().opCodes.size());
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Emit(OP_JUMP_END);
#endif
}

void Compiler::CompileScopeStmt(ScopeStmt *stmt)
{
    Emit(OP_SP_OFFSET);
    auto idx = Emit(0);

    for (const auto &s : stmt->stmts)
        CompileStmt(s);

    auto localVarCount = m_SymbolTable->GetDefinitionCount();

    CurChunk().opCodes[idx] = localVarCount;
    Emit(OP_SP_OFFSET);
    Emit(-localVarCount);
}

void Compiler::CompileWhileStmt(WhileStmt *stmt)
{
    EnterScope();

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Emit(OP_JUMP_START);
    Emit(JumpMode::WHILE);
#endif

    auto jumpAddress = (int32_t)CurChunk().opCodes.size();
    CompileExpr(stmt->condition);

    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(INVALID_OPCODE);
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Emit(JumpMode::WHILE);
#endif

    CompileStmt(stmt->body);

    Emit(OP_JUMP);
    Emit(jumpAddress);
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Emit(JumpMode::WHILE);
#endif

    ModifyOpCode(jumpIfFalseAddress, (int16_t)CurChunk().opCodes.size());

    ExitScope();
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

void Compiler::CompileStructStmt(StructStmt *stmt)
{
    auto symbol = m_SymbolTable->Define(stmt->name, true);

    m_ScopeChunks.emplace_back(Chunk());

    CompileStructExpr(stmt->body);

    auto chunk = m_ScopeChunks.back();
    m_ScopeChunks.pop_back();

    chunk.opCodes.emplace_back(OP_RETURN);
    chunk.opCodes.emplace_back(1);

    auto fn = Allocator::GetInstance()->CreateObject<FunctionObject>(chunk);

    EmitConstant(fn);

    StoreSymbol(symbol);
}

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
    case AstType::PREFIX:
        return CompilePrefixExpr((PrefixExpr *)expr);
    case AstType::INFIX:
        return CompileInfixExpr((InfixExpr *)expr);
    case AstType::FUNCTION_CALL:
        return CompileFunctionCallExpr((FunctionCallExpr *)expr);
    case AstType::STRUCT_CALL:
        return CompileStructCallExpr((StructCallExpr *)expr, state);
    case AstType::REF:
        return CompileRefExpr((RefExpr *)expr);
    case AstType::FUNCTION:
        return CompileFunctionExpr((FunctionExpr *)expr);
    case AstType::STRUCT:
        return CompileStructExpr((StructExpr *)expr);
    case AstType::DLL_IMPORT:
        return CompileDllImportExpr((DllImportExpr *)expr);
    default:
        ASSERT("Unknown expr.");
    }
}

void Compiler::CompileInfixExpr(InfixExpr *expr)
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

void Compiler::CompilePrefixExpr(PrefixExpr *expr)
{
    CompileExpr(expr->right);

    if (expr->op == "-")
        Emit(OP_MINUS);
    else if (expr->op == "~")
        Emit(OP_BIT_NOT);


    else if (expr->op == "not")
        Emit(OP_NOT);

    else
        ASSERT("Unrecognized prefix type.");
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
            symbol = m_SymbolTable->Define(expr->literal);
        StoreSymbol(symbol);
    }
}

void Compiler::CompileFunctionExpr(FunctionExpr *expr)
{
    EnterScope();

    m_ScopeChunks.emplace_back(Chunk());

    for (const auto &param : expr->parameters)
        m_SymbolTable->Define(param->literal);

    for (const auto &s : expr->body->stmts)
        CompileStmt(s);

    auto localVarCount = m_SymbolTable->GetDefinitionCount();

    auto chunk = m_ScopeChunks.back();
    m_ScopeChunks.pop_back();

    ExitScope();

    // for non return  or empty stmt in function scope:add a return to return nothing
    if (chunk.opCodes.empty() || chunk.opCodes[chunk.opCodes.size() - 2] != OP_RETURN)
    {
        chunk.opCodes.emplace_back(OP_RETURN);
        chunk.opCodes.emplace_back(0);
    }

    auto fn = Allocator::GetInstance()->CreateObject<FunctionObject>(chunk, localVarCount, static_cast<uint8_t>(expr->parameters.size()));

    EmitConstant(fn);
}

void Compiler::CompileFunctionCallExpr(FunctionCallExpr *expr)
{
    CompileExpr(expr->name);

    for (const auto &argu : expr->arguments)
        CompileExpr(argu);

    Emit(OP_FUNCTION_CALL);
    Emit(static_cast<int16_t>(expr->arguments.size()));
}

void Compiler::CompileStructCallExpr(StructCallExpr *expr, const RWState &state)
{
    if (expr->callMember->type == AstType::FUNCTION_CALL && state == RWState::WRITE)
        ASSERT("Cannot assign to a struct's function call expr");

    CompileExpr(expr->callee);

    if (expr->callMember->type == AstType::IDENTIFIER)
        EmitConstant(Allocator::GetInstance()->CreateObject<StrObject>(((IdentifierExpr *)expr->callMember)->literal.c_str()));
    else if (expr->callMember->type == AstType::FUNCTION_CALL)
        EmitConstant(Allocator::GetInstance()->CreateObject<StrObject>(((IdentifierExpr *)((FunctionCallExpr *)expr->callMember)->name)->literal.data()));

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

void Compiler::CompileRefExpr(RefExpr *expr)
{
    Symbol symbol;
    if (expr->refExpr->type == AstType::INDEX)
    {
        CompileExpr(((IndexExpr *)expr->refExpr)->index);
        bool isFound = m_SymbolTable->Resolve(((IndexExpr *)expr->refExpr)->ds->Stringify(), symbol);
        if (!isFound)
            ASSERT("Undefined variable:%s", expr->Stringify().c_str());

        switch (symbol.scope)
        {
        case SymbolScope::GLOBAL:
            Emit(OP_REF_INDEX_GLOBAL);
            Emit(symbol.index);
            break;
        case SymbolScope::LOCAL:
            Emit(OP_REF_INDEX_LOCAL);
            Emit(symbol.scopeDepth);
            Emit(symbol.index);
            Emit(symbol.isUpValue);
            break;
        default:
            break;
        }
    }
    else
    {
        bool isFound = m_SymbolTable->Resolve(expr->refExpr->Stringify(), symbol);
        if (!isFound)
            ASSERT("Undefined variable:%s", expr->Stringify().c_str());

        switch (symbol.scope)
        {
        case SymbolScope::GLOBAL:
            Emit(OP_REF_GLOBAL);
            Emit(symbol.index);
            break;
        case SymbolScope::LOCAL:
            Emit(OP_REF_LOCAL);
            Emit(symbol.scopeDepth);
            Emit(symbol.index);
            Emit(symbol.isUpValue);
            break;
        default:
            break;
        }
    }
}

void Compiler::CompileStructExpr(StructExpr *expr)
{
    for (const auto &[k, v] : expr->members)
    {
        CompileExpr(v);
        EmitConstant(Allocator::GetInstance()->CreateObject<StrObject>(k->literal.c_str()));
    }

    Emit(OP_STRUCT);
    Emit(static_cast<int16_t>(expr->members.size()));
}

void Compiler::CompileDllImportExpr(DllImportExpr *expr)
{
    auto dllpath = expr->dllPath;

    RegisterDLLs(dllpath);

    for (const auto &[k, v] : BuiltinManager::GetInstance()->GetBuiltinObjectList())
        m_SymbolTable->DefineBuiltin(k);

    EmitConstant(Allocator::GetInstance()->CreateObject<StrObject>(dllpath.c_str()));
    Emit(OP_DLL_IMPORT);
}

void Compiler::EnterScope()
{
    m_SymbolTable = new SymbolTable(m_SymbolTable);
}
void Compiler::ExitScope()
{
    m_SymbolTable = m_SymbolTable->GetEnclosing();
}

Chunk &Compiler::CurChunk()
{
    return m_ScopeChunks.back();
}

uint32_t Compiler::Emit(int16_t opcode)
{
    CurChunk().opCodes.emplace_back(opcode);
    return static_cast<uint32_t>(CurChunk().opCodes.size() - 1);
}

uint32_t Compiler::EmitConstant(const Value &value)
{
    PUSH(value);// push value to stack for avoiding GC while compiler running

    CurChunk().constants.emplace_back(value);
    auto pos = static_cast<int16_t>(CurChunk().constants.size() - 1);

    Emit(OP_CONSTANT);
    Emit(pos);
    return static_cast<uint32_t>(CurChunk().opCodes.size() - 1);
}

void Compiler::ModifyOpCode(uint32_t pos, int16_t opcode)
{
    CurChunk().opCodes[pos] = opcode;
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
        Emit(symbol.scopeDepth);
        Emit(symbol.index);
        Emit(symbol.isUpValue);
        break;
    case SymbolScope::BUILTIN:
    {
        CurChunk().constants.emplace_back(Allocator::GetInstance()->CreateObject<StrObject>(symbol.name.data()));
        auto pos = static_cast<int16_t>(CurChunk().constants.size() - 1);
        Emit(OP_GET_BUILTIN);
        Emit(pos);
        break;
    }
    default:
        break;
    }

    if (symbol.isStructSymbol)
    {
        Emit(OP_FUNCTION_CALL);
        Emit(0);
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
    case SymbolScope::LOCAL:
        Emit(OP_SET_LOCAL);
        Emit(symbol.scopeDepth);
        Emit(symbol.index);
        Emit(symbol.isUpValue);
        break;
    default:
        break;
    }
}