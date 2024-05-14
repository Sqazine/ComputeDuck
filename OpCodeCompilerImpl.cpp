#include "OpCodeCompilerImpl.h"
#include "Object.h"
#include "BuiltinManager.h"
#include <limits>

#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#elif __APPLE__
#endif

constexpr int16_t INVALID_OPCODE = std::numeric_limits<int16_t>::max();

OpCodeCompilerImpl::OpCodeCompilerImpl()
    : m_SymbolTable(nullptr)
{
}

OpCodeCompilerImpl::~OpCodeCompilerImpl()
{
    if (m_SymbolTable)
        SAFE_DELETE(m_SymbolTable);
}

Chunk *OpCodeCompilerImpl::Compile(const std::vector<Stmt *> &stmts)
{
    ResetStatus();

    for (const auto &stmt : stmts)
        CompileStmt(stmt);

    return new Chunk(CurOpCodes(), m_Constants);
}

void OpCodeCompilerImpl::ResetStatus()
{
    std::vector<Value>().swap(m_Constants);
    std::vector<OpCodes>().swap(m_Scopes);
    m_Scopes.emplace_back(OpCodes()); // set a default opcodes

    if (m_SymbolTable)
        SAFE_DELETE(m_SymbolTable);
    m_SymbolTable = new SymbolTable();

    m_BuiltinNames.clear();

    for (int32_t i = 0; i < BuiltinManager::GetInstance()->m_BuiltinNames.size(); ++i)
    {
        m_BuiltinNames.emplace_back(BuiltinManager::GetInstance()->m_BuiltinNames[i]);
        m_SymbolTable->DefineBuiltin(BuiltinManager::GetInstance()->m_BuiltinNames[i], i);
    }
}

void OpCodeCompilerImpl::CompileStmt(Stmt *stmt)
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
        break;
    }
}

void OpCodeCompilerImpl::CompileExprStmt(ExprStmt *stmt)
{
    CompileExpr(stmt->expr);
}

void OpCodeCompilerImpl::CompileIfStmt(IfStmt *stmt)
{
    CompileExpr(stmt->condition);
    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(INVALID_OPCODE);

    CompileStmt(stmt->thenBranch);

    Emit(OP_JUMP);
    auto jumpAddress = Emit(INVALID_OPCODE);

    ModifyOpCode(jumpIfFalseAddress, (int16_t)CurOpCodes().size() - 1);

    if (stmt->elseBranch)
        CompileStmt(stmt->elseBranch);

    ModifyOpCode(jumpAddress, (int16_t)CurOpCodes().size() - 1);
}

void OpCodeCompilerImpl::CompileScopeStmt(ScopeStmt *stmt)
{
    EnterScope();
    Emit(OP_SP_OFFSET);
    auto idx = Emit(0);

    for (const auto &s : stmt->stmts)
        CompileStmt(s);

    auto localVarCount = m_SymbolTable->definitionCount;

    if (localVarCount == 0)
    {
        m_Scopes.back().erase(m_Scopes.back().begin() + idx);
        m_Scopes.back().erase(m_Scopes.back().begin() + (idx - 1));
    }
    else
    {
        CurOpCodes()[idx] = localVarCount;
        Emit(OP_SP_OFFSET);
        Emit(-localVarCount);
    }

    ExitScope();
}

void OpCodeCompilerImpl::CompileWhileStmt(WhileStmt *stmt)
{
    auto jumpAddress = (int32_t)CurOpCodes().size() - 1;
    CompileExpr(stmt->condition);

    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(INVALID_OPCODE);

    CompileStmt(stmt->body);

    Emit(OP_JUMP);
    Emit(jumpAddress);

    ModifyOpCode(jumpIfFalseAddress, (int16_t)CurOpCodes().size() - 1);
}

void OpCodeCompilerImpl::CompileReturnStmt(ReturnStmt *stmt)
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

void OpCodeCompilerImpl::CompileStructStmt(StructStmt *stmt)
{
    auto symbol = m_SymbolTable->Define(stmt->name, true);

    m_Scopes.emplace_back(OpCodes());

    CompileStructExpr(stmt->body);

    auto opCodes = m_Scopes.back();
    m_Scopes.pop_back();

    opCodes.emplace_back(OP_RETURN);
    opCodes.emplace_back(1);

    auto fn = new FunctionObject(opCodes);

    EmitConstant(AddConstant(fn));

    StoreSymbol(symbol);
}

void OpCodeCompilerImpl::CompileExpr(Expr *expr, const RWState &state)
{
    switch (expr->type)
    {
    case AstType::NUM:
        CompileNumExpr((NumExpr *)expr);
        break;
    case AstType::STR:
        CompileStrExpr((StrExpr *)expr);
        break;
    case AstType::BOOL:
        CompileBoolExpr((BoolExpr *)expr);
        break;
    case AstType::NIL:
        CompileNilExpr((NilExpr *)expr);
        break;
    case AstType::IDENTIFIER:
        CompileIdentifierExpr((IdentifierExpr *)expr, state);
        break;
    case AstType::GROUP:
        CompileGroupExpr((GroupExpr *)expr);
        break;
    case AstType::ARRAY:
        CompileArrayExpr((ArrayExpr *)expr);
        break;
    case AstType::INDEX:
        CompileIndexExpr((IndexExpr *)expr);
        break;
    case AstType::PREFIX:
        CompilePrefixExpr((PrefixExpr *)expr);
        break;
    case AstType::INFIX:
        CompileInfixExpr((InfixExpr *)expr);
        break;
    case AstType::FUNCTION_CALL:
        CompileFunctionCallExpr((FunctionCallExpr *)expr);
        break;
    case AstType::STRUCT_CALL:
        CompileStructCallExpr((StructCallExpr *)expr, state);
        break;
    case AstType::REF:
        CompileRefExpr((RefExpr *)expr);
        break;
    case AstType::FUNCTION:
        CompileFunctionExpr((FunctionExpr *)expr);
        break;
    case AstType::STRUCT:
        CompileStructExpr((StructExpr *)expr);
        break;
    case AstType::DLL_IMPORT:
        CompileDllImportExpr((DllImportExpr *)expr);
        break;
    default:
        break;
    }
}

void OpCodeCompilerImpl::CompileInfixExpr(InfixExpr *expr)
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

void OpCodeCompilerImpl::CompileNumExpr(NumExpr *expr)
{
    EmitConstant(AddConstant(expr->value));
}

void OpCodeCompilerImpl::CompileBoolExpr(BoolExpr *expr)
{
    if (expr->value)
        EmitConstant(AddConstant(true));
    else
        EmitConstant(AddConstant(false));
}

void OpCodeCompilerImpl::CompilePrefixExpr(PrefixExpr *expr)
{
    CompileExpr(expr->right);
    if (expr->op == "-")
        Emit(OP_MINUS);
    else if (expr->op == "not")
        Emit(OP_NOT);
    else if (expr->op == "~")
        Emit(OP_BIT_NOT);
    else
        ASSERT("Unrecognized prefix op");
}

void OpCodeCompilerImpl::CompileStrExpr(StrExpr *expr)
{
    EmitConstant(AddConstant(new StrObject(expr->value)));
}

void OpCodeCompilerImpl::CompileNilExpr(NilExpr *expr)
{
    EmitConstant(AddConstant(Value()));
}

void OpCodeCompilerImpl::CompileGroupExpr(GroupExpr *expr)
{
    CompileExpr(expr->expr);
}

void OpCodeCompilerImpl::CompileArrayExpr(ArrayExpr *expr)
{
    for (const auto &e : expr->elements)
        CompileExpr(e);

    Emit(OP_ARRAY);
    Emit(expr->elements.size());
}

void OpCodeCompilerImpl::CompileIndexExpr(IndexExpr *expr)
{
    CompileExpr(expr->ds);
    CompileExpr(expr->index);
    Emit(OP_INDEX);
}

void OpCodeCompilerImpl::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state)
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

void OpCodeCompilerImpl::CompileFunctionExpr(FunctionExpr *expr)
{
    EnterScope();

    m_Scopes.emplace_back(OpCodes());

    for (const auto &param : expr->parameters)
        m_SymbolTable->Define(param->literal);

    for (const auto &s : expr->body->stmts)
        CompileStmt(s);

    auto localVarCount = m_SymbolTable->definitionCount;

    auto opCodes = m_Scopes.back();
    m_Scopes.pop_back();

    ExitScope();

    // for non return  or empty stmt in function scope:add a return to return nothing
    if (opCodes.empty() || opCodes[opCodes.size() - 2] != OP_RETURN)
    {
        opCodes.emplace_back(OP_RETURN);
        opCodes.emplace_back(0);
    }

    auto fn = new FunctionObject(opCodes, localVarCount, expr->parameters.size());

    EmitConstant(AddConstant(fn));
}

void OpCodeCompilerImpl::CompileFunctionCallExpr(FunctionCallExpr *expr)
{
    CompileExpr(expr->name);

    for (const auto &argu : expr->arguments)
        CompileExpr(argu);

    Emit(OP_FUNCTION_CALL);
    Emit(expr->arguments.size());
}

void OpCodeCompilerImpl::CompileStructCallExpr(StructCallExpr *expr, const RWState &state)
{
    if (expr->callMember->type == AstType::FUNCTION_CALL && state == RWState::WRITE)
        ASSERT("Cannot assign to a struct's function call expr");

    CompileExpr(expr->callee);

    if (expr->callMember->type == AstType::IDENTIFIER)
        EmitConstant(AddConstant(new StrObject(((IdentifierExpr *)expr->callMember)->literal)));
    else if (expr->callMember->type == AstType::FUNCTION_CALL)
        EmitConstant(AddConstant(new StrObject(((IdentifierExpr *)((FunctionCallExpr *)expr->callMember)->name)->literal)));

    if (state == RWState::READ)
    {
        Emit(OP_GET_STRUCT);

        if (expr->callMember->type == AstType::FUNCTION_CALL)
        {
            auto funcCall = (FunctionCallExpr *)expr->callMember;
            for (const auto &argu : funcCall->arguments)
                CompileExpr(argu);

            Emit(OP_FUNCTION_CALL);
            Emit(funcCall->arguments.size());
        }
    }
    else
        Emit(OP_SET_STRUCT);
}

void OpCodeCompilerImpl::CompileRefExpr(RefExpr *expr)
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
            Emit(symbol.isInUpScope);
            Emit(symbol.scopeDepth);
            Emit(symbol.index);
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
            Emit(symbol.isInUpScope);
            Emit(symbol.scopeDepth);
            Emit(symbol.index);
            break;
        default:
            break;
        }
    }
}

void OpCodeCompilerImpl::CompileStructExpr(StructExpr *expr)
{
    for (const auto &[k, v] : expr->members)
    {
        CompileExpr(v);
        EmitConstant(AddConstant(new StrObject(k->literal)));
    }

    Emit(OP_STRUCT);
    Emit(expr->members.size());
}

void OpCodeCompilerImpl::CompileDllImportExpr(DllImportExpr *expr)
{
    typedef void (*RegFn)();
    auto dllpath = expr->dllPath;

    if (dllpath.find(".") == std::string::npos) // no file suffix
    {
#ifdef _WIN32
        dllpath = "./lib" + dllpath + ".dll";
#elif __linux__
        dllpath = "./lib" + dllpath + ".so";
#elif __APPLE__
        dllpath += ".dylib";
#endif
    }

#ifdef _WIN32

    HINSTANCE hInstance;
    hInstance = LoadLibrary(dllpath.c_str());
    if (!hInstance)
        ASSERT("Failed to load dll library:%s", dllpath.c_str());

    RegFn RegisterBuiltins = (RegFn)(GetProcAddress(hInstance, "RegisterBuiltins"));

    RegisterBuiltins();
#elif __linux__
    void *handle;
    double (*cosine)(double);
    char *error;

    handle = dlopen(dllpath.c_str(), RTLD_LAZY);
    if (!handle)
        ASSERT("Failed to load dll library:%s", dllpath.c_str());

    RegFn RegisterBuiltins = (RegFn)(dlsym(handle, "RegisterBuiltins"));
    RegisterBuiltins();
#elif __APPLE__
#error "DllImport Expr not implemented yet on APPLE Platform!"
#endif

    std::vector<std::string> newAddedBuiltinNames;

    for (const auto &name : BuiltinManager::GetInstance()->m_BuiltinNames)
    {
        bool found = false;
        for (const auto &recName : m_BuiltinNames)
        {
            if (name == recName)
            {
                found = true;
                break;
            }
        }
        if (found == false)
            newAddedBuiltinNames.emplace_back(name);
    }

    int32_t legacyBuiltinCount = (int32_t)m_BuiltinNames.size();
    for (int32_t i = 0; i < newAddedBuiltinNames.size(); ++i)
    {
        m_BuiltinNames.emplace_back(newAddedBuiltinNames[i]);
        m_SymbolTable->DefineBuiltin(newAddedBuiltinNames[i], legacyBuiltinCount + i);
    }
}

void OpCodeCompilerImpl::EnterScope()
{
    m_SymbolTable = new SymbolTable(m_SymbolTable);
}
void OpCodeCompilerImpl::ExitScope()
{
    m_SymbolTable = m_SymbolTable->enclosing;
}

OpCodes &OpCodeCompilerImpl::CurOpCodes()
{
    return m_Scopes.back();
}

uint32_t OpCodeCompilerImpl::Emit(int16_t opcode)
{
    CurOpCodes().emplace_back(opcode);
    return CurOpCodes().size() - 1;
}

uint32_t OpCodeCompilerImpl::EmitConstant(uint32_t pos)
{
    Emit(OP_CONSTANT);
    Emit(pos);
    return CurOpCodes().size() - 1;
}

void OpCodeCompilerImpl::ModifyOpCode(uint32_t pos, int16_t opcode)
{
    CurOpCodes()[pos] = opcode;
}

uint32_t OpCodeCompilerImpl::AddConstant(const Value &value)
{
    m_Constants.emplace_back(value);
    return m_Constants.size() - 1;
}

void OpCodeCompilerImpl::LoadSymbol(const Symbol &symbol)
{
    switch (symbol.scope)
    {
    case SymbolScope::GLOBAL:
        Emit(OP_GET_GLOBAL);
        Emit(symbol.index);
        break;
    case SymbolScope::LOCAL:
        Emit(OP_GET_LOCAL);
        Emit(symbol.isInUpScope);
        Emit(symbol.scopeDepth);
        Emit(symbol.index);
        break;
    case SymbolScope::BUILTIN:
        Emit(OP_GET_BUILTIN);
        Emit(symbol.index);
        break;
    default:
        break;
    }

    if (symbol.isStructSymbol)
    {
        Emit(OP_FUNCTION_CALL);
        Emit(0);
    }
}

void OpCodeCompilerImpl::StoreSymbol(const Symbol &symbol)
{
    switch (symbol.scope)
    {
    case SymbolScope::GLOBAL:
        Emit(OP_SET_GLOBAL);
        Emit(symbol.index);
        break;
    case SymbolScope::LOCAL:
        Emit(OP_SET_LOCAL);
        Emit(symbol.isInUpScope);
        Emit(symbol.scopeDepth);
        Emit(symbol.index);
        break;
    default:
        break;
    }
}