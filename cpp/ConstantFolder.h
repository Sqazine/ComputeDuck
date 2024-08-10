#pragma once
#include <vector>
#include "Ast.h"
class ConstantFolder
{
public:
    ConstantFolder() = default;
    ~ConstantFolder() = default;

    void Fold(std::vector<Stmt *> &stmts);

private:
    Stmt *FoldStmt(Stmt *stmt);
    Stmt *FoldExprStmt(ExprStmt *stmt);
    Stmt *FoldIfStmt(IfStmt *stmt);
    Stmt *FoldScopeStmt(ScopeStmt *stmt);
    Stmt *FoldWhileStmt(WhileStmt *stmt);
    Stmt *FoldReturnStmt(ReturnStmt *stmt);
    Stmt *FoldStructStmt(StructStmt *stmt);

    Expr *FoldExpr(Expr *expr);
    Expr *FoldInfixExpr(InfixExpr *expr);
    Expr *FoldNumExpr(NumExpr *expr);
    Expr *FoldBoolExpr(BoolExpr *expr);
    Expr *FoldPrefixExpr(PrefixExpr *expr);
    Expr *FoldStrExpr(StrExpr *expr);
    Expr *FoldNilExpr(NilExpr *expr);
    Expr *FoldGroupExpr(GroupExpr *expr);
    Expr *FoldArrayExpr(ArrayExpr *expr);
    Expr *FoldIndexExpr(IndexExpr *expr);
    Expr *FoldIdentifierExpr(IdentifierExpr *expr);
    Expr *FoldFunctionExpr(FunctionExpr *expr);
    Expr *FoldFunctionCallExpr(FunctionCallExpr *expr);
    Expr *FoldStructCallExpr(StructCallExpr *expr);
    Expr *FoldRefExpr(RefExpr *expr);
    Expr *FoldStructExpr(StructExpr *expr);

    Expr *ConstantFold(Expr *expr);
};