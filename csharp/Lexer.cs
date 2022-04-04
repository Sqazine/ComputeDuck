using System.Collections;
namespace ComputeDuck
{
    public class Lexer
    {
        public Lexer()
        {
            ResetStatus();
        }

        public List<Token> GenerateTokens(string src)
        {
            ResetStatus();
            m_Source = src;
            while (!IsAtEnd())
            {
                m_StartPos = m_CurPos;
                GenerateToken();
            }

            AddToken(TokenType.END, "EOF");
            return m_Tokens;
        }

        private void ResetStatus()
        {
            m_StartPos = m_CurPos = 0;
            m_Line = 1;
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
                case '#':
                    {
                        while (!IsMatchCurChar('\n') && !IsAtEnd())
                            GetCurCharAndStepOnce();
                        m_Line++;
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
                        AddToken(TokenType.UNKNOWN);
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
                m_CurPos++;
            return result;
        }

        private bool IsMatchNextChar(char c)
        {
            return GetNextChar() == c;
        }

        private bool IsMatchNextCharAndStepOnce(char c)
        {
            bool result = GetNextChar() == c;
            if (result)
                m_CurPos++;
            return result;
        }

        private char GetNextCharAndStepOnce()
        {
            if (m_CurPos + 1 < m_Source.Length)
                return m_Source[++m_CurPos];
            return '\0';
        }
        private char GetNextChar()
        {
            if (m_CurPos + 1 < m_Source.Length)
                return m_Source[m_CurPos + 1];
            return '\0';
        }
        private char GetCurCharAndStepOnce()
        {
            if (!IsAtEnd())
                return m_Source[m_CurPos++];
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
            m_Tokens.Add(new Token(type, literal, m_Line));
        }
        private void AddToken(TokenType type, string literal)
        {
            m_Tokens.Add(new Token(type, literal, m_Line));
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
                AddToken(TokenType.NUMBER);
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
        private string m_Source;
        private List<Token> m_Tokens;

        private static Dictionary<string, TokenType> keywords = new Dictionary<string, TokenType>
        {
            {"var", TokenType.VAR},
            {"if", TokenType.IF},
            {"else", TokenType.ELSE},
            {"true", TokenType.TRUE},
            {"false", TokenType.FALSE},
            {"nil", TokenType.NIL},
            {"while", TokenType.WHILE},
            {"fn", TokenType.FUNCTION},
            {"return", TokenType.RETURN},
            {"and", TokenType.AND},
            {"or", TokenType.OR},
            {"not", TokenType.NOT},
            {"struct", TokenType.STRUCT},
            {"ref", TokenType.REF},
            {"lambda",TokenType.LAMBDA},
        };
    }
}