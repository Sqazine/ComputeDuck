#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <memory>
#include "Utils.h"

enum class AstType
{
	NUM,
	STR,
    NIL,
	BOOL,
	GROUP,
	UNARY,
	BINARY,

	EXPR,
};

struct AstNode
{
	AstNode(AstType type) : type(type) {}
	virtual ~AstNode() = default;

	virtual std::string Stringify() = 0;

	AstType type;
};

struct Expr : public AstNode
{
	Expr(AstType type) : AstNode(type) {}
	virtual ~Expr() = default;

	virtual std::string Stringify() = 0;
};

struct NumExpr : public Expr
{
	NumExpr() : Expr(AstType::NUM), value(0.0) {}
	NumExpr(double value) : Expr(AstType::NUM), value(value) {}
	~NumExpr() override = default;

	std::string Stringify() override { return std::to_string(value); }

	double value;
};

struct StrExpr : public Expr
{
	StrExpr() : Expr(AstType::STR) {}
	StrExpr(std::string_view str) : Expr(AstType::STR), value(str) {}
	~StrExpr() override = default;

	std::string Stringify() override { return "\"" + value + "\""; }

	std::string value;
};

struct NilExpr : public Expr
{
	NilExpr() : Expr(AstType::NIL) {}
	~NilExpr() override = default;

	std::string Stringify() override { return "nil"; }
};

struct BoolExpr : public Expr
{
	BoolExpr() : Expr(AstType::BOOL), value(false) {}
	BoolExpr(bool value) : Expr(AstType::BOOL), value(value) {}
	~BoolExpr() override = default;

	std::string Stringify() override { return value ? "true" : "false"; }
	bool value;
};


struct GroupExpr : public Expr
{
	GroupExpr() : Expr(AstType::GROUP), expr(nullptr) {}
	GroupExpr(Expr *expr) : Expr(AstType::GROUP), expr(expr) {}
	~GroupExpr() override { SAFE_DELETE(expr); }

	std::string Stringify() override { return "(" + expr->Stringify() + ")"; }

	Expr *expr;
};

struct UnaryExpr : public Expr
{
	UnaryExpr() : Expr(AstType::UNARY), right(nullptr) {}
	UnaryExpr(std::string_view op, Expr *right) : Expr(AstType::UNARY), op(op), right(right) {}
	~UnaryExpr() override { SAFE_DELETE(right); }

	std::string Stringify() override { return op + right->Stringify(); }

	std::string op;
	Expr *right;
};

struct BinaryExpr : public Expr
{
	BinaryExpr() : Expr(AstType::BINARY), left(nullptr), right(nullptr) {}
	BinaryExpr(std::string_view op, Expr *left, Expr *right) : Expr(AstType::BINARY), op(op), left(left), right(right) {}
	~BinaryExpr() override
	{
		SAFE_DELETE(left);
		SAFE_DELETE(right);
	}

	std::string Stringify() override { return left->Stringify() + " " + op + " " + right->Stringify(); }

	std::string op;
	Expr *left;
	Expr *right;
};

struct Stmt : public AstNode
{
	Stmt(AstType type) : AstNode(type) {}
	virtual ~Stmt() {}

	virtual std::string Stringify() = 0;
};

struct ExprStmt : public Stmt
{
	ExprStmt() : Stmt(AstType::EXPR), expr(nullptr) {}
	ExprStmt(Expr *expr) : Stmt(AstType::EXPR), expr(expr) {}
	~ExprStmt() override { SAFE_DELETE(expr); }

	std::string Stringify() override { return expr->Stringify() + ";"; }

	Expr *expr;
};