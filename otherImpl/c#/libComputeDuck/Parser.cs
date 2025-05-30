using System;
using System.Collections.Generic;

namespace ComputeDuck
{
    public enum Precedence
    {
        LOWEST = 0, // ,
        ASSIGN,     // =
        OR,         // or
        AND,        // and
        BIT_OR,		// |
        BIT_XOR,	// ^
        BIT_AND,	// &
        EQUAL,      // == !=
        COMPARE,    // < <= > >=
        ADD_PLUS,   // + -
        MUL_DIV,    //  /
        UNARY,     // !
        CALL,      // [] () .
    };

    public delegate Expr UnaryFn();
    public delegate Expr BinaryFn(Expr unary);

    public class Parser
    {
        public Parser() { }

        public List<Stmt> Parse(List<Token> tokens)
        {
            m_CurPos = 0;
            m_Tokens = tokens;
            m_FunctionOrFunctionScopeDepth = 0;

            List<Stmt> stmts = new List<Stmt>();
            while (!IsMatchCurToken(TokenType.END))
                stmts.Add(ParseStmt());

            stmts = m_ConstantFolder.Fold(stmts);

            return stmts;
        }
        private static Stmt ParseStmt()
        {
            if (IsMatchCurToken(TokenType.RETURN))
                return ParseReturnStmt();
            else if (IsMatchCurToken(TokenType.IF))
                return ParseIfStmt();
            else if (IsMatchCurToken(TokenType.LBRACE))
                return ParseScopeStmt();
            else if (IsMatchCurToken(TokenType.WHILE))
                return ParseWhileStmt();
            else if (IsMatchCurToken(TokenType.STRUCT))
                return ParseStructStmt();
            else if(IsMatchCurToken(TokenType.DLLIMPORT))
                return ParseDllImportStmt();
            else
                return ParseExprStmt();
        }
        private static Stmt ParseExprStmt()
        {
            var exprStmt = new ExprStmt(ParseExpr());
            Consume(TokenType.SEMICOLON, "Expect ';' after expr stmt.");
            return exprStmt;
        }
        private static Stmt ParseReturnStmt()
        {
            if (m_FunctionOrFunctionScopeDepth == 0)
                Utils.Assert("Return statement only available in function");

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
        private static Expr ParseFunctionExpr()
        {
            m_FunctionOrFunctionScopeDepth++;

            Consume(TokenType.FUNCTION, "Expect 'function' keyword");

            var funcExpr = new FunctionExpr();

            Consume(TokenType.LPAREN, "Expect '(' after function name");

            if (!IsMatchCurToken(TokenType.RPAREN)) //has parameter
            {
                IdentifierExpr idenExpr = (IdentifierExpr)ParseIdentifierExpr();
                funcExpr.parameters.Add(idenExpr);
                while (IsMatchCurTokenAndStepOnce(TokenType.COMMA))
                {
                    idenExpr = (IdentifierExpr)ParseIdentifierExpr();
                    funcExpr.parameters.Add(idenExpr);
                }
            }
            Consume(TokenType.RPAREN, "Expect ')' after function expr's '('");

            funcExpr.body = (ScopeStmt)ParseScopeStmt();

            m_FunctionOrFunctionScopeDepth--;

            return funcExpr;
        }
        private static Stmt ParseStructStmt()
        {
            Consume(TokenType.STRUCT, "Expect 'struct' keyword");

            var structStmt = new StructStmt();

            structStmt.name = ParseIdentifierExpr().Stringify();

            Consume(TokenType.LBRACE, "Expect '{' after struct name");

            while (!IsMatchCurToken(TokenType.RBRACE))
            {
                var k = (IdentifierExpr)ParseIdentifierExpr();
                Expr v = new NilExpr();
                if (IsMatchCurToken(TokenType.COLON))
                {
                    Consume(TokenType.COLON, "Expect ':'");
                    v = ParseExpr();
                }
                IsMatchCurTokenAndStepOnce(TokenType.COMMA);
                structStmt.members.Add(new KeyValuePair<IdentifierExpr, Expr?>(k, v));
            }

            Consume(TokenType.RBRACE, "Expect '}' after struct's '{'");

            return structStmt;
        }

        private static Stmt ParseDllImportStmt()
        {
            Consume(TokenType.DLLIMPORT, "Expect 'dllimport' keyword");

            Consume(TokenType.LPAREN, "Expect '(' after 'dllimport' keyword");

            var path = Consume(TokenType.STRING, "Expect dll path.").literal;

            Consume(TokenType.RPAREN, "Expect ')' after dllimport stmt");
            Consume(TokenType.SEMICOLON, "Expect ';' after dllimport stmt ')'");

            if (!path.Contains(".")) //no file suffix
                path += ".dll";

            path = "library-" + path;

            return new DllImportStmt(path);
        }

        private static Expr ParseExpr(Precedence precedence = Precedence.LOWEST)
        {
            if (!m_UnaryFunctions.ContainsKey(GetCurToken().type))
            {
                Console.Write("no unary definition for:" + GetCurTokenAndStepOnce().literal);
                return new NilExpr();
            }
            var unaryFn = m_UnaryFunctions[GetCurToken().type];

            var leftExpr = unaryFn();

            while (!IsMatchCurToken(TokenType.SEMICOLON) && precedence < GetCurTokenPrecedence())
            {
                if (!m_BinaryFunctions.ContainsKey(GetCurToken().type))
                    return leftExpr;

                var binaryFn = m_BinaryFunctions[GetCurToken().type];

                leftExpr = binaryFn(leftExpr);
            }

            return leftExpr;
        }
        private static Expr ParseIdentifierExpr()
        {
            return new IdentifierExpr(Consume(TokenType.IDENTIFIER, "Unexpect identifier'" + GetCurToken().literal + "'.").literal);
        }
        private static Expr ParseNumExpr()
        {
            string numLiteral = Consume(TokenType.NUMBER, "Expect a number literal.").literal;
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
        private static Expr ParseUnaryExpr()
        {
            var unaryExpr = new UnaryExpr();
            unaryExpr.op = GetCurTokenAndStepOnce().literal;
            unaryExpr.right = ParseExpr(Precedence.UNARY);
            return unaryExpr;
        }
        private static Expr ParseRefExpr()
        {
            Consume(TokenType.REF, "Expect 'ref' keyword.");
            var refExpr = ParseExpr(Precedence.LOWEST);
            return new RefExpr(refExpr);
        }
        private static Expr ParseBinaryExpr(Expr unaryExpr)
        {
            var binaryExpr = new BinaryExpr();
            binaryExpr.left = unaryExpr;

            Precedence opPrece = GetCurTokenPrecedence();

            binaryExpr.op = GetCurTokenAndStepOnce().literal;
            binaryExpr.right = ParseExpr(opPrece);
            return binaryExpr;
        }
        private static Expr ParseIndexExpr(Expr unaryExpr)
        {
            Consume(TokenType.LBRACKET, "Expect '['.");
            var indexExpr = new IndexExpr();
            indexExpr.ds = unaryExpr;
            indexExpr.index = ParseExpr(Precedence.CALL);
            Consume(TokenType.RBRACKET, "Expect ']'.");
            return indexExpr;
        }
        private static Expr ParseFunctionCallExpr(Expr unaryExpr)
        {
            var funcCallExpr = new FunctionCallExpr();

            funcCallExpr.name = unaryExpr;
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
        private static Expr ParseStructCallExpr(Expr unaryExpr)
        {
            Consume(TokenType.DOT, "Expect '.'.");
            var structCallExpr = new StructCallExpr();
            structCallExpr.callee = unaryExpr;
            structCallExpr.callMember = ParseExpr(Precedence.CALL);
            return structCallExpr;
        }

        private static Expr ParseStructExpr()
        {
            List<KeyValuePair<IdentifierExpr, Expr?>> memPairs = new List<KeyValuePair<IdentifierExpr, Expr?>>();
            Consume(TokenType.LBRACE, "Expect '{'.");
            while (!IsMatchCurToken(TokenType.RBRACE))
            {
                var k = (IdentifierExpr)ParseIdentifierExpr();
                Expr v = new NilExpr();
                if (IsMatchCurToken(TokenType.COLON))
                {
                    Consume(TokenType.COLON, "Expect ':'");
                    v = ParseExpr();
                }
                IsMatchCurTokenAndStepOnce(TokenType.COMMA);
                memPairs.Add(new KeyValuePair<IdentifierExpr, Expr?>(k, v));
            }

            Consume(TokenType.RBRACE, "Expect '}'.");
            return new StructExpr(memPairs);
        }

        private static Token GetCurToken()
        {
            if (!IsAtEnd())
                return m_Tokens[m_CurPos];
            return m_Tokens[m_Tokens.Count - 1];
        }
        private static Token GetCurTokenAndStepOnce()
        {
            if (!IsAtEnd())
                return m_Tokens[m_CurPos++];
            return m_Tokens[m_Tokens.Count - 1];
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
            return m_Tokens[m_Tokens.Count - 1];
        }
        private static Token GetNextTokenAndStepOnce()
        {
            if (m_CurPos + 1 < m_Tokens.Count)
                return m_Tokens[++m_CurPos];
            return m_Tokens[m_Tokens.Count - 1];
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
            return new Token(TokenType.END, "", 0,0, "RootFile");
        }

        private static bool IsAtEnd()
        {
            return m_CurPos >= m_Tokens.Count;
        }

        private static int m_CurPos;

        private static List<Token> m_Tokens = new List<Token>();
        private static ConstantFolder m_ConstantFolder = new ConstantFolder();

        private static int m_FunctionOrFunctionScopeDepth = 0;

        private static Dictionary<TokenType, UnaryFn> m_UnaryFunctions = new Dictionary<TokenType, UnaryFn>()
        {
            {TokenType.IDENTIFIER, Parser.ParseIdentifierExpr},
            {TokenType.NUMBER, Parser.ParseNumExpr},
            {TokenType.STRING, Parser.ParseStrExpr},
            {TokenType.NIL, Parser.ParseNilExpr},
            {TokenType.TRUE, Parser.ParseTrueExpr},
            {TokenType.FALSE, Parser.ParseFalseExpr},
            {TokenType.MINUS, Parser.ParseUnaryExpr},
            {TokenType.NOT, Parser.ParseUnaryExpr},
            {TokenType.LPAREN, Parser.ParseGroupExpr},
            {TokenType.LBRACKET, Parser.ParseArrayExpr},
            {TokenType.REF, Parser.ParseRefExpr},
            {TokenType.FUNCTION,Parser.ParseFunctionExpr},
            {TokenType.LBRACE,Parser.ParseStructExpr},
            {TokenType.TILDE,Parser.ParseUnaryExpr},
        };
        private static Dictionary<TokenType, BinaryFn> m_BinaryFunctions = new Dictionary<TokenType, BinaryFn>()
        {
            {TokenType.EQUAL, Parser.ParseBinaryExpr},
            {TokenType.EQUAL_EQUAL, Parser.ParseBinaryExpr},
            {TokenType.BANG_EQUAL, Parser.ParseBinaryExpr},
            {TokenType.LESS, Parser.ParseBinaryExpr},
            {TokenType.LESS_EQUAL, Parser.ParseBinaryExpr},
            {TokenType.GREATER, Parser.ParseBinaryExpr},
            {TokenType.GREATER_EQUAL, Parser.ParseBinaryExpr},
            {TokenType.PLUS, Parser.ParseBinaryExpr},
            {TokenType.MINUS, Parser.ParseBinaryExpr},
            {TokenType.ASTERISK, Parser.ParseBinaryExpr},
            {TokenType.SLASH, Parser.ParseBinaryExpr},
            {TokenType.LPAREN, Parser.ParseFunctionCallExpr},
            {TokenType.LBRACKET, Parser.ParseIndexExpr},
            {TokenType.AND, Parser.ParseBinaryExpr},
            {TokenType.OR, Parser.ParseBinaryExpr},
            {TokenType.DOT, Parser.ParseStructCallExpr},
            {TokenType.AMPERSAND,Parser.ParseBinaryExpr},
            {TokenType.VBAR,Parser.ParseBinaryExpr},
            {TokenType.CARET,Parser.ParseBinaryExpr},
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
            {TokenType.LBRACKET, Precedence.CALL},
            {TokenType.LPAREN, Precedence.CALL},
            {TokenType.AND, Precedence.AND},
            {TokenType.OR, Precedence.OR},
            {TokenType.DOT, Precedence.CALL},
            {TokenType.AMPERSAND,Precedence.BIT_AND},
            {TokenType.VBAR,Precedence.BIT_OR},
            {TokenType.CARET,Precedence.BIT_XOR},
        };
    }
}