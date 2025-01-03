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
	UNARY,
	BINARY,
	INDEX,
	REF,
	FUNCTION,
	FUNCTION_CALL,
	STRUCT_CALL,

	EXPR,
	RETURN,
	IF,
	SCOPE,
	WHILE,
	DLL_IMPORT,

	STRUCT,
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

struct RefExpr : public Expr
{
	RefExpr() : Expr(AstType::REF), refExpr(nullptr) {}
	RefExpr(Expr *refExpr) : Expr(AstType::REF), refExpr(refExpr) {}
	~RefExpr() override { SAFE_DELETE(refExpr); }
	std::string Stringify() override { return "ref " + refExpr->Stringify(); }

	Expr *refExpr;
};

struct FunctionCallExpr : public Expr
{
	FunctionCallExpr() : Expr(AstType::FUNCTION_CALL), name(nullptr) {}
	FunctionCallExpr(Expr *name, std::vector<Expr *> arguments) : Expr(AstType::FUNCTION_CALL), name(name), arguments(arguments) {}
	~FunctionCallExpr() override
	{
		SAFE_DELETE(name);
		std::vector<Expr *>().swap(arguments);
	}

	std::string Stringify() override
	{
		std::string result = name->Stringify() + "(";

		if (!arguments.empty())
		{
			for (const auto &arg : arguments)
				result += arg->Stringify() + ",";
			result = result.substr(0, result.size() - 1);
		}
		result += ")";
		return result;
	}

	Expr *name;
	std::vector<Expr *> arguments;
};

struct StructCallExpr : public Expr
{
	StructCallExpr() : Expr(AstType::STRUCT_CALL), callee(nullptr), callMember(nullptr) {}
	StructCallExpr(Expr *callee, Expr *callMember) : Expr(AstType::STRUCT_CALL), callee(callee), callMember(callMember) {}
	~StructCallExpr() override
	{
		SAFE_DELETE(callee);
		SAFE_DELETE(callMember);
	}

	std::string Stringify() override { return callee->Stringify() + "." + callMember->Stringify(); }

	Expr *callee;
	Expr *callMember;
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

struct ReturnStmt : public Stmt
{
	ReturnStmt() : Stmt(AstType::RETURN), expr(nullptr) {}
	ReturnStmt(Expr *expr) : Stmt(AstType::RETURN), expr(expr) {}
	~ReturnStmt() override { SAFE_DELETE(expr); }

	std::string Stringify() override { return "return " + expr->Stringify() + ";"; }

	Expr *expr;
};

struct IfStmt : public Stmt
{
	IfStmt() : Stmt(AstType::IF), condition(nullptr), thenBranch(nullptr), elseBranch(nullptr) {}
	IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
		: Stmt(AstType::IF),
		  condition(condition),
		  thenBranch(thenBranch),
		  elseBranch(elseBranch)
	{
	}
	~IfStmt() override
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(thenBranch);
		SAFE_DELETE(elseBranch);
	}

	std::string Stringify() override
	{
		std::string result;
		result = "if(" + condition->Stringify() + ")" + thenBranch->Stringify();
		if (elseBranch != nullptr)
			result += "else " + elseBranch->Stringify();
		return result;
	}

	Expr *condition;
	Stmt *thenBranch;
	Stmt *elseBranch;
};

struct ScopeStmt : public Stmt
{
	ScopeStmt() : Stmt(AstType::SCOPE) {}
	ScopeStmt(std::vector<Stmt *> stmts) : Stmt(AstType::SCOPE), stmts(stmts) {}
	~ScopeStmt() override { std::vector<Stmt *>().swap(stmts); }

	std::string Stringify() override
	{
		std::string result = "{";
		for (const auto &stmt : stmts)
			result += stmt->Stringify();
		result += "}";
		return result;
	}

	std::vector<Stmt *> stmts;
};

struct FunctionExpr : public Expr
{
	FunctionExpr() : Expr(AstType::FUNCTION), body(nullptr) {}
	FunctionExpr(std::vector<IdentifierExpr *> parameters, ScopeStmt *body) : Expr(AstType::FUNCTION), parameters(parameters), body(body) {}
	~FunctionExpr() override
	{
		std::vector<IdentifierExpr *>().swap(parameters);
		SAFE_DELETE(body);
	}

	std::string Stringify() override
	{
		std::string result = "function(";
		if (!parameters.empty())
		{
			for (auto param : parameters)
				result += param->Stringify() + ",";
			result = result.substr(0, result.size() - 1);
		}
		result += ")";
		result += body->Stringify();
		return result;
	}

	std::vector<IdentifierExpr *> parameters;
	ScopeStmt *body;
};

struct StructExpr : public Expr
{
	StructExpr() : Expr(AstType::STRUCT) {}
	StructExpr(const std::unordered_map<IdentifierExpr *, Expr *> &members) : Expr(AstType::STRUCT), members(members) {}
	~StructExpr() override { std::unordered_map<IdentifierExpr *, Expr *>().swap(members); }

	std::string Stringify() override
	{
		std::string result = "{";
		for (const auto &[k, v] : members)
			result += k->Stringify() + ":" + v->Stringify() + ",\n";
		result += "}";
		return result;
	}

	std::unordered_map<IdentifierExpr *, Expr *> members;
};

struct WhileStmt : public Stmt
{
	WhileStmt() : Stmt(AstType::WHILE), condition(nullptr), body(nullptr) {}
	WhileStmt(Expr *condition, Stmt *body) : Stmt(AstType::WHILE), condition(condition), body(body) {}
	~WhileStmt() override
	{
		SAFE_DELETE(condition);
		SAFE_DELETE(body);
	}

	std::string Stringify() override
	{
		return "while(" + condition->Stringify() + ")" + body->Stringify();
	}

	Expr *condition;
	Stmt *body;
};

struct StructStmt : public Stmt
{
	StructStmt() : Stmt(AstType::STRUCT), body(new StructExpr()) {}
	StructStmt(std::string_view name, StructExpr *body) : Stmt(AstType::STRUCT), name(name), body(body) {}
	~StructStmt() override { SAFE_DELETE(body); }

	std::string Stringify() override
	{
		return "struct " + name + body->Stringify();
	}

	std::string name;
	StructExpr *body;
};

struct DllImportStmt : public Stmt
{
	DllImportStmt() : Stmt(AstType::DLL_IMPORT) {}
	DllImportStmt(std::string_view path) : Stmt(AstType::DLL_IMPORT), dllPath(path) {}
	~DllImportStmt() override = default;

	std::string Stringify() override { return "dllimport(\"" + dllPath + "\")"; }

	std::string dllPath;
};