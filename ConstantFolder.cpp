#include "ConstantFolder.h"

ConstantFolder::ConstantFolder()
{
}
ConstantFolder::~ConstantFolder()
{
}

void ConstantFolder::Fold(std::vector<Stmt *> &stmts)
{
    for (auto &s : stmts)
        s = FoldStmt(s);
}

Stmt *ConstantFolder::FoldStmt(Stmt *stmt)
{
    switch (stmt->Type())
    {
    case AstType::RETURN:
        return FoldReturnStmt((ReturnStmt *)stmt);
    case AstType::EXPR:
        return FoldExprStmt((ExprStmt *)stmt);
    case AstType::SCOPE:
        return FoldScopeStmt((ScopeStmt *)stmt);
    case AstType::IF:
        return FoldIfStmt((IfStmt *)stmt);
    case AstType::WHILE:
        return FoldWhileStmt((WhileStmt *)stmt);
    case AstType::STRUCT:
        return FoldStructStmt((StructStmt *)stmt);
    default:
        return nullptr;
    }
}
Stmt *ConstantFolder::FoldExprStmt(ExprStmt *stmt)
{
    stmt->expr = FoldExpr(stmt->expr);
    return stmt;
}
Stmt *ConstantFolder::FoldIfStmt(IfStmt *stmt)
{
    stmt->condition = FoldExpr(stmt->condition);
    stmt->thenBranch = FoldStmt(stmt->thenBranch);
    if (stmt->elseBranch)
        stmt->elseBranch = FoldStmt(stmt->elseBranch);

    if (stmt->condition->Type() == AstType::BOOL)
    {
        if (((BoolExpr *)stmt->condition)->value == true)
            return stmt->thenBranch;
        else
            return stmt->elseBranch;
    }

    return stmt;
}
Stmt *ConstantFolder::FoldScopeStmt(ScopeStmt *stmt)
{
    for (auto &s : stmt->stmts)
        s = FoldStmt(s);
    return stmt;
}
Stmt *ConstantFolder::FoldWhileStmt(WhileStmt *stmt)
{
    stmt->condition = FoldExpr(stmt->condition);
    stmt->body = FoldStmt(stmt->body);
    return stmt;
}
Stmt *ConstantFolder::FoldReturnStmt(ReturnStmt *stmt)
{
    stmt->expr = FoldExpr(stmt->expr);
    return stmt;
}

Stmt *ConstantFolder::FoldStructStmt(StructStmt *stmt)
{
    for (auto &[k, v] : stmt->members)
        v = FoldExpr(v);
    return stmt;
}
Expr *ConstantFolder::FoldExpr(Expr *expr)
{
    switch (expr->Type())
    {
    case AstType::NUM:
        return FoldNumExpr((NumExpr *)expr);
    case AstType::STR:
        return FoldStrExpr((StrExpr *)expr);
    case AstType::BOOL:
        return FoldBoolExpr((BoolExpr *)expr);
    case AstType::NIL:
        return FoldNilExpr((NilExpr *)expr);
    case AstType::IDENTIFIER:
        return FoldIdentifierExpr((IdentifierExpr *)expr);
    case AstType::GROUP:
        return FoldGroupExpr((GroupExpr *)expr);
    case AstType::ARRAY:
        return FoldArrayExpr((ArrayExpr *)expr);
    case AstType::INDEX:
        return FoldIndexExpr((IndexExpr *)expr);
    case AstType::PREFIX:
        return FoldPrefixExpr((PrefixExpr *)expr);
    case AstType::INFIX:
        return FoldInfixExpr((InfixExpr *)expr);
    case AstType::FUNCTION_CALL:
        return FoldFunctionCallExpr((FunctionCallExpr *)expr);
    case AstType::STRUCT_CALL:
        return FoldStructCallExpr((StructCallExpr *)expr);
    case AstType::REF:
        return FoldRefExpr((RefExpr *)expr);
    case AstType::FUNCTION:
        return FoldFunctionExpr((FunctionExpr *)expr);
    case AstType::ANONY_STRUCT:
        return FoldAnonyStructExpr((AnonyStructExpr *)expr);
    default:
        return nullptr;
    }
}
Expr *ConstantFolder::FoldInfixExpr(InfixExpr *expr)
{
    expr->left = FoldExpr(expr->left);
    expr->right = FoldExpr(expr->right);

    return ConstantFold(expr);
}
Expr *ConstantFolder::FoldNumExpr(NumExpr *expr)
{
    return expr;
}
Expr *ConstantFolder::FoldBoolExpr(BoolExpr *expr)
{
    return expr;
}
Expr *ConstantFolder::FoldPrefixExpr(PrefixExpr *expr)
{
    expr->right = FoldExpr(expr->right);
    return ConstantFold(expr);
}
Expr *ConstantFolder::FoldStrExpr(StrExpr *expr)
{
    return expr;
}
Expr *ConstantFolder::FoldNilExpr(NilExpr *expr)
{
    return expr;
}
Expr *ConstantFolder::FoldGroupExpr(GroupExpr *expr)
{
    return FoldExpr(expr->expr);
}
Expr *ConstantFolder::FoldArrayExpr(ArrayExpr *expr)
{
    for (auto &e : expr->elements)
        e = FoldExpr(e);
    return expr;
}
Expr *ConstantFolder::FoldIndexExpr(IndexExpr *expr)
{
    expr->ds = FoldExpr(expr->ds);
    expr->index = FoldExpr(expr->index);
    return expr;
}
Expr *ConstantFolder::FoldIdentifierExpr(IdentifierExpr *expr)
{
    return expr;
}
Expr *ConstantFolder::FoldFunctionExpr(FunctionExpr *expr)
{
    for (auto &e : expr->parameters)
        e = (IdentifierExpr *)FoldIdentifierExpr(e);
    expr->body = (ScopeStmt *)FoldScopeStmt(expr->body);
    return expr;
}
Expr *ConstantFolder::FoldFunctionCallExpr(FunctionCallExpr *expr)
{
    expr->name = FoldExpr(expr->name);
    for (auto &e : expr->arguments)
        e = FoldExpr(e);
    return expr;
}
Expr *ConstantFolder::FoldStructCallExpr(StructCallExpr *expr)
{
    expr->callee = FoldExpr(expr->callee);
    expr->callMember = FoldExpr(expr->callMember);
    return expr;
}
Expr *ConstantFolder::FoldRefExpr(RefExpr *expr)
{
    expr->refExpr = (IdentifierExpr *)FoldExpr(expr->refExpr);
    return expr;
}

Expr *ConstantFolder::FoldAnonyStructExpr(AnonyStructExpr *expr)
{
    for (auto &[k, v] : expr->memberPairs)
        v = FoldExpr(v);
    return expr;
}

Expr *ConstantFolder::ConstantFold(Expr *expr)
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
        else if (prefix->right->Type() == AstType::BOOL && prefix->op == "not")
        {
            auto boolExpr = new BoolExpr(!((BoolExpr *)prefix->right)->value);
            delete prefix;
            prefix = nullptr;
            return boolExpr;
        }
    }

    return expr;
}