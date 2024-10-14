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
	IDENTIFIER,
	GROUP,
	ARRAY,
	INDEX,
	UNARY,
	BINARY,

	EXPR,

	// ++ 新增内容
	PRINT,
	// -- 新增内容
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

struct IdentifierExpr : public Expr
{
	IdentifierExpr() : Expr(AstType::IDENTIFIER) {}
	IdentifierExpr(std::string_view literal) : Expr(AstType::IDENTIFIER), literal(literal) {}
	~IdentifierExpr() override = default;

	std::string Stringify() override { return literal; }

	std::string literal;
};

struct ArrayExpr : public Expr
{
	ArrayExpr() : Expr(AstType::ARRAY) {}
	ArrayExpr(std::vector<Expr *> elements) : Expr(AstType::ARRAY), elements(elements) {}
	~ArrayExpr() override { std::vector<Expr *>().swap(elements); }

	std::string Stringify() override
	{
		std::string result = "[";

		if (!elements.empty())
		{
			for (auto e : elements)
				result += e->Stringify() + ",";
			result = result.substr(0, result.size() - 1);
		}
		result += "]";
		return result;
	}

	std::vector<Expr *> elements;
};

struct IndexExpr : public Expr
{
	IndexExpr() : Expr(AstType::INDEX), ds(nullptr), index(nullptr) {}
	IndexExpr(Expr *ds, Expr *index) : Expr(AstType::INDEX), ds(ds), index(index) {}
	~IndexExpr() override
	{
		SAFE_DELETE(ds);
		SAFE_DELETE(index);
	}
	std::string Stringify() override { return ds->Stringify() + "[" + index->Stringify() + "]"; }

	Expr *ds;
	Expr *index;
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
// ++ 新增内容
struct PrintStmt : public Stmt
{
	PrintStmt() : Stmt(AstType::PRINT), expr(nullptr) {}
	PrintStmt(Expr *expr) : Stmt(AstType::PRINT), expr(expr) {}
	~PrintStmt() override { SAFE_DELETE(expr); }

	std::string Stringify() override { return "print " + expr->Stringify() + ";"; }

	Expr *expr;
};
// -- 新增内容