#include "ConstantFolder.h"
#include "Utils.h"

void ConstantFolder::Fold(std::vector<Stmt *> &stmts)
{
    for (auto &s : stmts)
        s = FoldStmt(s);
}

Stmt *ConstantFolder::FoldStmt(Stmt *stmt)
{
    switch (stmt->type)
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
        return stmt;
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

    if (stmt->condition->type == AstType::BOOL)
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
    stmt->body = (StructExpr *)FoldExpr(stmt->body);
    return stmt;
}
Expr *ConstantFolder::FoldExpr(Expr *expr)
{
    switch (expr->type)
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
    case AstType::STRUCT:
        return FoldStructExpr((StructExpr *)expr);
    default:
        return expr;
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
    expr->refExpr = FoldExpr(expr->refExpr);
    return expr;
}

Expr *ConstantFolder::FoldStructExpr(StructExpr *expr)
{
    for (auto &[k, v] : expr->members)
        v = FoldExpr(v);
    return expr;
}

Expr *ConstantFolder::ConstantFold(Expr *expr)
{
    if (expr->type == AstType::INFIX)
    {
        auto infix = (InfixExpr *)expr;
        if (infix->left->type == AstType::NUM && infix->right->type == AstType::NUM)
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
            else if (infix->op == "&")
                newExpr = new NumExpr((double)((int64_t)((NumExpr *)infix->left)->value & (int64_t)((NumExpr *)infix->right)->value));
            else if (infix->op == "|")
                newExpr = new NumExpr((double)((int64_t)((NumExpr *)infix->left)->value | (int64_t)((NumExpr *)infix->right)->value));
            else if (infix->op == "^")
                newExpr = new NumExpr((double)((int64_t)((NumExpr *)infix->left)->value ^ (int64_t)((NumExpr *)infix->right)->value));
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

            SAFE_DELETE(infix);
            return newExpr;
        }
        else if (infix->left->type == AstType::STR && infix->right->type == AstType::STR)
        {
            auto strExpr = new StrExpr(((StrExpr *)infix->left)->value + ((StrExpr *)infix->right)->value);
            SAFE_DELETE(infix);
            return strExpr;
        }
    }
    else if (expr->type == AstType::PREFIX)
    {
        auto prefix = (PrefixExpr *)expr;
        if (prefix->right->type == AstType::NUM && prefix->op == "-")
        {
            auto numExpr = new NumExpr(-((NumExpr *)prefix->right)->value);
            SAFE_DELETE(prefix);
            return numExpr;
        }
        else if (prefix->right->type == AstType::BOOL && prefix->op == "not")
        {
            auto boolExpr = new BoolExpr(!((BoolExpr *)prefix->right)->value);
            SAFE_DELETE(prefix);
            return boolExpr;
        }
        else if (prefix->right->type == AstType::NUM && prefix->op == "~")
        {
            auto v = ~(int64_t)((NumExpr *)prefix->right)->value;
            auto numExpr = new NumExpr((double)v);
            SAFE_DELETE(prefix);
            return numExpr;
        }
    }

    return expr;
}