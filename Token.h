#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>

enum class TokenType
{
	NUMBER = 0,
	STRING,
	IDENTIFIER,
	COMMA,		   // ,
	DOT,		   // .
	COLON,		   // :
	SEMICOLON,	   // ;
	LBRACKET,	   // [
	RBRACKET,	   // ]
	LBRACE,		   // {
	RBRACE,		   // }
	LPAREN,		   // (
	RPAREN,		   // )
	PLUS,		   // +
	MINUS,		   // -
	ASTERISK,	   // *
	SLASH,		   // /
	EQUAL,		   // =
	LESS,		   // <
	GREATER,	   // >
	VBAR,		   // |
	CARET,		   // ^
	AMPERSAND,	   // &
	TILDE,		   // ~
	EQUAL_EQUAL,   // ==
	LESS_EQUAL,	   // <=
	GREATER_EQUAL, // >=
	BANG_EQUAL,	   // !=
	IF,			   // if
	ELSE,		   // else
	TRUE,		   // true
	FALSE,		   // false
	NIL,		   // nil
	WHILE,		   // while
	FUNCTION,	   // function
	RETURN,		   // return
	AND,		   // and
	OR,			   // or
	NOT,		   // not
	STRUCT,		   // struct
	REF,		   // ref
	DLLIMPORT,	   // dllimport
	IMPORT,		   // import
	END
};

struct Token
{
	Token(TokenType type, std::string_view literal, uint64_t line, std::string_view filePath) : type(type), literal(literal), line(line), filePath(filePath) {}

	std::string filePath;
	TokenType type;
	std::string literal;
	uint64_t line;
};

inline std::ostream &operator<<(std::ostream &stream, const Token &token)
{
	return stream << token.filePath << ":'" << token.literal << "'," << token.line;
}