#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Ast.h"
#include "Utils.h"
#include "ConstantFolder.h"

enum class Precedence
{
	LOWEST = 0, // ,
	ASSIGN,		// =
	OR,			// or
	AND,		// and
	BIT_OR,		// |
	BIT_XOR,	// ^
	BIT_AND,	// &
	EQUAL,		// == !=
	COMPARE,	// < <= > >=
	ADD_PLUS,	// + -
	MUL_DIV,	// * /
	UNARY,		// not - ~
	CALL,		// [] () .
};

class Parser;

typedef Expr *(Parser::*PrefixFn)();
typedef Expr *(Parser::*InfixFn)(Expr *);

class COMPUTE_DUCK_API Parser
{
public:
	Parser();
	~Parser();

	std::vector<Stmt *> Parse(const std::vector<Token> &tokens);

private:
	Stmt *ParseStmt();
	Stmt *ParseExprStmt();
	Stmt *ParseReturnStmt();
	Stmt *ParseIfStmt();
	Stmt *ParseScopeStmt();
	Stmt *ParseWhileStmt();
	Stmt *ParseStructStmt();
	Stmt *ParseDllImportStmt();

	Expr *ParseExpr(Precedence precedence = Precedence::LOWEST);
	Expr *ParseIdentifierExpr();
	Expr *ParseNumExpr();
	Expr *ParseStrExpr();
	Expr *ParseNilExpr();
	Expr *ParseBoolExpr();
	Expr *ParseGroupExpr();
	Expr *ParseArrayExpr();
	Expr *ParseUnaryExpr();
	Expr *ParseRefExpr();
	Expr *ParseFunctionExpr();
	Expr *ParseStructExpr();
	Expr *ParseBinaryExpr(Expr *unaryExpr);
	Expr *ParseIndexExpr(Expr *unaryExpr);
	Expr *ParseFunctionCallExpr(Expr *unaryExpr);
	Expr *ParseStructCallExpr(Expr *unaryExpr);

	Token GetCurToken();
	Token GetCurTokenAndStepOnce();
	Precedence GetCurTokenPrecedence();

	Token GetNextToken();
	Token GetNextTokenAndStepOnce();
	Precedence GetNextTokenPrecedence();

	bool IsMatchCurToken(TokenType type);
	bool IsMatchCurTokenAndStepOnce(TokenType type);
	bool IsMatchNextToken(TokenType type);
	bool IsMatchNextTokenAndStepOnce(TokenType type);

	Token Consume(TokenType type, std::string_view errMsg);

	bool IsAtEnd();

	int64_t m_CurPos;

	std::vector<Token> m_Tokens;

	int32_t m_FunctionScopeDepth;

	ConstantFolder m_ConstantFolder;

	static std::unordered_map<TokenType, PrefixFn> m_PrefixFunctions;
	static std::unordered_map<TokenType, InfixFn> m_InfixFunctions;
	static std::unordered_map<TokenType, Precedence> m_Precedence;
};