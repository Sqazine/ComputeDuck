using System.Collections.Generic;

namespace ComputeDuck
{
    public class Lexer
    {
        public Lexer()
        {
            ResetStatus();
        }

        public List<Token> GenerateTokens(string src, string filePath = "RootFile")
        {
            ResetStatus();
            m_Source = src;
            m_FilePath = filePath;
            while (!IsAtEnd())
            {
                m_StartPos = m_CurPos;
                GenerateToken();
            }

            return m_Tokens;
        }

        private void ResetStatus()
        {
            m_StartPos = m_CurPos = 0;
            m_Line = 1;
            m_Column = 1;
            m_Tokens = new List<Token>();
        }


        private void GenerateToken()
        {
            char c = GetCurCharAndStepOnce();
            switch (c)
            {
                case '(':
                    AddToken(TokenType.LPAREN);
                    break;
                case ')':
                    AddToken(TokenType.RPAREN);
                    break;
                case '[':
                    AddToken(TokenType.LBRACKET);
                    break;
                case ']':
                    AddToken(TokenType.RBRACKET);
                    break;
                case '{':
                    AddToken(TokenType.LBRACE);
                    break;
                case '}':
                    AddToken(TokenType.RBRACE);
                    break;
                case ',':
                    AddToken(TokenType.COMMA);
                    break;
                case '.':
                    AddToken(TokenType.DOT);
                    break;
                case ':':
                    AddToken(TokenType.COLON);
                    break;
                case ';':
                    AddToken(TokenType.SEMICOLON);
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
                    AddToken(TokenType.PLUS);
                    break;
                case '-':
                    AddToken(TokenType.MINUS);
                    break;
                case '*':
                    AddToken(TokenType.ASTERISK);
                    break;
                case '/':
                    AddToken(TokenType.SLASH);
                    break;
                case '&':
                    AddToken(TokenType.AMPERSAND);
                    break;
                case '|':
                    AddToken(TokenType.VBAR);
                    break;
                case '~':
                    AddToken(TokenType.TILDE);
                    break;
                case '^':
                    AddToken(TokenType.CARET);
                    break;
                case '#':
                    {
                        while (!IsMatchCurChar('\n') && !IsAtEnd())
                            GetCurCharAndStepOnce();
                        break;
                    }
                case '!':
                    if (IsMatchCurCharAndStepOnce('='))
                        AddToken(TokenType.BANG_EQUAL);
                    break;
                case '<':
                    if (IsMatchCurCharAndStepOnce('='))
                        AddToken(TokenType.LESS_EQUAL);
                    else
                        AddToken(TokenType.LESS);
                    break;
                case '>':
                    if (IsMatchCurCharAndStepOnce('='))
                        AddToken(TokenType.GREATER_EQUAL);
                    else
                        AddToken(TokenType.GREATER);
                    break;
                case '=':
                    if (IsMatchCurCharAndStepOnce('='))
                        AddToken(TokenType.EQUAL_EQUAL);
                    else
                        AddToken(TokenType.EQUAL);
                    break;
                default:
                    if (IsNumber(c))
                        Number();
                    else if (IsLetter(c))
                        Identifier();
                    else
                        AddToken(TokenType.END);
                    break;
            }
        }

        private bool IsMatchCurChar(char c)
        {
            return GetCurChar() == c;
        }

        private bool IsMatchCurCharAndStepOnce(char c)
        {
            bool result = GetCurChar() == c;
            if (result)
            {
                m_CurPos++;
                m_Column++;
            }
            return result;
        }

        private char GetCurCharAndStepOnce()
        {
            if (!IsAtEnd())
            {
                m_Column++;
                return m_Source[m_CurPos++];
            }
            return '\0';
        }
        private char GetCurChar()
        {
            if (!IsAtEnd())
                return m_Source[m_CurPos];
            return '\0';
        }

        private void AddToken(TokenType type)
        {
            var literal = m_Source.Substring(m_StartPos, m_CurPos - m_StartPos);
            m_Tokens.Add(new Token(type, literal, m_Line,m_Column, m_FilePath));
        }
        private void AddToken(TokenType type, string literal)
        {
            m_Tokens.Add(new Token(type, literal, m_Line, m_Column, m_FilePath));
        }

        private bool IsAtEnd()
        {
            return m_CurPos >= m_Source.Length;
        }

        bool IsNumber(char c)
        {
            return c >= '0' && c <= '9';
        }
        bool IsLetter(char c)
        {
            return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
        }
        bool IsLetterOrNumber(char c)
        {
            return IsLetter(c) || IsNumber(c);
        }

        void Number()
        {
            while (IsNumber(GetCurChar()))
                GetCurCharAndStepOnce();

            if (IsMatchCurCharAndStepOnce('.'))
            {
                if (IsNumber(GetCurChar()))
                    while (IsNumber(GetCurChar()))
                        GetCurCharAndStepOnce();
                else
                    Utils.Assert("[line " + m_Line.ToString() + "]:Number cannot end with '.'");
            }
            AddToken(TokenType.NUMBER);
        }
        void Identifier()
        {
            while (IsLetterOrNumber(GetCurChar()))
                GetCurCharAndStepOnce();

            string literal = m_Source.Substring(m_StartPos, m_CurPos - m_StartPos);

            bool isKeyWord = false;
            foreach (var entry in keywords)
                if (entry.Key == literal)
                {
                    AddToken(entry.Value, literal);
                    isKeyWord = true;
                    break;
                }

            if (!isKeyWord)
                AddToken(TokenType.IDENTIFIER, literal);
        }
        void String()
        {
            while (!IsMatchCurChar('\"') && !IsAtEnd())
            {
                if (IsMatchCurChar('\n'))
                    m_Line++;
                GetCurCharAndStepOnce();
            }

            if (IsAtEnd())
                Utils.Assert("[line " + m_Line + "]:Uniterminated string.");

            GetCurCharAndStepOnce(); //eat the second '\"'

            AddToken(TokenType.STRING, m_Source.Substring(m_StartPos + 1, m_CurPos - m_StartPos - 2));
        }

        private int m_StartPos;
        private int m_CurPos;
        private int m_Line;
        private int m_Column;
        private string m_Source;
        private List<Token> m_Tokens;

        private string m_FilePath;

        private static Dictionary<string, TokenType> keywords = new Dictionary<string, TokenType>
        {
            {"if", TokenType.IF},
            {"else", TokenType.ELSE},
            {"true", TokenType.TRUE},
            {"false", TokenType.FALSE},
            {"nil", TokenType.NIL},
            {"while", TokenType.WHILE},
            {"function", TokenType.FUNCTION},
            {"return", TokenType.RETURN},
            {"and", TokenType.AND},
            {"or", TokenType.OR},
            {"not", TokenType.NOT},
            {"struct", TokenType.STRUCT},
            {"ref", TokenType.REF},
            {"dllimport",TokenType.DLLIMPORT},
            {"import",TokenType.IMPORT},
        };
    }
}