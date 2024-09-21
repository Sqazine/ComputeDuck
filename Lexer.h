#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "Token.h"
#include "Utils.h"

class COMPUTE_DUCK_API Lexer
{
public:
public:
	Lexer();
	~Lexer();

	const std::vector<Token> &GenerateTokens(std::string_view src, std::string_view filePath = "RootFile");

private:
	void ResetStatus();

	void GenerateToken();

	bool IsMatchCurChar(char c);
	bool IsMatchCurCharAndStepOnce(char c);

	char GetCurCharAndStepOnce();
	char GetCurChar();

	void AddToken(TokenType type);
	void AddToken(TokenType type, std::string_view literal);

	bool IsAtEnd();

	bool IsNumber(char c);
	bool IsLetter(char c);
	bool IsLetterOrNumber(char c);

	void Number();
	void Identifier();
	void String();

	uint32_t m_StartPos;
	uint32_t m_CurPos;
	uint32_t m_Line;
	uint32_t m_Column;
	std::string m_Source;
	std::vector<Token> m_Tokens;
	std::string m_FilePath;
};