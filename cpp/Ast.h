#pragma once
#include <string>

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
	LAMBDA,
	FUNCTION_CALL,
	STRUCT_CALL,
	//stmt
	VAR,
	EXPR,
	RETURN,
	IF,
	SCOPE,
	FUNCTION,
	WHILE,
	STRUCT,
};

struct AstNode
{
	AstNode() {}
	virtual ~AstNode() {}

	virtual std::string Stringify() = 0;
	virtual AstType Type() = 0;
};

struct Expr:public AstNode
{
	Expr() {}
	virtual ~Expr() {}

	virtual std::string Stringify() = 0;
	virtual AstType Type() = 0;
};

struct NumExpr : public Expr
{
	NumExpr() : value(0.0) {}
	NumExpr(double value) : value(value) {}
	~NumExpr() {}

	std::string Stringify() override { return std::to_string(value); }
	AstType Type() override { return AstType::NUM; }

	double value;
};

struct StrExpr : public Expr
{
	StrExpr() {}
	StrExpr(std::string_view str) : value(str) {}

	std::string Stringify() override { return "\"" + value + "\""; }
	AstType Type() override { return AstType::STR; }

	std::string value;
};

struct NilExpr : public Expr
{
	NilExpr() {}
	~NilExpr() {}

	std::string Stringify() override { return "nil"; }
	AstType Type() override { return AstType::NIL; }
};

struct BoolExpr : public Expr
{
	BoolExpr() : value(false) {}
	BoolExpr(bool value) : value(value) {}
	~BoolExpr() {}

	std::string Stringify() override { return value ? "true" : "false"; }
	AstType Type() override { return AstType::BOOL; }
	bool value;
};

struct IdentifierExpr : public Expr
{
	IdentifierExpr() {}
	IdentifierExpr(std::string_view literal) : literal(literal) {}
	~IdentifierExpr() {}

	std::string Stringify() override { return literal; }
	AstType Type() override { return AstType::IDENTIFIER; }

	std::string literal;
};

struct ArrayExpr : public Expr
{
	ArrayExpr() {}
	ArrayExpr(std::vector<Expr *> elements) : elements(elements) {}
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
	AstType Type() override { return AstType::ARRAY; }

	std::vector<Expr *> elements;
};

struct GroupExpr : public Expr
{
	GroupExpr() : expr(nullptr) {}
	GroupExpr(Expr *expr) : expr(expr) {}
	~GroupExpr() {}

	std::string Stringify() override { return "(" + expr->Stringify() + ")"; }
	AstType Type() override { return AstType::GROUP; }

	Expr *expr;
};

struct PrefixExpr : public Expr
{
	PrefixExpr() : right(nullptr) {}
	PrefixExpr(std::string_view op, Expr *right) : op(op), right(right) {}
	~PrefixExpr()
	{
		delete right;
		right = nullptr;
	}

	std::string Stringify() override { return op + right->Stringify(); }
	AstType Type() override { return AstType::PREFIX; }

	std::string op;
	Expr *right;
};

struct InfixExpr : public Expr
{
	InfixExpr() : left(nullptr), right(nullptr) {}
	InfixExpr(std::string_view op, Expr *left, Expr *right) : op(op), left(left), right(right) {}
	~InfixExpr()
	{
		delete left;
		left = nullptr;

		delete right;
		right = nullptr;
	}

	std::string Stringify() override { return left->Stringify() + op + right->Stringify(); }
	AstType Type() override { return AstType::INFIX; }

	std::string op;
	Expr *left;
	Expr *right;
};

struct IndexExpr : public Expr
{
	IndexExpr() : ds(nullptr), index(nullptr) {}
	IndexExpr(Expr *ds, Expr *index) : ds(ds), index(index) {}
	~IndexExpr()
	{
		delete ds;
		ds = nullptr;
		delete index;
		index = nullptr;
	}
	std::string Stringify() override { return ds->Stringify() + "[" + index->Stringify() + "]"; }

	AstType Type() override { return AstType::INDEX; }

	Expr *ds;
	Expr *index;
};

struct RefExpr:public Expr
{
	RefExpr() : refExpr(nullptr){}
	RefExpr(IdentifierExpr* refExpr) : refExpr(refExpr) {}
	~RefExpr()
	{
		delete refExpr;
		refExpr = nullptr;
	}
	std::string Stringify() override { return "ref "+refExpr->Stringify(); }

	AstType Type() override { return AstType::REF; }

	IdentifierExpr *refExpr;
};

struct FunctionCallExpr : public Expr
{
	FunctionCallExpr() {}
	FunctionCallExpr(std::string_view name, std::vector<Expr *> arguments) : name(name), arguments(arguments) {}
	~FunctionCallExpr() {}

	std::string Stringify() override
	{
		std::string result = name + "(";

		if (!arguments.empty())
		{
			for (const auto &arg : arguments)
				result += arg->Stringify() + ",";
			result = result.substr(0, result.size() - 1);
		}
		result += ")";
		return result;
	}
	AstType Type() override { return AstType::FUNCTION_CALL; }

	std::string name;
	std::vector<Expr *> arguments;
};

struct StructCallExpr :public Expr
{
	StructCallExpr():callee(nullptr),callMember(nullptr) {}
	StructCallExpr(Expr* callee, Expr* callMember) : callee(callee), callMember(callMember) {}
	~StructCallExpr() {}

	std::string Stringify() override { return callee->Stringify() + "." + callMember->Stringify(); }
	AstType Type() override { return AstType::STRUCT_CALL; }

	Expr* callee;
	Expr* callMember;
};

struct Stmt : public AstNode
{
	Stmt() {}
	virtual ~Stmt() {}

	virtual std::string Stringify() = 0;
	virtual AstType Type() = 0;
};

struct ExprStmt : public Stmt
{
	ExprStmt() : expr(nullptr) {}
	ExprStmt(Expr *expr) : expr(expr) {}
	~ExprStmt()
	{
		delete expr;
		expr = nullptr;
	}

	std::string Stringify() override { return expr->Stringify() + ";"; }
	AstType Type() override { return AstType::EXPR; }

	Expr *expr;
};

struct VarStmt : public Stmt
{
	VarStmt() {}
	VarStmt(IdentifierExpr *name, Expr *value) : name(name), value(value) {}
	~VarStmt()
	{
		delete name;
		delete value;
	}

	std::string Stringify() override
	{
		return "var " + name->Stringify() + "=" + value->Stringify();
	}

	AstType Type() override { return AstType::VAR; }

	IdentifierExpr *name;
	Expr *value;
};

struct ReturnStmt : public Stmt
{
	ReturnStmt() : expr(nullptr) {}
	ReturnStmt(Expr *expr) : expr(expr) {}
	~ReturnStmt()
	{
		delete expr;
		expr = nullptr;
	}

	std::string Stringify() override { return "return " + expr->Stringify() + ";"; }
	AstType Type() override { return AstType::RETURN; }

	Expr *expr;
};

struct IfStmt : public Stmt
{
	IfStmt() : condition(nullptr), thenBranch(nullptr), elseBranch(nullptr) {}
	IfStmt(Expr *condition, Stmt *thenBranch, Stmt *elseBranch)
		: condition(condition),
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
	AstType Type() override { return AstType::IF; }

	Expr *condition;
	Stmt *thenBranch;
	Stmt *elseBranch;
};

struct ScopeStmt : public Stmt
{
	ScopeStmt() {}
	ScopeStmt(std::vector<Stmt *> stmts) : stmts(stmts) {}
	~ScopeStmt() { std::vector<Stmt *>().swap(stmts); }

	std::string Stringify() override
	{
		std::string result = "{";
		for (const auto &stmt : stmts)
			result += stmt->Stringify();
		result += "}";
		return result;
	}

	AstType Type() override { return AstType::SCOPE; }
	std::vector<Stmt *> stmts;
};

struct FunctionStmt : public Stmt
{
	FunctionStmt() : body(nullptr) {}
	FunctionStmt(std::string_view name, std::vector<IdentifierExpr *> parameters, ScopeStmt *body) : name(name),parameters(parameters), body(body) {}
	~FunctionStmt()
	{
		std::vector<IdentifierExpr *>().swap(parameters);

		delete body;
		body = nullptr;
	}

	std::string Stringify() override
	{
		std::string result = "fn " + name + "(";
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
	AstType Type() override { return AstType::FUNCTION; }

	std::string name;
	std::vector<IdentifierExpr *> parameters;
	ScopeStmt *body;
};

struct LambdaExpr : public Expr
{
	LambdaExpr() : body(nullptr) {}
	LambdaExpr(std::string_view name, std::vector<IdentifierExpr *> parameters, ScopeStmt *body) : parameters(parameters), body(body) {}
	~LambdaExpr()
	{
		std::vector<IdentifierExpr *>().swap(parameters);

		delete body;
		body = nullptr;
	}

	std::string Stringify() override
	{
		std::string result = "lambda(";
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
	AstType Type() override { return AstType::LAMBDA; }

	std::vector<IdentifierExpr *> parameters;
	ScopeStmt *body;
};

struct WhileStmt : public Stmt
{
	WhileStmt() : condition(nullptr), body(nullptr) {}
	WhileStmt(Expr *condition, Stmt *body) : condition(condition), body(body) {}
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
	AstType Type() override { return AstType::WHILE; }

	Expr *condition;
	Stmt *body;
};

struct StructStmt : public Stmt
{
	StructStmt() {}
	StructStmt(std::string_view name, std::vector<VarStmt *> members) : name(name), members(members) {}
	~StructStmt()
	{
		std::vector<VarStmt *>().swap(members);
	}

	std::string Stringify() override
	{
		std::string result = "struct " + name + "{";

		for (const auto &varStmt : members)
			result += varStmt->Stringify() + "\n";

		return result + "}";
	}
	AstType Type() override { return AstType::STRUCT; }

	std::string name;
	std::vector<VarStmt *> members;
};

static NilExpr *nilExpr = new NilExpr();
static BoolExpr *trueExpr = new BoolExpr(true);
static BoolExpr *falseExpr = new BoolExpr(false);