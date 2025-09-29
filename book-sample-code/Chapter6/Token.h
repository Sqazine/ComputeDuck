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
    SEMICOLON,     // ;
    LPAREN,        // (
    RPAREN,        // )
    LBRACKET,      // [
    RBRACKET,      // ]
    PLUS,          // +
    MINUS,         // -
    ASTERISK,      // *
    SLASH,         // /
    LESS,          // <
    GREATER,       // >
    VBAR,          // |
    CARET,         // ^
    AMPERSAND,     // &
    TILDE,         // ~
    EQUAL,         // =
    EQUAL_EQUAL,   // ==
    LESS_EQUAL,    // <=
    GREATER_EQUAL, // >=
    BANG_EQUAL,    // !=
    TRUE,          // true
    FALSE,         // false
    NIL,           // nil
    AND,           // and
    OR,            // or
    NOT,           // not
    END
};

struct Token
{
    Token(TokenType type, std::string_view literal, uint32_t line, uint32_t column, std::string_view filePath) : type(type), literal(literal), line(line), column(column), filePath(filePath) {}

    TokenType type;
    std::string filePath; // 当前Token所在的文件路径
    std::string literal;  // 当前Token字面量
    uint32_t line;        // 当前Token在文件中所在的行
    uint32_t column;      // 当前Token在文件中所在的列
};

inline std::ostream &operator<<(std::ostream &stream, const Token &token)
{
    return stream << token.filePath << ":'" << token.literal << "'," << token.line << "," << token.column;
}