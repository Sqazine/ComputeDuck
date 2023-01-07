#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <memory>

enum class AstType
{
	//expr
	NUM,
	STR,
	NIL,
	BOOL,
	IDENTIFIER,
	GROUP,
	ARRAY,
	PREFIX,
	INFIX,
	INDEX,
	REF,
	FUNCTION,
	ANONY_STRUCT,
	FUNCTION_CALL,
	STRUCT_CALL,
	DLL_IMPORT,
	//stmt
	EXPR,
	RETURN,
	IF,
	SCOPE,
	WHILE,
	STRUCT,
};

struct AstNode
{
	AstNode(AstType type) : type(type) {}
	virtual ~AstNode() {}

	virtual std::string Stringify() = 0;

	AstType type;
};

struct Expr : public AstNode
{
	Expr(AstType type) : AstNode(type) {}
	virtual ~Expr() {}

	virtual std::string Stringify() = 0;
};

struct NumExpr : public Expr
{
	NumExpr() : Expr(AstType::NUM), value(0.0) {}
	NumExpr(double value) : Expr(AstType::NUM), value(value) {}

	std::string Stringify() override { return std::to_string(value); }

	double value;
};

struct StrExpr : public Expr
{
	StrExpr() : Expr(AstType::STR) {}
	StrExpr(std::string_view str) : Expr(AstType::STR), value(str) {}

	std::string Stringify() override { return "\"" + value + "\""; }

	std::string value;
};

struct NilExpr : public Expr
{
	NilExpr() : Expr(AstType::NIL) {}
	~NilExpr() {}

	std::string Stringify() override { return "nil"; }
};

struct BoolExpr : public Expr
{
	BoolExpr() : Expr(AstType::BOOL), value(false) {}
	BoolExpr(bool value) : Expr(AstType::BOOL), value(value) {}
	~BoolExpr() {}

	std::string Stringify() override { return value ? "true" : "false"; }
	bool value;
};

struct IdentifierExpr : public Expr
{
	IdentifierExpr() : Expr(AstType::IDENTIFIER) {}
	IdentifierExpr(std::string_view literal) : Expr(AstType::IDENTIFIER), literal(literal) {}
	~IdentifierExpr() {}

	std::string Stringify() override { return literal; }

	std::string literal;
};

struct ArrayExpr : public Expr
{
	ArrayExpr() : Expr(AstType::ARRAY) {}
	ArrayExpr(std::vector<Expr *> elements) : Expr(AstType::ARRAY), elements(elements) {}
	~ArrayExpr()
	{
		std::vector<Expr *>().swap(elements);
	}

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
	~GroupExpr()
	{
		delete expr;
		expr = nullptr;
	}

	std::string Stringify() override { return "(" + expr->Stringify() + ")"; }

	Expr *expr;
};

struct PrefixExpr : public Expr
{
	PrefixExpr() : Expr(AstType::PREFIX), right(nullptr) {}
	PrefixExpr(std::string_view op, Expr *right) : Expr(AstType::PREFIX), op(op), right(right) {}
	~PrefixExpr()
	{
		delete right;
		right = nullptr;
	}

	std::string Stringify() override { return op + right->Stringify(); }

	std::string op;
	Expr *right;
};

struct InfixExpr : public Expr
{
	InfixExpr() : Expr(AstType::INFIX), left(nullptr), right(nullptr) {}
	InfixExpr(std::string_view op, Expr *left, Expr *right) : Expr(AstType::INFIX), op(op), left(left), right(right) {}
	~InfixExpr()
	{
		delete left;
		left = nullptr;

		delete right;
		right = nullptr;
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
	~IndexExpr()
	{
		delete ds;
		ds = nullptr;
		delete index;
		index = nullptr;
	}
	std::string Stringify() override { return ds->Stringify() + "[" + index->Stringify() + "]"; }

	Expr *ds;
	Expr *index;
};

struct RefExpr : public Expr
{
	RefExpr() : Expr(AstType::REF), refExpr(nullptr) {}
	RefExpr(Expr *refExpr) : Expr(AstType::REF), refExpr(refExpr) {}
	~RefExpr()
	{
		delete refExpr;
		refExpr = nullptr;
	}
	std::string Stringify() override { return "ref " + refExpr->Stringify(); }

	Expr *refExpr;
};

struct FunctionCallExpr : public Expr
{
	FunctionCallExpr() : Expr(AstType::FUNCTION_CALL), name(nullptr) {}
	FunctionCallExpr(Expr *name, std::vector<Expr *> arguments) : Expr(AstType::FUNCTION_CALL), name(name), arguments(arguments) {}
	~FunctionCallExpr()
	{
		delete name;
		name = nullptr;

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
	~StructCallExpr()
	{
		delete callee;
		callee = nullptr;
		delete callMember;
		callMember = nullptr;
	}

	std::string Stringify() override { return callee->Stringify() + "." + callMember->Stringify(); }

	Expr *callee;
	Expr *callMember;
};

struct DllImportExpr : public Expr
{
	DllImportExpr() : Expr(AstType::DLL_IMPORT) {}
	DllImportExpr(std::string_view path) : Expr(AstType::DLL_IMPORT), dllPath(path) {}
	~DllImportExpr()
	{
	}

	std::string Stringify() override { return "dllimport(\"" + dllPath + "\")"; }

	std::string dllPath;
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
	~ExprStmt()
	{
		delete expr;
		expr = nullptr;
	}

	std::string Stringify() override { return expr->Stringify() + ";"; }

	Expr *expr;
};

struct ReturnStmt : public Stmt
{
	ReturnStmt() : Stmt(AstType::RETURN), expr(nullptr) {}
	ReturnStmt(Expr *expr) : Stmt(AstType::RETURN), expr(expr) {}
	~ReturnStmt()
	{
		delete expr;
		expr = nullptr;
	}

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
	~IfStmt()
	{
		delete condition;
		condition = nullptr;
		delete thenBranch;
		thenBranch = nullptr;
		delete elseBranch;
		elseBranch = nullptr;
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
	~ScopeStmt() { std::vector<Stmt *>().swap(stmts); }

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
	~FunctionExpr()
	{
		std::vector<IdentifierExpr *>().swap(parameters);

		delete body;
		body = nullptr;
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

struct AnonyStructExpr : public Expr
{
	AnonyStructExpr(const std::unordered_map<IdentifierExpr *, Expr *> &memberPairs) : Expr(AstType::ANONY_STRUCT), memberPairs(memberPairs) {}
	~AnonyStructExpr() {}

	std::string Stringify() override
	{
		std::string result = "{";
		for (const auto &[k, v] : memberPairs)
			result += k->Stringify() + ":" + v->Stringify() + ",\n";
		result += "}";
		return result;
	}

	std::unordered_map<IdentifierExpr *, Expr *> memberPairs;
};

struct WhileStmt : public Stmt
{
	WhileStmt() : Stmt(AstType::WHILE), condition(nullptr), body(nullptr) {}
	WhileStmt(Expr *condition, Stmt *body) : Stmt(AstType::WHILE), condition(condition), body(body) {}
	~WhileStmt()
	{
		delete condition;
		condition = nullptr;
		delete body;
		body = nullptr;
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
	StructStmt() : Stmt(AstType::STRUCT) {}
	StructStmt(std::string_view name, std::vector<std::pair<IdentifierExpr *, Expr *>> members) : Stmt(AstType::STRUCT), name(name), members(members) {}
	~StructStmt()
	{
		std::vector<std::pair<IdentifierExpr *, Expr *>>().swap(members);
	}

	std::string Stringify() override
	{
		std::string result = "struct " + name + "{";

		for (const auto &member : members)
			result += member.first->Stringify() + ":" + member.second->Stringify() + "\n";

		return result + "}";
	}

	std::string name;
	std::vector<std::pair<IdentifierExpr *, Expr *>> members;
};

static NilExpr *nilExpr = new NilExpr();
static BoolExpr *trueExpr = new BoolExpr(true);
static BoolExpr *falseExpr = new BoolExpr(false);