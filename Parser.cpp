#include "Parser.h"

std::unordered_map<TokenType, PrefixFn> Parser::m_PrefixFunctions =
	{
		{TokenType::IDENTIFIER, &Parser::ParseIdentifierExpr},
		{TokenType::NUMBER, &Parser::ParseNumExpr},
		{TokenType::STRING, &Parser::ParseStrExpr},
		{TokenType::NIL, &Parser::ParseNilExpr},
		{TokenType::TRUE, &Parser::ParseTrueExpr},
		{TokenType::FALSE, &Parser::ParseFalseExpr},
		{TokenType::MINUS, &Parser::ParsePrefixExpr},
		{TokenType::NOT, &Parser::ParsePrefixExpr},
		{TokenType::LPAREN, &Parser::ParseGroupExpr},
		{TokenType::LBRACKET, &Parser::ParseArrayExpr},
		{TokenType::REF, &Parser::ParseRefExpr},
		{TokenType::FUNCTION, &Parser::ParseFunctionExpr},
		{TokenType::LBRACE, &Parser::ParseAnonyStructExpr},
		{TokenType::DLLIMPORT, &Parser::ParseDllImportExpr},
		{TokenType::TILDE, &Parser::ParsePrefixExpr},
};

std::unordered_map<TokenType, InfixFn> Parser::m_InfixFunctions =
	{
		{TokenType::EQUAL, &Parser::ParseInfixExpr},
		{TokenType::EQUAL_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::BANG_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::LESS, &Parser::ParseInfixExpr},
		{TokenType::LESS_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::GREATER, &Parser::ParseInfixExpr},
		{TokenType::GREATER_EQUAL, &Parser::ParseInfixExpr},
		{TokenType::PLUS, &Parser::ParseInfixExpr},
		{TokenType::MINUS, &Parser::ParseInfixExpr},
		{TokenType::ASTERISK, &Parser::ParseInfixExpr},
		{TokenType::SLASH, &Parser::ParseInfixExpr},
		{TokenType::LPAREN, &Parser::ParseFunctionCallExpr},
		{TokenType::LBRACKET, &Parser::ParseIndexExpr},
		{TokenType::AND, &Parser::ParseInfixExpr},
		{TokenType::OR, &Parser::ParseInfixExpr},
		{TokenType::DOT, &Parser::ParseStructCallExpr},
		{TokenType::AMPERSAND, &Parser::ParseInfixExpr},
		{TokenType::VBAR, &Parser::ParseInfixExpr},
		{TokenType::CARET, &Parser::ParseInfixExpr},
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
		{TokenType::PLUS, Precedence::ADD_PLUS},
		{TokenType::MINUS, Precedence::ADD_PLUS},
		{TokenType::ASTERISK, Precedence::MUL_DIV},
		{TokenType::SLASH, Precedence::MUL_DIV},
		{TokenType::LBRACKET, Precedence::INFIX},
		{TokenType::LPAREN, Precedence::INFIX},
		{TokenType::AND, Precedence::AND},
		{TokenType::OR, Precedence::OR},
		{TokenType::DOT, Precedence::INFIX},
		{TokenType::AMPERSAND, Precedence::BIT_AND},
		{TokenType::VBAR, Precedence::BIT_OR},
		{TokenType::CARET, Precedence::BIT_XOR},
};

Parser::Parser()
	:m_CurPos(0), m_FunctionOrFunctionScopeDepth(0)
{
}
Parser::~Parser()
{
	std::unordered_map<TokenType, PrefixFn>().swap(m_PrefixFunctions);
	std::unordered_map<TokenType, InfixFn>().swap(m_InfixFunctions);
	std::unordered_map<TokenType, Precedence>().swap(m_Precedence);
}

std::vector<Stmt *> Parser::Parse(const std::vector<Token> &tokens)
{
	m_CurPos = 0;
	m_Tokens = tokens;
	m_FunctionOrFunctionScopeDepth = 0;

	std::vector<Stmt *> stmts;
	while (!IsMatchCurToken(TokenType::END))
		stmts.emplace_back(ParseStmt());

	m_ConstantFolder.Fold(stmts);

	return stmts;
}

Stmt *Parser::ParseStmt()
{
	if (IsMatchCurToken(TokenType::RETURN))
		return ParseReturnStmt();
	else if (IsMatchCurToken(TokenType::IF))
		return ParseIfStmt();
	else if (IsMatchCurToken(TokenType::LBRACE))
		return ParseScopeStmt();
	else if (IsMatchCurToken(TokenType::WHILE))
		return ParseWhileStmt();
	else if (IsMatchCurToken(TokenType::STRUCT))
		return ParseStructStmt();
	else
		return ParseExprStmt();
}

Stmt *Parser::ParseExprStmt()
{
	auto exprStmt = new ExprStmt(ParseExpr());
	Consume(TokenType::SEMICOLON, "Expect ';' after expr stmt.");

	return exprStmt;
}

Stmt *Parser::ParseReturnStmt()
{
	if (m_FunctionOrFunctionScopeDepth == 0)
		ASSERT("Return statement only available in function");

	Consume(TokenType::RETURN, "Expect 'return' key word.");

	auto returnStmt = new ReturnStmt();

	if (!IsMatchCurToken(TokenType::SEMICOLON))
		returnStmt->expr = ParseExpr();

	Consume(TokenType::SEMICOLON, "Expect ';' after return stmt");

	return returnStmt;
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

Stmt *Parser::ParseScopeStmt()
{
	Consume(TokenType::LBRACE, "Expect '{'.");
	auto scopeStmt = new ScopeStmt();
	while (!IsMatchCurToken(TokenType::RBRACE))
		scopeStmt->stmts.emplace_back(ParseStmt());
	Consume(TokenType::RBRACE, "Expect '}'.");

	return scopeStmt;
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
		structStmt->members.emplace_back(k, v);
	}

	Consume(TokenType::RBRACE, "Expect '}'.");

	return structStmt;
}

Expr *Parser::ParseExpr(Precedence precedence)
{
	if (m_PrefixFunctions.find(GetCurToken().type) == m_PrefixFunctions.end())
	{
		ASSERT("no prefix definition for:%s", GetCurTokenAndStepOnce().literal.c_str());
		return new NilExpr();
	}
	auto prefixFn = m_PrefixFunctions[GetCurToken().type];

	auto leftExpr = (this->*prefixFn)();

	while (!IsMatchCurToken(TokenType::SEMICOLON) && precedence < GetCurTokenPrecedence())
	{
		if (m_InfixFunctions.find(GetCurToken().type) == m_InfixFunctions.end())
			return leftExpr;

		auto infixFn = m_InfixFunctions[GetCurToken().type];

		leftExpr = (this->*infixFn)(leftExpr);
	}

	return leftExpr;
}

Expr *Parser::ParseIdentifierExpr()
{
	return new IdentifierExpr(Consume(TokenType::IDENTIFIER, "Unexpect Identifier'" + GetCurToken().literal + "'.").literal);
}

Expr *Parser::ParseNumExpr()
{
	std::string numLiteral = Consume(TokenType::NUMBER, "Expect a number literal.").literal;
	return new NumExpr(std::stod(numLiteral));
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
Expr *Parser::ParseTrueExpr()
{
	Consume(TokenType::TRUE, "Expect 'true' keyword");
	return new BoolExpr(true);
}
Expr *Parser::ParseFalseExpr()
{
	Consume(TokenType::FALSE, "Expect 'false' keyword");
	return new BoolExpr(false);
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

Expr *Parser::ParsePrefixExpr()
{
	auto prefixExpr = new PrefixExpr();
	prefixExpr->op = GetCurTokenAndStepOnce().literal;
	prefixExpr->right = ParseExpr(Precedence::PREFIX);
	return prefixExpr;
}

Expr *Parser::ParseInfixExpr(Expr *prefixExpr)
{
	auto infixExpr = new InfixExpr();
	infixExpr->left = prefixExpr;

	Precedence opPrece = GetCurTokenPrecedence();

	infixExpr->op = GetCurTokenAndStepOnce().literal;
	infixExpr->right = ParseExpr(opPrece);
	return infixExpr;
}

Expr *Parser::ParseIndexExpr(Expr *prefixExpr)
{
	Consume(TokenType::LBRACKET, "Expect '['.");
	auto indexExpr = new IndexExpr();
	indexExpr->ds = prefixExpr;
	indexExpr->index = ParseExpr(Precedence::INFIX);
	Consume(TokenType::RBRACKET, "Expect ']'.");
	return indexExpr;
}

Expr *Parser::ParseRefExpr()
{
	Consume(TokenType::REF, "Expect 'ref' keyword.");

	auto refExpr = ParseExpr(Precedence::LOWEST);

	return new RefExpr(refExpr);
}

Expr *Parser::ParseFunctionExpr()
{
	m_FunctionOrFunctionScopeDepth++;

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

	m_FunctionOrFunctionScopeDepth--;

	return functionExpr;
}

Expr *Parser::ParseAnonyStructExpr()
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
	return new AnonyStructExpr(memPairs);
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

Expr *Parser::ParseStructCallExpr(Expr *prefixExpr)
{
	Consume(TokenType::DOT, "Expect '.'.");
	auto structCallExpr = new StructCallExpr();
	structCallExpr->callee = prefixExpr;
	structCallExpr->callMember = ParseExpr(Precedence::INFIX);
	return structCallExpr;
}

Expr *Parser::ParseDllImportExpr()
{
	Consume(TokenType::DLLIMPORT, "Expect 'dllimport' keyword");
	Consume(TokenType::LPAREN, "Expect '(' after 'dllimport' keyword");

	auto path = Consume(TokenType::STRING, "Expect dll path.").literal;

	Consume(TokenType::RPAREN, "Expect ')' after dllimport expr");

	if (path.find(".") == std::string::npos) // no file suffix
	{
#ifdef _WIN32
		path += ".dll";
#elif __linux__
		path += ".so";
#elif __APPLE__
		path += ".dylib";
#endif
	}

	return new DllImportExpr(path);
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

Token Parser::GetNextToken()
{
	if (m_CurPos + 1 < (int32_t)m_Tokens.size())
		return m_Tokens[m_CurPos + 1];
	return m_Tokens.back();
}
Token Parser::GetNextTokenAndStepOnce()
{
	if (m_CurPos + 1 < (int32_t)m_Tokens.size())
		return m_Tokens[++m_CurPos];
	return m_Tokens.back();
}

Precedence Parser::GetNextTokenPrecedence()
{
	if (m_Precedence.find(GetNextToken().type) != m_Precedence.end())
		return m_Precedence[GetNextToken().type];
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

bool Parser::IsMatchNextToken(TokenType type)
{
	return GetNextToken().type == type;
}

bool Parser::IsMatchNextTokenAndStepOnce(TokenType type)
{
	if (IsMatchNextToken(type))
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
	ASSERT("[file %s line %lld]:%s", GetCurToken().filePath.c_str(), GetCurToken().line, errMsg.data());
}

bool Parser::IsAtEnd()
{
	return m_CurPos >= (int32_t)m_Tokens.size();
}