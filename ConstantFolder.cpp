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
    case AstType::UNARY:
        return FoldUnaryExpr((UnaryExpr *)expr);
    case AstType::BINARY:
        return FoldBinaryExpr((BinaryExpr *)expr);
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
Expr *ConstantFolder::FoldBinaryExpr(BinaryExpr *expr)
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
Expr *ConstantFolder::FoldUnaryExpr(UnaryExpr *expr)
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
    if (expr->type == AstType::BINARY)
    {
        auto binary = (BinaryExpr *)expr;
        if (binary->left->type == AstType::NUM && binary->right->type == AstType::NUM)
        {
            Expr *newExpr = nullptr;
            if (binary->op == "+")
                newExpr = new NumExpr(((NumExpr *)binary->left)->value + ((NumExpr *)binary->right)->value);
            else if (binary->op == "-")
                newExpr = new NumExpr(((NumExpr *)binary->left)->value - ((NumExpr *)binary->right)->value);
            else if (binary->op == "*")
                newExpr = new NumExpr(((NumExpr *)binary->left)->value * ((NumExpr *)binary->right)->value);
            else if (binary->op == "/")
                newExpr = new NumExpr(((NumExpr *)binary->left)->value / ((NumExpr *)binary->right)->value);
            else if (binary->op == "&")
                newExpr = new NumExpr((double)((int64_t)((NumExpr *)binary->left)->value & (int64_t)((NumExpr *)binary->right)->value));
            else if (binary->op == "|")
                newExpr = new NumExpr((double)((int64_t)((NumExpr *)binary->left)->value | (int64_t)((NumExpr *)binary->right)->value));
            else if (binary->op == "^")
                newExpr = new NumExpr((double)((int64_t)((NumExpr *)binary->left)->value ^ (int64_t)((NumExpr *)binary->right)->value));
            else if (binary->op == "==")
                newExpr = new BoolExpr(((NumExpr *)binary->left)->value == ((NumExpr *)binary->right)->value);
            else if (binary->op == "!=")
                newExpr = new BoolExpr(((NumExpr *)binary->left)->value != ((NumExpr *)binary->right)->value);
            else if (binary->op == ">")
                newExpr = new BoolExpr(((NumExpr *)binary->left)->value > ((NumExpr *)binary->right)->value);
            else if (binary->op == ">=")
                newExpr = new BoolExpr(((NumExpr *)binary->left)->value >= ((NumExpr *)binary->right)->value);
            else if (binary->op == "<")
                newExpr = new BoolExpr(((NumExpr *)binary->left)->value < ((NumExpr *)binary->right)->value);
            else if (binary->op == "<=")
                newExpr = new BoolExpr(((NumExpr *)binary->left)->value <= ((NumExpr *)binary->right)->value);

            SAFE_DELETE(binary);
            return newExpr;
        }
        else if (binary->left->type == AstType::STR && binary->right->type == AstType::STR)
        {
            auto strExpr = new StrExpr(((StrExpr *)binary->left)->value + ((StrExpr *)binary->right)->value);
            SAFE_DELETE(binary);
            return strExpr;
        }
    }
    else if (expr->type == AstType::UNARY)
    {
        auto unary = (UnaryExpr *)expr;
        if (unary->right->type == AstType::NUM && unary->op == "-")
        {
            auto numExpr = new NumExpr(-((NumExpr *)unary->right)->value);
            SAFE_DELETE(unary);
            return numExpr;
        }
        else if (unary->right->type == AstType::BOOL && unary->op == "not")
        {
            auto boolExpr = new BoolExpr(!((BoolExpr *)unary->right)->value);
            SAFE_DELETE(unary);
            return boolExpr;
        }
        else if (unary->right->type == AstType::NUM && unary->op == "~")
        {
            auto v = ~(int64_t)((NumExpr *)unary->right)->value;
            auto numExpr = new NumExpr((double)v);
            SAFE_DELETE(unary);
            return numExpr;
        }
    }

    return expr;
}