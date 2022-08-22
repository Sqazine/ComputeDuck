#pragma once
#include <vector>
#include "Ast.h"
class SemanticAnalyzer
{
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    void Analyze(std::vector<Stmt *> &stmts);

private:
    Stmt* AnalyzeStmt(Stmt *stmt);
    Stmt* AnalyzeExprStmt(ExprStmt *stmt);
    Stmt* AnalyzeIfStmt(IfStmt *stmt);
    Stmt* AnalyzeScopeStmt(ScopeStmt *stmt);
    Stmt* AnalyzeWhileStmt(WhileStmt *stmt);
    Stmt* AnalyzeReturnStmt(ReturnStmt *stmt);
    Stmt* AnalyzeVarStmt(VarStmt *stmt);
    Stmt* AnalyzeFunctionStmt(FunctionStmt *stmt);
    Stmt* AnalyzeStructStmt(StructStmt *stmt);

    Expr* AnalyzeExpr(Expr *expr);
    Expr* AnalyzeInfixExpr(InfixExpr *expr);
    Expr* AnalyzeNumExpr(NumExpr *expr);
    Expr* AnalyzeBoolExpr(BoolExpr *expr);
    Expr* AnalyzePrefixExpr(PrefixExpr *expr);
    Expr* AnalyzeStrExpr(StrExpr *expr);
    Expr* AnalyzeNilExpr(NilExpr *expr);
    Expr* AnalyzeGroupExpr(GroupExpr *expr);
    Expr* AnalyzeArrayExpr(ArrayExpr *expr);
    Expr* AnalyzeIndexExpr(IndexExpr *expr);
    Expr* AnalyzeIdentifierExpr(IdentifierExpr *expr);
    Expr* AnalyzeLambdaExpr(LambdaExpr *expr);
    Expr* AnalyzeFunctionCallExpr(FunctionCallExpr *expr);
    Expr* AnalyzeStructCallExpr(StructCallExpr *expr);
    Expr* AnalyzeRefExpr(RefExpr *expr);

    Expr *ConstantFold(Expr *expr);
};