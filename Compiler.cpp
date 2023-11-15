#include "Compiler.h"
#include "Object.h"
#include "BuiltinManager.h"

#ifdef _WIN32
#include <Windows.h>
#endif

Compiler::Compiler()
    : m_SymbolTable(nullptr)
{
    ResetStatus();
}
Compiler::~Compiler()
{
}

Chunk Compiler::Compile(const std::vector<Stmt *> &stmts, bool isLineInterpret)
{
    for (const auto &stmt : stmts)
        CompileStmt(stmt);

    if (!isLineInterpret)
    {
        // tag as program exit
        Emit(OP_RETURN);
        Emit(0);
    }

    return Chunk(CurOpCodes(), m_Constants);
}

void Compiler::ResetStatus()
{
    std::vector<Value>().swap(m_Constants);
    std::vector<OpCodes>().swap(m_Scopes);
    m_Scopes.emplace_back(OpCodes()); // set a default opcodes

    if (m_SymbolTable)
        delete m_SymbolTable;
    m_SymbolTable = new SymbolTable();

    for (int32_t i = 0; i < BuiltinManager::GetInstance()->m_BuiltinFunctionNames.size(); ++i)
    {
        m_BuiltinFunctionNames.emplace_back(BuiltinManager::GetInstance()->m_BuiltinFunctionNames[i]);
        m_SymbolTable->DefineBuiltinFunction(BuiltinManager::GetInstance()->m_BuiltinFunctionNames[i], i);
    }

    for (int32_t i = 0; i < BuiltinManager::GetInstance()->m_BuiltinVariableNames.size(); ++i)
    {
        m_BuiltinVariableNames.emplace_back(BuiltinManager::GetInstance()->m_BuiltinVariableNames[i]);
        m_SymbolTable->DefineBuiltinVariable(BuiltinManager::GetInstance()->m_BuiltinVariableNames[i], i);
    }
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
        break;
    }
}

void Compiler::CompileExprStmt(ExprStmt *stmt)
{
    CompileExpr(stmt->expr);
}

void Compiler::CompileIfStmt(IfStmt *stmt)
{
    CompileExpr(stmt->condition);
    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(65536); // 65536 just a temp address

    CompileStmt(stmt->thenBranch);

    Emit(OP_JUMP);
    auto jumpAddress = Emit(65536);

    ModifyOpCode(jumpIfFalseAddress, (int32_t)CurOpCodes().size() - 1);

    if (stmt->elseBranch)
        CompileStmt(stmt->elseBranch);

    ModifyOpCode(jumpAddress, (int32_t)CurOpCodes().size() - 1);
}

void Compiler::CompileScopeStmt(ScopeStmt *stmt)
{
    EnterScope();
    Emit(OP_SP_OFFSET);
    auto idx = Emit(0);

    for (const auto &s : stmt->stmts)
        CompileStmt(s);

    auto localVarCount = m_SymbolTable->definitionCount;
    CurOpCodes()[idx] = localVarCount;

    Emit(OP_SP_OFFSET);
    Emit(-localVarCount);

    ExitScope();
}

void Compiler::CompileWhileStmt(WhileStmt *stmt)
{
    auto jumpAddress = (int32_t)CurOpCodes().size() - 1;
    CompileExpr(stmt->condition);

    Emit(OP_JUMP_IF_FALSE);
    auto jumpIfFalseAddress = Emit(65536); // 65536 just a temp address

    CompileStmt(stmt->body);

    Emit(OP_JUMP);
    Emit(jumpAddress);

    ModifyOpCode(jumpIfFalseAddress, (int32_t)CurOpCodes().size() - 1);
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

    EnterScope();
    m_Scopes.emplace_back(OpCodes());

    for (int32_t i = stmt->members.size() - 1; i >= 0; --i)
    {
        CompileExpr(stmt->members[i].second);
        EmitConstant(AddConstant(new StrObject(stmt->members[i].first->literal)));
    }

    auto localVarCount = m_SymbolTable->definitionCount;

    Emit(OP_STRUCT);
    Emit((int32_t)stmt->members.size());

    ExitScope();

    auto opCodes = m_Scopes.back();
    m_Scopes.pop_back();

    opCodes.emplace_back(OP_RETURN);
    opCodes.emplace_back(1);

    auto fn = new FunctionObject(opCodes, localVarCount);

    EmitConstant(AddConstant(fn));

    if (symbol.scope == SymbolScope::GLOBAL)
        Emit(OP_SET_GLOBAL);
    else
    {
        Emit(OP_SET_LOCAL);
        Emit(symbol.isInUpScope);
        Emit(symbol.scopeDepth);
    }
    Emit(symbol.index);
}

void Compiler::CompileExpr(Expr *expr, const RWState &state)
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
    case AstType::ANONY_STRUCT:
        CompileAnonyStructExpr((AnonyStructExpr *)expr);
        break;
    case AstType::DLL_IMPORT:
        CompileDllImportExpr((DllImportExpr *)expr);
        break;
    default:
        break;
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
    auto value = Value(expr->value);
    auto pos = AddConstant(value);
    EmitConstant(pos);
}

void Compiler::CompileBoolExpr(BoolExpr *expr)
{
    if (expr->value)
        EmitConstant(AddConstant(Value(true)));
    else
        EmitConstant(AddConstant(Value(false)));
}

void Compiler::CompilePrefixExpr(PrefixExpr *expr)
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

void Compiler::CompileStrExpr(StrExpr *expr)
{
    auto value = Value(new StrObject(expr->value));
    auto pos = AddConstant(value);
    EmitConstant(pos);
}

void Compiler::CompileNilExpr(NilExpr *expr)
{
    EmitConstant(AddConstant(Value()));
}

void Compiler::CompileGroupExpr(GroupExpr *expr)
{
    CompileExpr(expr->expr);
}

void Compiler::CompileArrayExpr(ArrayExpr *expr)
{
    for (const auto &e : expr->elements)
        CompileExpr(e);

    Emit(OP_ARRAY);
    Emit((uint32_t)expr->elements.size());
}

void Compiler::CompileIndexExpr(IndexExpr *expr)
{
    CompileExpr(expr->ds);
    CompileExpr(expr->index);
    Emit(OP_INDEX);
}

void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state)
{
    Symbol symbol;
    bool isFound = m_SymbolTable->Resolve(expr->literal, symbol);
    if (state == RWState::READ)
    {
        if (!isFound)
            ASSERT("Undefined variable:%s",expr->Stringify().c_str());
        LoadSymbol(symbol);
    }
    else
    {
        if (!isFound)
            symbol = m_SymbolTable->Define(expr->literal);
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
}

void Compiler::CompileFunctionExpr(FunctionExpr *expr)
{
    EnterScope();

    m_Scopes.emplace_back(OpCodes());

    for (const auto &param : expr->parameters)
        m_SymbolTable->Define(param->literal);

    for (const auto &s : expr->body->stmts)
        CompileStmt(s);

    auto localVarCount = m_SymbolTable->definitionCount;

    ExitScope();

    auto opCodes = m_Scopes.back();
    m_Scopes.pop_back();

    // for non return  or empty stmt in function scope:add a return to return nothing
    if (opCodes.empty() || opCodes[opCodes.size() - 2] != OP_RETURN)
    {
        opCodes.emplace_back(OP_RETURN);
        opCodes.emplace_back(0);
    }

    auto fn = new FunctionObject(opCodes, localVarCount, (int32_t)expr->parameters.size());

    EmitConstant(AddConstant(fn));
}

void Compiler::CompileFunctionCallExpr(FunctionCallExpr *expr)
{
    CompileExpr(expr->name);

    for (const auto &argu : expr->arguments)
        CompileExpr(argu);

    Emit(OP_FUNCTION_CALL);
    Emit((int32_t)expr->arguments.size());
}

void Compiler::CompileStructCallExpr(StructCallExpr *expr, const RWState &state)
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
            Emit((int32_t)funcCall->arguments.size());
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

void Compiler::CompileAnonyStructExpr(AnonyStructExpr *expr)
{
    EnterScope();

    for (const auto &[k, v] : expr->memberPairs)
    {
        CompileExpr(v);
        EmitConstant(AddConstant(new StrObject(k->literal)));
    }

    Emit(OP_STRUCT);
    Emit((int32_t)expr->memberPairs.size());

    ExitScope();
}

void Compiler::CompileDllImportExpr(DllImportExpr *expr)
{
#ifdef _WIN32
    auto dllpath = expr->dllPath;

    typedef void (*RegFn)();

    HINSTANCE hInstance;
    hInstance = LoadLibrary(dllpath.c_str());
    if (!hInstance)
        ASSERT("Failed to load dll library:%s",dllpath.c_str());

    RegFn RegisterBuiltins = (RegFn)(GetProcAddress(hInstance, "RegisterBuiltins"));

    RegisterBuiltins();

    std::vector<std::string> newAddedBuiltinFunctionNames;
    std::vector<std::string> newAddedBuiltinVariableNames;

    for (const auto &name : BuiltinManager::GetInstance()->m_BuiltinFunctionNames)
    {
        bool found = false;
        for (const auto &recName : m_BuiltinFunctionNames)
        {
            if (name == recName)
            {
                found = true;
                break;
            }
        }
        if (found == false)
            newAddedBuiltinFunctionNames.emplace_back(name);
    }

    for (const auto &name : BuiltinManager::GetInstance()->m_BuiltinVariableNames)
    {
        bool found = false;
        for (const auto &recName : m_BuiltinVariableNames)
        {
            if (name == recName)
            {
                found = true;
                break;
            }
        }
        if (found == false)
            newAddedBuiltinVariableNames.emplace_back(name);
    }

    auto legacyBuiltinFuncCount = m_BuiltinFunctionNames.size();
    for (int32_t i = 0; i < newAddedBuiltinFunctionNames.size(); ++i)
    {
        m_BuiltinFunctionNames.emplace_back(newAddedBuiltinFunctionNames[i]);
        m_SymbolTable->DefineBuiltinFunction(newAddedBuiltinFunctionNames[i], legacyBuiltinFuncCount + i);
    }

    auto legacyBuiltinVarCount = m_BuiltinVariableNames.size();
    for (int32_t i = 0; i < newAddedBuiltinVariableNames.size(); ++i)
    {
        m_BuiltinVariableNames.emplace_back(newAddedBuiltinVariableNames[i]);
        m_SymbolTable->DefineBuiltinVariable(newAddedBuiltinVariableNames[i], legacyBuiltinVarCount + i);
    }
#endif
}

void Compiler::EnterScope()
{
    m_SymbolTable = new SymbolTable(m_SymbolTable);
}
void Compiler::ExitScope()
{
    m_SymbolTable = m_SymbolTable->enclosing;
}

OpCodes &Compiler::CurOpCodes()
{
    return m_Scopes.back();
}

uint32_t Compiler::Emit(int32_t opcode)
{
    CurOpCodes().emplace_back(opcode);
    return (int32_t)CurOpCodes().size() - 1;
}

uint32_t Compiler::EmitConstant(uint32_t pos)
{
    Emit(OP_CONSTANT);
    Emit(pos);
    return (int32_t)CurOpCodes().size() - 1;
}

void Compiler::ModifyOpCode(uint32_t pos, int32_t opcode)
{
    CurOpCodes()[pos] = opcode;
}

uint32_t Compiler::AddConstant(const Value &value)
{
    m_Constants.emplace_back(value);
    return m_Constants.size()-1;
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
        Emit(symbol.isInUpScope);
        Emit(symbol.scopeDepth);
        Emit(symbol.index);
        break;
    case SymbolScope::BUILTIN_FUNCTION:
        Emit(OP_GET_BUILTIN_FUNCTION);
        Emit(symbol.index);
        break;
    case SymbolScope::BUILTIN_VARIABLE:
        Emit(OP_GET_BUILTIN_VARIABLE);
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