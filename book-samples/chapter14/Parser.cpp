#include "Parser.h"

std::unordered_map<TokenType, UnaryFn> Parser::m_UnaryFunctions =
	{
		{TokenType::IDENTIFIER, &Parser::ParseIdentifierExpr},
		{TokenType::NUMBER, &Parser::ParseNumExpr},
		{TokenType::NIL, &Parser::ParseNilExpr},
		{TokenType::STRING, &Parser::ParseStrExpr},
		{TokenType::TRUE, &Parser::ParseBoolExpr},
		{TokenType::FALSE, &Parser::ParseBoolExpr},
		{TokenType::MINUS, &Parser::ParseUnaryExpr},
		{TokenType::NOT, &Parser::ParseUnaryExpr},
		{TokenType::LPAREN, &Parser::ParseGroupExpr},
		{TokenType::FUNCTION, &Parser::ParseFunctionExpr},
		{TokenType::LBRACKET, &Parser::ParseArrayExpr},
		{TokenType::TILDE, &Parser::ParseUnaryExpr},
		{TokenType::LBRACE, &Parser::ParseStructExpr},
		// ++ 新增内容
		{TokenType::REF, &Parser::ParseRefExpr},
		// -- 新增内容
};

std::unordered_map<TokenType, BinaryFn> Parser::m_BinaryFunctions =
	{
		{TokenType::EQUAL, &Parser::ParseBinaryExpr},
		{TokenType::EQUAL_EQUAL, &Parser::ParseBinaryExpr},
		{TokenType::BANG_EQUAL, &Parser::ParseBinaryExpr},
		{TokenType::LESS, &Parser::ParseBinaryExpr},
		{TokenType::LESS_EQUAL, &Parser::ParseBinaryExpr},
		{TokenType::GREATER, &Parser::ParseBinaryExpr},
		{TokenType::GREATER_EQUAL, &Parser::ParseBinaryExpr},
		{TokenType::PLUS, &Parser::ParseBinaryExpr},
		{TokenType::MINUS, &Parser::ParseBinaryExpr},
		{TokenType::ASTERISK, &Parser::ParseBinaryExpr},
		{TokenType::SLASH, &Parser::ParseBinaryExpr},
		{TokenType::LPAREN, &Parser::ParseFunctionCallExpr},
		{TokenType::LBRACKET, &Parser::ParseIndexExpr},
		{TokenType::AND, &Parser::ParseBinaryExpr},
		{TokenType::OR, &Parser::ParseBinaryExpr},
		{TokenType::AMPERSAND, &Parser::ParseBinaryExpr},
		{TokenType::VBAR, &Parser::ParseBinaryExpr},
		{TokenType::CARET, &Parser::ParseBinaryExpr},
		{TokenType::DOT, &Parser::ParseStructCallExpr},
};

std::unordered_map<TokenType, Precedence> Parser::m_Precedence =
	{
		{TokenType::EQUAL, Precedence::ASSIGN},
		{TokenType::EQUAL_EQUAL, Precedence::EQUAL},
		{TokenType::BANG_EQUAL, Precedence::EQUAL},
		{TokenType::LESS, Precedence::COMPARE},
		{TokenType::LESS_EQUAL, Precedence::COMPARE},
		{TokenType::GREATER, Precedence::COMPARE},
		{TokenType::GREATER_EQUAL, Precedence::COMPARE},
		{TokenType::PLUS, Precedence::ADD_SUB},
		{TokenType::MINUS, Precedence::ADD_SUB},
		{TokenType::ASTERISK, Precedence::MUL_DIV},
		{TokenType::SLASH, Precedence::MUL_DIV},
		{TokenType::LBRACKET, Precedence::BINARY},
		{TokenType::LPAREN, Precedence::BINARY},
		{TokenType::AND, Precedence::AND},
		{TokenType::OR, Precedence::OR},
		{TokenType::AMPERSAND, Precedence::BIT_AND},
		{TokenType::VBAR, Precedence::BIT_OR},
		{TokenType::CARET, Precedence::BIT_XOR},
		{TokenType::DOT, Precedence::BINARY},
};

Parser::Parser()
	: m_CurPos(0)
	
	, m_FunctionScopeDepth(0)
	
{
}
Parser::~Parser()
{
	std::unordered_map<TokenType, UnaryFn>().swap(m_UnaryFunctions);
	std::unordered_map<TokenType, BinaryFn>().swap(m_BinaryFunctions);
	std::unordered_map<TokenType, Precedence>().swap(m_Precedence);
}

std::vector<Stmt *> Parser::Parse(const std::vector<Token> &tokens)
{
	m_CurPos = 0;
	m_Tokens = tokens;

	
	m_FunctionScopeDepth = 0;
	

	std::vector<Stmt *> stmts;
	while (!IsMatchCurToken(TokenType::END))
		stmts.emplace_back(ParseStmt());

	return stmts;
}

Stmt *Parser::ParseStmt()
{
	if (IsMatchCurToken(TokenType::LBRACE))
		return ParseScopeStmt();
	else if (IsMatchCurToken(TokenType::IF))
		return ParseIfStmt();
	else if (IsMatchCurToken(TokenType::RETURN))
		return ParseReturnStmt();
	else if (IsMatchCurToken(TokenType::WHILE))
		return ParseWhileStmt();
	else if (IsMatchCurToken(TokenType::STRUCT))
		return ParseStructStmt();
	else
		return ParseExprStmt();
}

Stmt *Parser::ParseReturnStmt()
{
	if (m_FunctionScopeDepth == 0)
		ASSERT("Return statement only available in function");

	Consume(TokenType::RETURN, "Expect 'return' key word.");

	auto returnStmt = new ReturnStmt();

	if (!IsMatchCurToken(TokenType::SEMICOLON))
		returnStmt->expr = ParseExpr();

	Consume(TokenType::SEMICOLON, "Expect ';' after return stmt");

	return returnStmt;
}


Stmt *Parser::ParseExprStmt()
{
	auto exprStmt = new ExprStmt(ParseExpr());
	Consume(TokenType::SEMICOLON, "Expect ';' after expr stmt.");

	return exprStmt;
}

Stmt *Parser::ParseScopeStmt()
{
	Consume(TokenType::LBRACE, "Expect '{'.");
	auto scopeStmt = new ScopeStmt();
	while (!IsMatchCurToken(TokenType::RBRACE))
		scopeStmt->stmts.emplace_back(ParseStmt());
	Consume(TokenType::RBRACE, "Expect '}'.");

	return scopeStmt;
}


Stmt *Parser::ParseIfStmt()
{
	Consume(TokenType::IF, "Expect 'if' key word.");
	Consume(TokenType::LPAREN, "Expect '(' after 'if'.");

	auto ifStmt = new IfStmt();

	ifStmt->condition = ParseExpr();

	Consume(TokenType::RPAREN, "Expect ')' after if condition");

	ifStmt->thenBranch = ParseStmt();

	if (IsMatchCurTokenAndStepOnce(TokenType::ELSE))
		ifStmt->elseBranch = ParseStmt();

	return ifStmt;
}

Stmt *Parser::ParseWhileStmt()
{
	Consume(TokenType::WHILE, "Expect 'while' keyword.");
	Consume(TokenType::LPAREN, "Expect '(' after 'while'.");

	auto whileStmt = new WhileStmt();

	whileStmt->condition = ParseExpr();

	Consume(TokenType::RPAREN, "Expect ')' after while stmt's condition.");

	whileStmt->body = ParseStmt();

	return whileStmt;
}

Stmt *Parser::ParseStructStmt()
{
	Consume(TokenType::STRUCT, "Expect 'struct' keyword");

	auto structStmt = new StructStmt();

	structStmt->name = ParseIdentifierExpr()->Stringify();
	structStmt->body = (StructExpr *)ParseStructExpr();

	return structStmt;
}

Expr *Parser::ParseExpr(Precedence precedence)
{
	if (m_UnaryFunctions.find(GetCurToken().type) == m_UnaryFunctions.end())
	{
		ASSERT("no unary definition for:%s", GetCurTokenAndStepOnce().literal.c_str());
		return new NilExpr();
	}
	auto unaryFn = m_UnaryFunctions[GetCurToken().type];

	auto expr = (this->*unaryFn)();

	while (!IsMatchCurToken(TokenType::SEMICOLON) && precedence < GetCurTokenPrecedence())
	{
		if (m_BinaryFunctions.find(GetCurToken().type) == m_BinaryFunctions.end())
			return expr;

		auto binaryFn = m_BinaryFunctions[GetCurToken().type];

		expr = (this->*binaryFn)(expr);
	}

	return expr;
}

Expr *Parser::ParseIdentifierExpr()
{
	return new IdentifierExpr(Consume(TokenType::IDENTIFIER, "Unexpect Identifier'" + GetCurToken().literal + "'.").literal);
}

Expr *Parser::ParseNumExpr()
{
	return new NumExpr(std::stod(Consume(TokenType::NUMBER, "Expect a number literal.").literal));
}

Expr *Parser::ParseStrExpr()
{
	return new StrExpr(Consume(TokenType::STRING, "Expect a string literal.").literal);
}

Expr *Parser::ParseNilExpr()
{
	Consume(TokenType::NIL, "Expect 'nil' keyword");
	return new NilExpr();
}
Expr *Parser::ParseBoolExpr()
{
	bool flag = false;
	if (IsMatchCurToken(TokenType::TRUE))
		flag = true;
	else if (IsMatchCurToken(TokenType::FALSE))
		flag = false;

	GetCurTokenAndStepOnce();

	return new BoolExpr(flag);
}

Expr *Parser::ParseGroupExpr()
{
	Consume(TokenType::LPAREN, "Expect '('.");
	auto groupExpr = new GroupExpr(ParseExpr());
	Consume(TokenType::RPAREN, "Expect ')'.");
	return groupExpr;
}

Expr *Parser::ParseArrayExpr()
{
	Consume(TokenType::LBRACKET, "Expect '['.");

	auto arrayExpr = new ArrayExpr();
	if (!IsMatchCurToken(TokenType::RBRACKET))
	{
		// first element
		arrayExpr->elements.emplace_back(ParseExpr());
		while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
			arrayExpr->elements.emplace_back(ParseExpr());
	}

	Consume(TokenType::RBRACKET, "Expect ']'.");

	return arrayExpr;
}

Expr *Parser::ParseUnaryExpr()
{
	auto unaryExpr = new UnaryExpr();
	unaryExpr->op = GetCurTokenAndStepOnce().literal;
	unaryExpr->right = ParseExpr(Precedence::UNARY);
	return unaryExpr;
}


Expr *Parser::ParseFunctionExpr()
{
	m_FunctionScopeDepth++;

	Consume(TokenType::FUNCTION, "Expect 'function' keyword");

	auto functionExpr = new FunctionExpr();

	Consume(TokenType::LPAREN, "Expect '(' after keyword 'function'");

	if (!IsMatchCurToken(TokenType::RPAREN)) // has parameter
	{
		IdentifierExpr *idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
		functionExpr->parameters.emplace_back(idenExpr);
		while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
		{
			idenExpr = (IdentifierExpr *)ParseIdentifierExpr();
			functionExpr->parameters.emplace_back(idenExpr);
		}
	}
	Consume(TokenType::RPAREN, "Expect ')' after function expr's '('");

	functionExpr->body = (ScopeStmt *)ParseScopeStmt();

	m_FunctionScopeDepth--;

	return functionExpr;
}

Expr *Parser::ParseFunctionCallExpr(Expr *prefixExpr)
{
	auto funcCallExpr = new FunctionCallExpr();

	funcCallExpr->name = prefixExpr;
	Consume(TokenType::LPAREN, "Expect '('.");
	if (!IsMatchCurToken(TokenType::RPAREN)) // has arguments
	{
		funcCallExpr->arguments.emplace_back(ParseExpr());
		while (IsMatchCurTokenAndStepOnce(TokenType::COMMA))
			funcCallExpr->arguments.emplace_back(ParseExpr());
	}
	Consume(TokenType::RPAREN, "Expect ')'.");

	return funcCallExpr;
}


Expr *Parser::ParseBinaryExpr(Expr *unaryExpr)
{
	auto binaryExpr = new BinaryExpr();
	binaryExpr->left = unaryExpr;

	Precedence opPrece = GetCurTokenPrecedence();

	binaryExpr->op = GetCurTokenAndStepOnce().literal;
	binaryExpr->right = ParseExpr(opPrece);
	return binaryExpr;
}

Expr *Parser::ParseIndexExpr(Expr *unaryExpr)
{
	Consume(TokenType::LBRACKET, "Expect '['.");
	auto indexExpr = new IndexExpr();
	indexExpr->ds = unaryExpr;
	indexExpr->index = ParseExpr(Precedence::BINARY);
	Consume(TokenType::RBRACKET, "Expect ']'.");
	return indexExpr;
}

Expr *Parser::ParseStructExpr()
{
	std::unordered_map<IdentifierExpr *, Expr *> memPairs;
	Consume(TokenType::LBRACE, "Expect '{'.");
	while (!IsMatchCurToken(TokenType::RBRACE))
	{
		auto k = (IdentifierExpr *)ParseIdentifierExpr();
		Expr *v = new NilExpr();
		if (IsMatchCurToken(TokenType::COLON))
		{
			Consume(TokenType::COLON, "Expect ':'");
			v = ParseExpr();
		}
		IsMatchCurTokenAndStepOnce(TokenType::COMMA);
		memPairs[k] = v;
	}

	Consume(TokenType::RBRACE, "Expect '}'.");
	return new StructExpr(memPairs);
}

// ++ 新增内容
Expr *Parser::ParseRefExpr()
{
	Consume(TokenType::REF, "Expect 'ref' keyword.");

	auto refExpr = ParseExpr(Precedence::LOWEST);

	return new RefExpr(refExpr);
}
// -- 新增内容


Expr *Parser::ParseStructCallExpr(Expr *prefixExpr)
{
	Consume(TokenType::DOT, "Expect '.'.");
	auto structCallExpr = new StructCallExpr();
	structCallExpr->callee = prefixExpr;
	structCallExpr->callMember = (IdentifierExpr *)ParseIdentifierExpr();
	return structCallExpr;
}

Token Parser::GetCurToken()
{
	if (!IsAtEnd())
		return m_Tokens[m_CurPos];
	return m_Tokens.back();
}
Token Parser::GetCurTokenAndStepOnce()
{
	if (!IsAtEnd())
		return m_Tokens[m_CurPos++];
	return m_Tokens.back();
}

Precedence Parser::GetCurTokenPrecedence()
{
	if (m_Precedence.find(GetCurToken().type) != m_Precedence.end())
		return m_Precedence[GetCurToken().type];
	return Precedence::LOWEST;
}

bool Parser::IsMatchCurToken(TokenType type)
{
	return GetCurToken().type == type;
}

bool Parser::IsMatchCurTokenAndStepOnce(TokenType type)
{
	if (IsMatchCurToken(type))
	{
		m_CurPos++;
		return true;
	}
	return false;
}

Token Parser::Consume(TokenType type, std::string_view errMsg)
{
	if (IsMatchCurToken(type))
		return GetCurTokenAndStepOnce();
	ASSERT("[file %s line %u]:%s", GetCurToken().filePath.c_str(), GetCurToken().line, errMsg.data());
}

bool Parser::IsAtEnd()
{
	return m_CurPos >= (int32_t)m_Tokens.size();
}