#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <ostream>

enum class TokenType
{
    NUMBER = 0,
    SEMICOLON,	   // ;
    LPAREN,		   // (
    RPAREN,		   // )
    PLUS,		   // +
    MINUS,		   // -
    ASTERISK,	   // *
    SLASH,		   // /
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
    TRUE,		   // true
    FALSE,		   // false
    NIL,		   // nil
    AND,		   // and
    OR,			   // or
    NOT,		   // not
    END
};

struct Token
{
    Token(TokenType type, std::string_view literal, uint32_t line, uint32_t column, std::string_view filePath) : type(type), literal(literal), line(line), column(column), filePath(filePath) {}

    std::string filePath;
    TokenType type;
    std::string literal;
    uint32_t line;
    uint32_t column;
};

inline std::ostream &operator<<(std::ostream &stream, const Token &token)
{
    return stream << token.filePath << ":'" << token.literal << "'," << token.line << "," << token.column;
}