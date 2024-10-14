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
		{TokenType::LBRACKET, &Parser::ParseArrayExpr},
		{TokenType::TILDE, &Parser::ParseUnaryExpr},
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
		{TokenType::LBRACKET, &Parser::ParseIndexExpr},
		{TokenType::AND, &Parser::ParseBinaryExpr},
		{TokenType::OR, &Parser::ParseBinaryExpr},
		{TokenType::AMPERSAND, &Parser::ParseBinaryExpr},
		{TokenType::VBAR, &Parser::ParseBinaryExpr},
		{TokenType::CARET, &Parser::ParseBinaryExpr},
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
		{TokenType::LBRACKET, Precedence::CALL},
		{TokenType::AND, Precedence::AND},
		{TokenType::OR, Precedence::OR},
		{TokenType::AMPERSAND, Precedence::BIT_AND},
		{TokenType::VBAR, Precedence::BIT_OR},
		{TokenType::CARET, Precedence::BIT_XOR},
};

Parser::Parser()
	: m_CurPos(0)
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

	std::vector<Stmt *> stmts;
	while (!IsMatchCurToken(TokenType::END))
		stmts.emplace_back(ParseStmt());

	return stmts;
}

Stmt *Parser::ParseStmt()
{
	// ++ 新增内容
	if (IsMatchCurToken(TokenType::PRINT))
		return ParsePrintStmt();
	else
	// -- 新增内容
		return ParseExprStmt();
}

Stmt *Parser::ParseExprStmt()
{
	auto exprStmt = new ExprStmt(ParseExpr());
	Consume(TokenType::SEMICOLON, "Expect ';' after expr stmt.");

	return exprStmt;
}
// ++ 新增内容
Stmt *Parser::ParsePrintStmt()
{
	Consume(TokenType::PRINT, "Expect 'print' keyword.");
	auto printStmt = new PrintStmt(ParseExpr());
	Consume(TokenType::SEMICOLON, "Expect ';' after expr stmt.");

	return printStmt;
}
// -- 新增内容

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
	indexExpr->index = ParseExpr(Precedence::CALL);
	Consume(TokenType::RBRACKET, "Expect ']'.");
	return indexExpr;
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