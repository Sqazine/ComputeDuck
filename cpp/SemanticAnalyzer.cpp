#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer()
{
}
SemanticAnalyzer::~SemanticAnalyzer()
{
}

void SemanticAnalyzer::Analyze(std::vector<Stmt *> &stmts)
{
    for (auto &s : stmts)
        s = AnalyzeStmt(s);
}

Stmt *SemanticAnalyzer::AnalyzeStmt(Stmt *stmt)
{
    switch (stmt->Type())
    {
    case AstType::RETURN:
        return AnalyzeReturnStmt((ReturnStmt *)stmt);
    case AstType::EXPR:
        return AnalyzeExprStmt((ExprStmt *)stmt);
    case AstType::VAR:
        return AnalyzeVarStmt((VarStmt *)stmt);
    case AstType::SCOPE:
        return AnalyzeScopeStmt((ScopeStmt *)stmt);
    case AstType::IF:
        return AnalyzeIfStmt((IfStmt *)stmt);
    case AstType::WHILE:
        return AnalyzeWhileStmt((WhileStmt *)stmt);
    case AstType::FUNCTION:
        return AnalyzeFunctionStmt((FunctionStmt *)stmt);
    case AstType::STRUCT:
        return AnalyzeStructStmt((StructStmt *)stmt);
    default:
        return nullptr;
    }
}
Stmt *SemanticAnalyzer::AnalyzeExprStmt(ExprStmt *stmt)
{
    stmt->expr = AnalyzeExpr(stmt->expr);
    return stmt;
}
Stmt *SemanticAnalyzer::AnalyzeIfStmt(IfStmt *stmt)
{
    stmt->condition = AnalyzeExpr(stmt->condition);
    stmt->thenBranch = AnalyzeStmt(stmt->thenBranch);
    if(stmt->elseBranch)
        stmt->elseBranch = AnalyzeStmt(stmt->elseBranch);

    if (stmt->condition->Type() == AstType::BOOL)
    {
        if (((BoolExpr *)stmt->condition)->value == true)
            return stmt->thenBranch;
        else
            return stmt->elseBranch;
    }

    return stmt;
}
Stmt *SemanticAnalyzer::AnalyzeScopeStmt(ScopeStmt *stmt)
{
    for (auto &s : stmt->stmts)
        s = AnalyzeStmt(s);
    return stmt;
}
Stmt *SemanticAnalyzer::AnalyzeWhileStmt(WhileStmt *stmt)
{
    stmt->condition = AnalyzeExpr(stmt->condition);
    stmt->body = AnalyzeStmt(stmt->body);
    return stmt;
}
Stmt *SemanticAnalyzer::AnalyzeReturnStmt(ReturnStmt *stmt)
{
    stmt->expr = AnalyzeExpr(stmt->expr);
    return stmt;
}
Stmt *SemanticAnalyzer::AnalyzeVarStmt(VarStmt *stmt)
{
    stmt->value = AnalyzeExpr(stmt->value);
    return stmt;
}
Stmt *SemanticAnalyzer::AnalyzeFunctionStmt(FunctionStmt *stmt)
{
    for (auto &e : stmt->parameters)
        e = (IdentifierExpr *)AnalyzeIdentifierExpr(e);

    stmt->body = (ScopeStmt *)AnalyzeScopeStmt(stmt->body);
    return stmt;
}
Stmt *SemanticAnalyzer::AnalyzeStructStmt(StructStmt *stmt)
{
    for (auto &s : stmt->members)
        s = (VarStmt *)AnalyzeVarStmt(s);
    return stmt;
}
Expr *SemanticAnalyzer::AnalyzeExpr(Expr *expr)
{
    switch (expr->Type())
    {
    case AstType::NUM:
        return AnalyzeNumExpr((NumExpr *)expr);
    case AstType::STR:
        return AnalyzeStrExpr((StrExpr *)expr);
    case AstType::BOOL:
        return AnalyzeBoolExpr((BoolExpr *)expr);
    case AstType::NIL:
        return AnalyzeNilExpr((NilExpr *)expr);
    case AstType::IDENTIFIER:
        return AnalyzeIdentifierExpr((IdentifierExpr *)expr);
    case AstType::GROUP:
        return AnalyzeGroupExpr((GroupExpr *)expr);
    case AstType::ARRAY:
        return AnalyzeArrayExpr((ArrayExpr *)expr);
    case AstType::INDEX:
        return AnalyzeIndexExpr((IndexExpr *)expr);
    case AstType::PREFIX:
        return AnalyzePrefixExpr((PrefixExpr *)expr);
    case AstType::INFIX:
        return AnalyzeInfixExpr((InfixExpr *)expr);
    case AstType::FUNCTION_CALL:
        return AnalyzeFunctionCallExpr((FunctionCallExpr *)expr);
    case AstType::STRUCT_CALL:
        return AnalyzeStructCallExpr((StructCallExpr *)expr);
    case AstType::REF:
        return AnalyzeRefExpr((RefExpr *)expr);
    case AstType::LAMBDA:
        return AnalyzeLambdaExpr((LambdaExpr *)expr);
    default:
        return nullptr;
    }
}
Expr *SemanticAnalyzer::AnalyzeInfixExpr(InfixExpr *expr)
{
    expr->left = AnalyzeExpr(expr->left);
    expr->right = AnalyzeExpr(expr->right);

    return ConstantFold(expr);
}
Expr *SemanticAnalyzer::AnalyzeNumExpr(NumExpr *expr)
{
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeBoolExpr(BoolExpr *expr)
{
    return expr;
}
Expr *SemanticAnalyzer::AnalyzePrefixExpr(PrefixExpr *expr)
{
    expr->right=AnalyzeExpr(expr->right);
    return ConstantFold(expr);
}
Expr *SemanticAnalyzer::AnalyzeStrExpr(StrExpr *expr)
{
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeNilExpr(NilExpr *expr)
{
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeGroupExpr(GroupExpr *expr)
{
    return AnalyzeExpr(expr->expr);
}
Expr *SemanticAnalyzer::AnalyzeArrayExpr(ArrayExpr *expr)
{
    for (auto &e : expr->elements)
        e = AnalyzeExpr(e);
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeIndexExpr(IndexExpr *expr)
{
    expr->ds = AnalyzeExpr(expr->ds);
    expr->index = AnalyzeExpr(expr->index);
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeIdentifierExpr(IdentifierExpr *expr)
{
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeLambdaExpr(LambdaExpr *expr)
{
    for (auto &e : expr->parameters)
        e = (IdentifierExpr *)AnalyzeIdentifierExpr(e);
    expr->body = (ScopeStmt *)AnalyzeScopeStmt(expr->body);
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeFunctionCallExpr(FunctionCallExpr *expr)
{
    expr->name = AnalyzeExpr(expr->name);
    for (auto &e : expr->arguments)
        e = AnalyzeExpr(e);
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeStructCallExpr(StructCallExpr *expr)
{
    expr->callee = AnalyzeExpr(expr->callee);
    expr->callMember = AnalyzeExpr(expr->callMember);
    return expr;
}
Expr *SemanticAnalyzer::AnalyzeRefExpr(RefExpr *expr)
{
    expr->refExpr = (IdentifierExpr *)AnalyzeExpr(expr->refExpr);
    return expr;
}

Expr *SemanticAnalyzer::ConstantFold(Expr *expr)
{
    if (expr->Type() == AstType::INFIX)
    {
        auto infix = (InfixExpr *)expr;
        if (infix->left->Type() == AstType::NUM && infix->right->Type() == AstType::NUM)
        {
            Expr *newExpr = nullptr;
            if (infix->op == "+")
                newExpr = new NumExpr(((NumExpr *)infix->left)->value + ((NumExpr *)infix->right)->value);
            else if (infix->op == "-")
                newExpr = new NumExpr(((NumExpr *)infix->left)->value - ((NumExpr *)infix->right)->value);
            else if (infix->op == "*")
                newExpr = new NumExpr(((NumExpr *)infix->left)->value * ((NumExpr *)infix->right)->value);
            else if (infix->op == "/")
                newExpr = new NumExpr(((NumExpr *)infix->left)->value / ((NumExpr *)infix->right)->value);
            else if (infix->op == "==")
                newExpr = new BoolExpr(((NumExpr *)infix->left)->value == ((NumExpr *)infix->right)->value);
            else if (infix->op == "!=")
                newExpr = new BoolExpr(((NumExpr *)infix->left)->value != ((NumExpr *)infix->right)->value);
            else if (infix->op == ">")
                newExpr = new BoolExpr(((NumExpr *)infix->left)->value > ((NumExpr *)infix->right)->value);
            else if (infix->op == ">=")
                newExpr = new BoolExpr(((NumExpr *)infix->left)->value >= ((NumExpr *)infix->right)->value);
            else if (infix->op == "<")
                newExpr = new BoolExpr(((NumExpr *)infix->left)->value < ((NumExpr *)infix->right)->value);
            else if (infix->op == "<=")
                newExpr = new BoolExpr(((NumExpr *)infix->left)->value <= ((NumExpr *)infix->right)->value);

            delete infix;
            infix = nullptr;
            return newExpr;
        }
        else if (infix->left->Type() == AstType::STR && infix->right->Type() == AstType::STR)
        {
            auto strExpr = new StrExpr(((StrExpr *)infix->left)->value + ((StrExpr *)infix->right)->value);
            delete infix;
            infix = nullptr;
            return strExpr;
        }
    }
    else if (expr->Type() == AstType::PREFIX)
    {
        auto prefix = (PrefixExpr *)expr;
        if (prefix->right->Type() == AstType::NUM && prefix->op == "-")
        {
            auto numExpr = new NumExpr(-((NumExpr *)prefix->right)->value);
            delete prefix;
            prefix = nullptr;
            return numExpr;
        }
        else if (prefix->right->Type() == AstType::BOOL && prefix->op == "!")
        {
            auto boolExpr = new BoolExpr(!((BoolExpr *)prefix->right)->value);
            delete prefix;
            prefix = nullptr;
            return boolExpr;
        }
    }

    return expr;
}