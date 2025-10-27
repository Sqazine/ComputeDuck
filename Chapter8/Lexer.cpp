#include "Lexer.h"

const std::unordered_map<std::string, TokenType> keywords =
    {
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"nil", TokenType::NIL},
        {"and", TokenType::AND},
        {"or", TokenType::OR},
        {"not", TokenType::NOT},
        
        {"print", TokenType::PRINT},
        
};

Lexer::Lexer()
{
    ResetStatus();
}

Lexer::~Lexer()
{
}

const std::vector<Token> &Lexer::GenerateTokens(std::string_view src, std::string_view filePath)
{
    ResetStatus();
    m_Source = src;
    m_FilePath = filePath;
    while (!IsAtEnd())
    {
        m_StartPos = m_CurPos;
        GenerateToken();
    }
    m_Tokens.emplace_back(TokenType::END, "", m_Line, m_Column, m_FilePath);
    return m_Tokens;
}

void Lexer::GenerateToken()
{
    char c = GetCurCharAndStepOnce();

    switch (c)
    {
    case '(':
        AddToken(TokenType::LPAREN);
        break;
    case ')':
        AddToken(TokenType::RPAREN);
        break;
    case '[':
        AddToken(TokenType::LBRACKET);
        break;
    case ']':
        AddToken(TokenType::RBRACKET);
        break;
    // ++ 新增内容
    case '{':
        AddToken(TokenType::LBRACE);
        break;
    case '}':
        AddToken(TokenType::RBRACE);
        break;
    // -- 新增内容
    case ',':
        AddToken(TokenType::COMMA);
        break;
    case ';':
        AddToken(TokenType::SEMICOLON);
        break;
    case '\"':
        String();
        break;
    case ' ':
    case '\t':
    case '\r':
        break;
    case '\n':
        m_Line++;
        m_Column = 1;
        break;
    case '+':
        AddToken(TokenType::PLUS);
        break;
    case '-':
        AddToken(TokenType::MINUS);
        break;
    case '*':
        AddToken(TokenType::ASTERISK);
        break;
    case '/':
        AddToken(TokenType::SLASH);
        break;
    case '&':
        AddToken(TokenType::AMPERSAND);
        break;
    case '|':
        AddToken(TokenType::VBAR);
        break;
    case '~':
        AddToken(TokenType::TILDE);
        break;
    case '^':
        AddToken(TokenType::CARET);
        break;
    case '#':
    {
        while (!IsMatchCurChar('\n') && !IsAtEnd())
            GetCurCharAndStepOnce();
        break;
    }
    case '!':
        if (IsMatchCurCharAndStepOnce('='))
            AddToken(TokenType::BANG_EQUAL);
        else
            ASSERT("[line %u]: Unknown character '!'", m_Line);
        break;
    case '<':
        if (IsMatchCurCharAndStepOnce('='))
            AddToken(TokenType::LESS_EQUAL);
        else
            AddToken(TokenType::LESS);
        break;
    case '>':
        if (IsMatchCurCharAndStepOnce('='))
            AddToken(TokenType::GREATER_EQUAL);
        else
            AddToken(TokenType::GREATER);
        break;
    case '=':
        if (IsMatchCurCharAndStepOnce('='))
            AddToken(TokenType::EQUAL_EQUAL);
        else
            AddToken(TokenType::EQUAL);
        break;
    default:
        if (IsNumber(c))
            Number();
        else if (IsLetter(c))
            KeyWordOrIdentifier();
        else
            AddToken(TokenType::END);
        break;
    }
}

void Lexer::ResetStatus()
{
    m_StartPos = m_CurPos = 0;
    m_Line = 1;
    m_Column = 1;
    std::vector<Token>().swap(m_Tokens);
}

bool Lexer::IsMatchCurChar(char c)
{
    return GetCurChar() == c;
}
bool Lexer::IsMatchCurCharAndStepOnce(char c)
{
    bool result = GetCurChar() == c;
    if (result)
    {
        m_CurPos++;
        m_Column++;
    }
    return result;
}

char Lexer::GetCurCharAndStepOnce()
{
    if (!IsAtEnd())
    {
        m_Column++;
        return m_Source[m_CurPos++];
    }
    return '\0';
}

char Lexer::GetCurChar()
{
    if (!IsAtEnd())
        return m_Source[m_CurPos];
    return '\0';
}

void Lexer::AddToken(TokenType type)
{
    auto literal = m_Source.substr(m_StartPos, m_CurPos - m_StartPos);
    m_Tokens.emplace_back(type, literal, m_Line, m_Column, m_FilePath);
}
void Lexer::AddToken(TokenType type, std::string_view literal)
{
    m_Tokens.emplace_back(type, literal, m_Line, m_Column, m_FilePath);
}

bool Lexer::IsAtEnd()
{
    return m_CurPos >= m_Source.size();
}

bool Lexer::IsNumber(char c)
{
    return c >= '0' && c <= '9';
}
bool Lexer::IsLetter(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

bool Lexer::IsLetterOrNumber(char c)
{
    return IsLetter(c) || IsNumber(c);
}

void Lexer::Number()
{
    while (IsNumber(GetCurChar()))
        GetCurCharAndStepOnce();

    if (IsMatchCurCharAndStepOnce('.'))
    {
        if (IsNumber(GetCurChar()))
            while (IsNumber(GetCurChar()))
                GetCurCharAndStepOnce();
        else
            ASSERT("[line %u]:Number cannot end with '.'", m_Line);
    }

    AddToken(TokenType::NUMBER);
}

void Lexer::KeyWordOrIdentifier()
{
    while (IsLetterOrNumber(GetCurChar()))
        GetCurCharAndStepOnce();

    std::string literal = m_Source.substr(m_StartPos, m_CurPos - m_StartPos);

    bool isKeyWord = false;
    for (const auto &[key, value] : keywords)
    {
        if (key.compare(literal) == 0)
        {
            AddToken(value, literal);
            isKeyWord = true;
            break;
        }
    }

    if (!isKeyWord)
        AddToken(TokenType::IDENTIFIER, literal);
}

void Lexer::String()
{
    while (!IsMatchCurChar('\"') && !IsAtEnd())
    {
        if (IsMatchCurChar('\n'))
        {
            m_Line++;
            m_Column = 1;
        }
        GetCurCharAndStepOnce();
    }

    if (IsAtEnd())
        ASSERT("[line %u]:Uniterminated string.", m_Line);

    GetCurCharAndStepOnce(); // eat the second '\"'

    AddToken(TokenType::STRING, m_Source.substr(m_StartPos + 1, m_CurPos - m_StartPos - 2));
}