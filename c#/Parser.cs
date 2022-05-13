namespace ComputeDuck
{
    public enum Precedence
    {
        LOWEST = 0, // ,
        ASSIGN,     // =
        OR,         // or
        AND,        // and
        EQUAL,      // == !=
        COMPARE,    // < <= > >=
        ADD_PLUS,   // + -
        MUL_DIV,    //  /
        PREFIX,     // !
        INFIX,      // [] () .
    };

    public delegate Expr PrefixFn();
    public delegate Expr InfixFn(Expr prefix);

    public class Parser
    {
        public Parser() { }

        public List<Stmt> Parse(List<Token> tokens)
        {
            m_CurPos = 0;
            m_Tokens = tokens;

            List<Stmt> stmts = new List<Stmt>();
            while (!IsMatchCurToken(TokenType.END))
                stmts.Add(ParseStmt());
            return stmts;
        }
        private static Stmt ParseStmt()
        {
            if (IsMatchCurToken(TokenType.VAR))
                return ParseVarStmt();
            else if (IsMatchCurToken(TokenType.RETURN))
                return ParseReturnStmt();
            else if (IsMatchCurToken(TokenType.IF))
                return ParseIfStmt();
            else if (IsMatchCurToken(TokenType.LBRACE))
                return ParseScopeStmt();
            else if (IsMatchCurToken(TokenType.WHILE))
                return ParseWhileStmt();
            else if (IsMatchCurToken(TokenType.FUNCTION))
                return ParseFunctionStmt();
            else if (IsMatchCurToken(TokenType.STRUCT))
                return ParseStructStmt();
            else
                return ParseExprStmt();
        }
        private static Stmt ParseExprStmt()
        {
            var exprStmt = new ExprStmt(ParseExpr());
            Consume(TokenType.SEMICOLON, "Expect ';' after expr stmt.");
            return exprStmt;
        }
        private static Stmt ParseVarStmt()
        {
            Consume(TokenType.VAR, "Expect 'var' key word");

            var name = (IdentifierExpr)ParseIdentifierExpr();
            Expr value = new NilExpr();
            if (IsMatchCurTokenAndStepOnce(TokenType.EQUAL))
                value = ParseExpr();

            Consume(TokenType.SEMICOLON, "Expect ';' after var stmt.");

            return new VarStmt(name, value);
        }
        private static Stmt ParseReturnStmt()
        {
            Consume(TokenType.RETURN, "Expect 'return' key word.");

            var returnStmt = new ReturnStmt();

            if (!IsMatchCurToken(TokenType.SEMICOLON))
                returnStmt.expr = ParseExpr();

            Consume(TokenType.SEMICOLON, "Expect ';' after return stmt");
            return returnStmt;
        }
        private static Stmt ParseIfStmt()
        {
            Consume(TokenType.IF, "Expect 'if' key word.");
            Consume(TokenType.LPAREN, "Expect '(' after 'if'.");

            var ifStmt = new IfStmt();

            ifStmt.condition = ParseExpr();

            Consume(TokenType.RPAREN, "Expect ')' after if condition");

            ifStmt.thenBranch = ParseStmt();

            if (IsMatchCurTokenAndStepOnce(TokenType.ELSE))
                ifStmt.elseBranch = ParseStmt();

            return ifStmt;
        }
        private static Stmt ParseScopeStmt()
        {
            Consume(TokenType.LBRACE, "Expect '{'.");
            var scopeStmt = new ScopeStmt();
            while (!IsMatchCurToken(TokenType.RBRACE))
                scopeStmt.stmts.Add(ParseStmt());
            Consume(TokenType.RBRACE, "Expect '}'.");
            return scopeStmt;
        }
        private static Stmt ParseWhileStmt()
        {
            Consume(TokenType.WHILE, "Expect 'while' keyword.");
            Consume(TokenType.LPAREN, "Expect '(' after 'while'.");

            var whileStmt = new WhileStmt();

            whileStmt.condition = ParseExpr();

            Consume(TokenType.RPAREN, "Expect ')' after while stmt's condition.");

            whileStmt.body = ParseStmt();

            return whileStmt;
        }
        private static Stmt ParseFunctionStmt()
        {
            Consume(TokenType.FUNCTION, "Expect 'fn' keyword");

            var funcStmt = new FunctionStmt();

            funcStmt.name = ParseIdentifierExpr().Stringify();

            Consume(TokenType.LPAREN, "Expect '(' after function name");

            if (!IsMatchCurToken(TokenType.RPAREN)) //has parameter
            {
                IdentifierExpr idenExpr = (IdentifierExpr)ParseIdentifierExpr();
                funcStmt.parameters.Add(idenExpr);
                while (IsMatchCurTokenAndStepOnce(TokenType.COMMA))
                {
                    idenExpr = (IdentifierExpr)ParseIdentifierExpr();
                    funcStmt.parameters.Add(idenExpr);
                }
            }
            Consume(TokenType.RPAREN, "Expect ')' after function expr's '('");

            funcStmt.body = (ScopeStmt)ParseScopeStmt();

            return funcStmt;
        }
        private static Stmt ParseStructStmt()
        {
            Consume(TokenType.STRUCT, "Expect 'struct' keyword");

            var structStmt = new StructStmt();

            structStmt.name = ParseIdentifierExpr().Stringify();

            Consume(TokenType.LBRACE, "Expect '{' after struct name");

            while (!IsMatchCurToken(TokenType.RBRACE))
                structStmt.members.Add((VarStmt)ParseVarStmt());

            Consume(TokenType.RBRACE, "Expect '}' after struct's '{'");

            return structStmt;
        }

        private static Expr ParseExpr(Precedence precedence = Precedence.LOWEST)
        {
            if (!m_PrefixFunctions.ContainsKey(GetCurToken().type))
            {
                Console.Write("no prefix definition for:" + GetCurTokenAndStepOnce().literal);
                return new NilExpr();
            }
            var prefixFn = m_PrefixFunctions[GetCurToken().type];

            var leftExpr = prefixFn();

            while (!IsMatchCurToken(TokenType.SEMICOLON) && precedence < GetCurTokenPrecedence())
            {
                if (!m_InfixFunctions.ContainsKey(GetCurToken().type))
                    return leftExpr;

                var infixFn = m_InfixFunctions[GetCurToken().type];

                leftExpr = infixFn(leftExpr);
            }

            return leftExpr;
        }
        private static Expr ParseIdentifierExpr()
        {
            return new IdentifierExpr(Consume(TokenType.IDENTIFIER, "Unexpect identifier'" + GetCurToken().literal + "'.").literal);
        }
        private static Expr ParseNumExpr()
        {
            string numLiteral = Consume(TokenType.NUMBER, "Expexct a number literal.").literal;
            return new NumExpr(Double.Parse(numLiteral));
        }
        private static Expr ParseStrExpr()
        {
            return new StrExpr(Consume(TokenType.STRING, "Expect a string literal.").literal);
        }
        private static Expr ParseNilExpr()
        {
            Consume(TokenType.NIL, "Expect 'nil' keyword");
            return new NilExpr();
        }
        private static Expr ParseTrueExpr()
        {
            Consume(TokenType.TRUE, "Expect 'true' keyword");
            return new BoolExpr(true);
        }
        private static Expr ParseFalseExpr()
        {
            Consume(TokenType.FALSE, "Expect 'false' keyword");
            return new BoolExpr(false);
        }
        private static Expr ParseGroupExpr()
        {
            Consume(TokenType.LPAREN, "Expect '('.");
            var groupExpr = new GroupExpr(ParseExpr());
            Consume(TokenType.RPAREN, "Expect ')'.");
            return groupExpr;
        }
        private static Expr ParseArrayExpr()
        {
            Consume(TokenType.LBRACKET, "Expect '['.");

            var arrayExpr = new ArrayExpr();
            if (!IsMatchCurToken(TokenType.RBRACKET))
            {
                //first element
                arrayExpr.elements.Add(ParseExpr());
                while (IsMatchCurTokenAndStepOnce(TokenType.COMMA))
                    arrayExpr.elements.Add(ParseExpr());
            }

            Consume(TokenType.RBRACKET, "Expect ']'.");

            return arrayExpr;
        }
        private static Expr ParsePrefixExpr()
        {
            var prefixExpr = new PrefixExpr();
            prefixExpr.op = GetCurTokenAndStepOnce().literal;
            prefixExpr.right = ParseExpr(Precedence.PREFIX);
            return prefixExpr;
        }
        private static Expr ParseRefExpr()
        {
            Consume(TokenType.REF, "Expect 'ref' keyword.");

            var refExpr = ParseExpr(Precedence.LOWEST);

            if (refExpr.Type() != AstType.IDENTIFIER)
                Utils.Assert("Invalid reference type, only variable can be referenced.");

            return new RefExpr((IdentifierExpr)refExpr);
        }
        private static Expr ParseLambdaExpr()
        {
            Consume(TokenType.LAMBDA, "Expect 'lambda' keyword");

            var lambdaExpr = new LambdaExpr();

            Consume(TokenType.LPAREN, "Expect '(' after keyword 'lambda'");

            if (!IsMatchCurToken(TokenType.RPAREN)) //has parameter
            {
                IdentifierExpr idenExpr = (IdentifierExpr)ParseIdentifierExpr();
                lambdaExpr.parameters.Add(idenExpr);
                while (IsMatchCurTokenAndStepOnce(TokenType.COMMA))
                {
                    idenExpr = (IdentifierExpr)ParseIdentifierExpr();
                    lambdaExpr.parameters.Add(idenExpr);
                }
            }
            Consume(TokenType.RPAREN, "Expect ')' after function expr's '('");

            lambdaExpr.body = (ScopeStmt)ParseScopeStmt();

            return lambdaExpr;
        }
        private static Expr ParseInfixExpr(Expr prefixExpr)
        {
            var infixExpr = new InfixExpr();
            infixExpr.left = prefixExpr;

            Precedence opPrece = GetCurTokenPrecedence();

            infixExpr.op = GetCurTokenAndStepOnce().literal;
            infixExpr.right = ParseExpr(opPrece);
            return infixExpr;
        }
        private static Expr ParseIndexExpr(Expr prefixExpr)
        {
            Consume(TokenType.LBRACKET, "Expect '['.");
            var indexExpr = new IndexExpr();
            indexExpr.ds = prefixExpr;
            indexExpr.index = ParseExpr(Precedence.INFIX);
            Consume(TokenType.RBRACKET, "Expect ']'.");
            return indexExpr;
        }
        private static Expr ParseFunctionCallExpr(Expr prefixExpr)
        {
            var funcCallExpr = new FunctionCallExpr();

            funcCallExpr.name = prefixExpr;
            Consume(TokenType.LPAREN, "Expect '('.");
            if (!IsMatchCurToken(TokenType.RPAREN)) //has arguments
            {
                funcCallExpr.arguments.Add(ParseExpr());
                while (IsMatchCurTokenAndStepOnce(TokenType.COMMA))
                    funcCallExpr.arguments.Add(ParseExpr());
            }
            Consume(TokenType.RPAREN, "Expect ')'.");

            return funcCallExpr;
        }
        private static Expr ParseStructCallExpr(Expr prefixExpr)
        {
            Consume(TokenType.DOT, "Expect '.'.");
            var structCallExpr = new StructCallExpr();
            structCallExpr.callee = prefixExpr;
            structCallExpr.callMember = ParseExpr(Precedence.INFIX);
            return structCallExpr;
        }

        private static Token GetCurToken()
        {
            if (!IsAtEnd())
                return m_Tokens[m_CurPos];
            return m_Tokens.Last();
        }
        private static Token GetCurTokenAndStepOnce()
        {
            if (!IsAtEnd())
                return m_Tokens[m_CurPos++];
            return m_Tokens.Last();
        }
        private static Precedence GetCurTokenPrecedence()
        {
            if (m_Precedence.ContainsKey(GetCurToken().type))
                return m_Precedence[GetCurToken().type];
            return Precedence.LOWEST;
        }

        private static Token GetNextToken()
        {
            if (m_CurPos + 1 < m_Tokens.Count)
                return m_Tokens[m_CurPos + 1];
            return m_Tokens.Last();
        }
        private static Token GetNextTokenAndStepOnce()
        {
            if (m_CurPos + 1 < m_Tokens.Count)
                return m_Tokens[++m_CurPos];
            return m_Tokens.Last();
        }
        private static Precedence GetNextTokenPrecedence()
        {
            if (m_Precedence.ContainsKey(GetNextToken().type))
                return m_Precedence[GetNextToken().type];
            return Precedence.LOWEST;
        }

        private static bool IsMatchCurToken(TokenType type)
        {
            return GetCurToken().type == type;
        }
        private static bool IsMatchCurTokenAndStepOnce(TokenType type)
        {
            if (IsMatchCurToken(type))
            {
                m_CurPos++;
                return true;
            }
            return false;
        }
        private static bool IsMatchNextToken(TokenType type)
        {
            return GetNextToken().type == type;
        }
        private static bool IsMatchNextTokenAndStepOnce(TokenType type)
        {
            if (IsMatchNextToken(type))
            {
                m_CurPos++;
                return true;
            }
            return false;
        }

        private static Token Consume(TokenType type, string errMsg)
        {
            if (IsMatchCurToken(type))
                return GetCurTokenAndStepOnce();
            Utils.Assert("[line " + (GetCurToken().line).ToString() + "]:" + errMsg);
            //avoid warning
            return new Token(TokenType.END, "", 0);
        }

        private static bool IsAtEnd()
        {
            return m_CurPos >= m_Tokens.Count;
        }

        private static int m_CurPos;

        private static List<Token> m_Tokens=new List<Token>();

        private static Dictionary<TokenType, PrefixFn> m_PrefixFunctions = new Dictionary<TokenType, PrefixFn>()
        {
            {TokenType.IDENTIFIER, Parser.ParseIdentifierExpr},
            {TokenType.NUMBER, Parser.ParseNumExpr},
            {TokenType.STRING, Parser.ParseStrExpr},
            {TokenType.NIL, Parser.ParseNilExpr},
            {TokenType.TRUE, Parser.ParseTrueExpr},
            {TokenType.FALSE, Parser.ParseFalseExpr},
            {TokenType.MINUS, Parser.ParsePrefixExpr},
            {TokenType.NOT, Parser.ParsePrefixExpr},
            {TokenType.LPAREN, Parser.ParseGroupExpr},
            {TokenType.LBRACKET, Parser.ParseArrayExpr},
            {TokenType.REF, Parser.ParseRefExpr},
            {TokenType.LAMBDA,Parser.ParseLambdaExpr},
        };
        private static Dictionary<TokenType, InfixFn> m_InfixFunctions = new Dictionary<TokenType, InfixFn>()
        {
            {TokenType.EQUAL, Parser.ParseInfixExpr},
            {TokenType.EQUAL_EQUAL, Parser.ParseInfixExpr},
            {TokenType.BANG_EQUAL, Parser.ParseInfixExpr},
            {TokenType.LESS, Parser.ParseInfixExpr},
            {TokenType.LESS_EQUAL, Parser.ParseInfixExpr},
            {TokenType.GREATER, Parser.ParseInfixExpr},
            {TokenType.GREATER_EQUAL, Parser.ParseInfixExpr},
            {TokenType.PLUS, Parser.ParseInfixExpr},
            {TokenType.MINUS, Parser.ParseInfixExpr},
            {TokenType.ASTERISK, Parser.ParseInfixExpr},
            {TokenType.SLASH, Parser.ParseInfixExpr},
            {TokenType.LPAREN, Parser.ParseFunctionCallExpr},
            {TokenType.LBRACKET, Parser.ParseIndexExpr},
            {TokenType.AND, Parser.ParseInfixExpr},
            {TokenType.OR, Parser.ParseInfixExpr},
            {TokenType.DOT, Parser.ParseStructCallExpr},
        };
        private static Dictionary<TokenType, Precedence> m_Precedence = new Dictionary<TokenType, Precedence>()
        {
            {TokenType.EQUAL, Precedence.ASSIGN},
            {TokenType.EQUAL_EQUAL, Precedence.EQUAL},
            {TokenType.BANG_EQUAL, Precedence.EQUAL},
            {TokenType.LESS, Precedence.COMPARE},
            {TokenType.LESS_EQUAL, Precedence.COMPARE},
            {TokenType.GREATER, Precedence.COMPARE},
            {TokenType.GREATER_EQUAL, Precedence.COMPARE},
            {TokenType.PLUS, Precedence.ADD_PLUS},
            {TokenType.MINUS, Precedence.ADD_PLUS},
            {TokenType.ASTERISK, Precedence.MUL_DIV},
            {TokenType.SLASH, Precedence.MUL_DIV},
            {TokenType.LBRACKET, Precedence.INFIX},
            {TokenType.LPAREN, Precedence.INFIX},
            {TokenType.AND, Precedence.AND},
            {TokenType.OR, Precedence.OR},
            {TokenType.DOT, Precedence.INFIX}
        };
    }
}