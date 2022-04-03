namespace ComputeDuck
{
    public enum TokenType
    {
        NUMBER = 0,
        STRING,
        IDENTIFIER,
        COMMA,         // ,
        DOT,           // .
        COLON,         // :
        SEMICOLON,     // ;
        LBRACKET,      // [
        RBRACKET,      // ]
        LBRACE,        // {
        RBRACE,        // }
        LPAREN,        // (
        RPAREN,        // )
        PLUS,          // +
        MINUS,         // -
        ASTERISK,      // *
        SLASH,         // /
        EQUAL,         // =
        LESS,          // <
        GREATER,       // >
        EQUAL_EQUAL,   // ==
        LESS_EQUAL,    // <=
        GREATER_EQUAL, // >=
        BANG_EQUAL,    // !=
        VAR,           // var
        IF,            // if
        ELSE,          // else
        TRUE,          // true
        FALSE,         // false
        NIL,           // nil
        WHILE,         // while
        FUNCTION,      // function
        RETURN,        // return
        AND,           // and
        OR,            // or
        NOT,           // not
        STRUCT,        // struct
        REF,           // ref
        LAMBDA,        // lambda

        UNKNOWN,
        END
    }

  public  class Token
    {
        public Token(TokenType type, string literal, int line)
        {
            this.literal = literal;
            this.type = type;
            this.line = line;
        }

        public string Stringify()
        {
            return this.literal + "," + this.line.ToString();
        }

        public TokenType type;
        public string literal;
        int line;
    }
}