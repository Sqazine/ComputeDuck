#pragma once
#include <vector>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Ast.h"
#include "Utils.h"

enum class Precedence
{
	LOWEST = 0, // ,
	ASSIGN,		// =
	OR,			// or
	AND,		// and
	EQUAL,		// == !=
	COMPARE,	// < <= > >=
	ADD_PLUS,	// + -
	MUL_DIV,	// * /
	PREFIX,		// !
	INFIX,		// [] () .
};

class Parser;

typedef Expr *(Parser::*PrefixFn)();
typedef Expr *(Parser::*InfixFn)(Expr *);

class Parser
{
public:
	Parser();
	~Parser();

	std::vector<Stmt *> Parse(const std::vector<Token> &tokens);

private:

	Stmt *ParseStmt();
	Stmt *ParseExprStmt();
	Stmt *ParseVarStmt();
	Stmt *ParseReturnStmt();
	Stmt *ParseIfStmt();
	Stmt *ParseScopeStmt();
	Stmt *ParseWhileStmt();
	Stmt *ParseFunctionStmt();
	Stmt *ParseStructStmt();

	Expr *ParseExpr(Precedence precedence = Precedence::LOWEST);
	Expr *ParseIdentifierExpr();
	Expr *ParseNumExpr();
	Expr *ParseStrExpr();
	Expr *ParseNilExpr();
	Expr *ParseTrueExpr();
	Expr *ParseFalseExpr();
	Expr *ParseGroupExpr();
	Expr *ParseArrayExpr();
	Expr *ParsePrefixExpr();
	Expr *ParseInfixExpr(Expr *prefixExpr);
	Expr *ParseConditionExpr(Expr *prefixExpr);
	Expr *ParseIndexExpr(Expr *prefixExpr);
	Expr *ParseFunctionCallExpr(Expr *prefixExpr);
	Expr *ParseStructCallExpr(Expr *prefixExpr);

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
	Token Consume(const std::vector<TokenType> &types, std::string_view errMsg);

	bool IsAtEnd();

	int64_t m_CurPos;

	std::vector<Token> m_Tokens;

	static std::unordered_map<TokenType, PrefixFn> m_PrefixFunctions;
	static std::unordered_map<TokenType, InfixFn> m_InfixFunctions;
	static std::unordered_map<TokenType, Precedence> m_Precedence;
};