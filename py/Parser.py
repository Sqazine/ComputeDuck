from ast import Lambda
from enum import IntEnum
from typing import Any

from Ast import Stmt
from Ast import Expr

from Token import Token, TokenType
from Utils import Assert
from Ast import AstType, ArrayExpr, BoolExpr, ExprStmt, FunctionCallExpr, FunctionStmt, GroupExpr, IdentifierExpr, IfStmt, IndexExpr, InfixExpr, NilExpr, NumExpr, PrefixExpr, ReturnStmt, ScopeStmt, StrExpr, StructCallExpr, StructStmt, VarStmt, WhileStmt, RefExpr,LambdaExpr,AnonyStructExpr
from SemanticAnalyzer import SemanticAnalyzer


class Precedence(IntEnum):
    LOWEST = 0,  # ,
    ASSIGN = 1,		# =
    OR = 2,			# or
    AND = 3,		# and
    EQUAL = 4,		# == !=
    COMPARE = 5,  # < <= > >=
    ADD_PLUS = 6,  # + -
    MUL_DIV = 7,  # * /
    PREFIX = 8,		# !
    INFIX = 9,		# [] () .


class Parser:
    __curPos: int = 0
    __tokens: list[Token] = []
    __prefixFunctions: dict[TokenType, Any] = {}
    __infixFunctions: dict[TokenType, Any] = {}
    __precedence: dict[TokenType, Any] = {}
    __isInFunctionOrLambdascope=False
    __semanticAnalyzer:SemanticAnalyzer

    def __init__(self) -> None:
        self.__curPos: int = 0
        self.__tokens: list[Token] = []
        self.__prefixFunctions: dict[TokenType, Any] = {}
        self.__infixFunctions: dict[TokenType, Any] = {}
        self.__precedence: dict[TokenType, Any] = {}
        self.__semanticAnalyzer=SemanticAnalyzer()

        self.__isInFunctionOrLambdascope=False
        
        self.__prefixFunctions = {
            TokenType.IDENTIFIER: self.ParseIdentifierExpr,
            TokenType.NUMBER: self.ParseNumExpr,
            TokenType.STRING: self.ParseStrExpr,
            TokenType.NIL: self.ParseNilExpr,
            TokenType.TRUE: self.ParseTrueExpr,
            TokenType.FALSE: self.ParseFalseExpr,
            TokenType.MINUS: self.ParsePrefixExpr,
            TokenType.NOT: self.ParsePrefixExpr,
            TokenType.LPAREN: self.ParseGroupExpr,
            TokenType.LBRACKET: self.ParseArrayExpr,
            TokenType.REF:self.ParseRefExpr,
            TokenType.LAMBDA:self.ParseLambdaExpr,
            TokenType.LBRACE:self.ParseAnonyStructExpr,
        }

        self.__infixFunctions = {
            TokenType.EQUAL: self.ParseInfixExpr,
            TokenType.EQUAL_EQUAL: self.ParseInfixExpr,
            TokenType.BANG_EQUAL: self.ParseInfixExpr,
            TokenType.LESS: self.ParseInfixExpr,
            TokenType.LESS_EQUAL: self.ParseInfixExpr,
            TokenType.GREATER: self.ParseInfixExpr,
            TokenType.GREATER_EQUAL: self.ParseInfixExpr,
            TokenType.PLUS: self.ParseInfixExpr,
            TokenType.MINUS: self.ParseInfixExpr,
            TokenType.ASTERISK: self.ParseInfixExpr,
            TokenType.SLASH: self.ParseInfixExpr,
            TokenType.LPAREN: self.ParseFunctionCallExpr,
            TokenType.LBRACKET: self.ParseIndexExpr,
            TokenType.AND: self.ParseInfixExpr,
            TokenType.OR: self.ParseInfixExpr,
            TokenType.DOT: self.ParseStructCallExpr,
        }

        self.__precedence = {
            TokenType.EQUAL: Precedence.ASSIGN,
          		TokenType.EQUAL_EQUAL: Precedence.EQUAL,
          		TokenType.BANG_EQUAL: Precedence.EQUAL,
          		TokenType.LESS: Precedence.COMPARE,
          		TokenType.LESS_EQUAL: Precedence.COMPARE,
          		TokenType.GREATER: Precedence.COMPARE,
          		TokenType.GREATER_EQUAL: Precedence.COMPARE,
          		TokenType.PLUS: Precedence.ADD_PLUS,
          		TokenType.MINUS: Precedence.ADD_PLUS,
          		TokenType.ASTERISK: Precedence.MUL_DIV,
          		TokenType.SLASH: Precedence.MUL_DIV,
          		TokenType.LBRACKET: Precedence.INFIX,
          		TokenType.LPAREN: Precedence.INFIX,
          		TokenType.AND: Precedence.AND,
          		TokenType.OR: Precedence.OR,
          		TokenType.DOT: Precedence.INFIX
        }

    def Parse(self, tokens: list[Token]) -> list[Stmt]:
        self.__curPos = 0
        self.__tokens = tokens

        stmts: list[Stmt] = []
        while (not self.IsMatchCurToken(TokenType.END)):
            stmts.append(self.ParseStmt())

        self.__semanticAnalyzer.Analyze(stmts)

        return stmts

    def IsAtEnd(self) -> bool:
        return self.__curPos >= len(self.__tokens)

    def Consume(self, type, errMsg) -> Token:
        if self.IsMatchCurToken(type):
            return self.GetCurTokenAndStepOnce()
        Assert("[line "+str(self.GetCurToken().line)+"]:"+errMsg)
        return Token(TokenType.END, "", 0)

    def GetCurToken(self) -> Token:
        if not self.IsAtEnd():
            return self.__tokens[self.__curPos]
        return self.__tokens[-1]

    def GetCurTokenAndStepOnce(self) -> Token:
        if not self.IsAtEnd():
            result = self.__tokens[self.__curPos]
            self.__curPos += 1
            return result
        return self.__tokens[-1]

    def GetCurTokenPrecedence(self) -> Token:
        if self.__precedence.get(self.GetCurToken().type)==None:
            return Precedence.LOWEST
        return self.__precedence.get(self.GetCurToken().type)

    def GetNextToken(self) -> Token:
        if self.__curPos+1 < self.__tokens.count:
            return self.__tokens[self.__curPos+1]
        return self.__tokens[-1]

    def GetNextTokenAndStepOnce(self) -> Token:
        if self.__curPos+1 < self.__tokens.count:
            self.__curPos += 1
            return self.__tokens[self.__curPos]
        return self.__tokens[-1]

    def GetNextTokenPrecedence(self) -> Token:
        return self.__precedence.get(self.GetNextToken().type, default=Precedence.LOWEST)

    def IsMatchCurToken(self, type) -> bool:
        return self.GetCurToken().type == type

    def IsMatchCurTokenAndStepOnce(self, type) -> bool:
        if self.IsMatchCurToken(type):
            self.__curPos += 1
            return True
        return False

    def IsMatchNextToken(self, type) -> bool:
        return self.GetNextToken().type == type

    def IsMatchNextTokenAndStepOnce(self, type) -> bool:
        if self.IsMatchNextToken(type):
            self.__curPos += 1
            return True
        return False

    def ParseStmt(self) -> Stmt:
        if self.IsMatchCurToken(TokenType.VAR):
            return self.ParseVarStmt()
        elif self.IsMatchCurToken(TokenType.RETURN):
            return self.ParseReturnStmt()
        elif self.IsMatchCurToken(TokenType.IF):
            return self.ParseIfStmt()
        elif self.IsMatchCurToken(TokenType.LBRACE):
            return self.ParseScopeStmt()
        elif self.IsMatchCurToken(TokenType.WHILE):
            return self.ParseWhileStmt()
        elif self.IsMatchCurToken(TokenType.FUNCTION):
            return self.ParseFunctionStmt()
        elif self.IsMatchCurToken(TokenType.STRUCT):
            return self.ParseStructStmt()
        else:
            return self.ParseExprStmt()

    def ParseExprStmt(self) -> Stmt:
        exprStmt = ExprStmt(self.ParseExpr())
        self.Consume(TokenType.SEMICOLON, "Expect ';' after expr stmt.")
        return exprStmt

    def ParseVarStmt(self) -> Stmt:
        self.Consume(TokenType.VAR, "Expect 'var' key word")
        name = self.ParseIdentifierExpr()
        value = NilExpr()
        if self.IsMatchCurTokenAndStepOnce(TokenType.EQUAL):
            value = self.ParseExpr()
        self.Consume(TokenType.SEMICOLON, "Expect ';' after var stmt")
        return VarStmt(name, value)

    def ParseReturnStmt(self) -> Stmt:

        if self.__isInFunctionOrLambdascope==False:
            Assert("Return statement only available in function or lambda")

        self.Consume(TokenType.RETURN, "Expecr 'return' keyword")
        expr = None
        if not self.IsMatchCurToken(TokenType.SEMICOLON):
           expr = self.ParseExpr()
        self.Consume(TokenType.SEMICOLON, "Expect ';' after return stmt.")
        return ReturnStmt(expr)

    def ParseIfStmt(self) -> Stmt:
       self.Consume(TokenType.IF, "Expect 'if' key word.")
       self.Consume(TokenType.LPAREN, "Expect '(' after 'if'.")

       condition = self.ParseExpr()
       self.Consume(TokenType.RPAREN, "Expect ')' after if condition")

       thenBranch = self.ParseStmt()

       elseBranch = None
       if self.IsMatchCurTokenAndStepOnce(TokenType.ELSE):
           elseBranch = self.ParseStmt()

       return IfStmt(condition, thenBranch, elseBranch)

    def ParseScopeStmt(self) -> Stmt:
       self.Consume(TokenType.LBRACE, "Expect '{'.")
       scopeStmt = ScopeStmt([])
       while (not self.IsMatchCurToken(TokenType.RBRACE)):
           scopeStmt.stmts.append(self.ParseStmt())
       self.Consume(TokenType.RBRACE, "Expect '}'.")
       return scopeStmt

    def ParseWhileStmt(self) -> Stmt:
        self.Consume(TokenType.WHILE, "Expect 'while' keyword.")
        self.Consume(TokenType.LPAREN, "Expect '(' after 'while'.")

        condition = self.ParseExpr()

        self.Consume(TokenType.RPAREN,
                     "Expect ')' after while stmt's condition")

        body = self.ParseStmt()

        return WhileStmt(condition, body)

    def ParseFunctionStmt(self) -> Stmt:

        self.__isInFunctionOrLambdascope=True

        self.Consume(TokenType.FUNCTION, "Expect 'function' keyword")
        funcStmt = FunctionStmt("", [], None)
        funcStmt.name = self.ParseIdentifierExpr().Stringify()
        self.Consume(TokenType.LPAREN, "Expect '(' after function name")
 
        if (not self.IsMatchCurToken(TokenType.RPAREN)):
            idenExpr = self.ParseIdentifierExpr()
            funcStmt.parameters.append(idenExpr)
            while self.IsMatchCurTokenAndStepOnce(TokenType.COMMA):
                idenExpr = self.ParseIdentifierExpr()
                funcStmt.parameters.append(idenExpr)
        self.Consume(TokenType.RPAREN, "Expect ')' after function expr's '('")
        funcStmt.body = self.ParseScopeStmt()

        self.__isInFunctionOrLambdascope=False

        return funcStmt

    def ParseStructStmt(self) -> Stmt:
        self.Consume(TokenType.STRUCT, "Expect 'struct keyword'")
        structStmt = StructStmt("", [])
        structStmt.name = self.ParseIdentifierExpr().Stringify()
        self.Consume(TokenType.LBRACE, "Expect '{' after struct name")
        while not self.IsMatchCurToken(TokenType.RBRACE):
            structStmt.members.append(self.ParseVarStmt())
        self.Consume(TokenType.RBRACE, "Expect '}' after struct's '{'")

        return structStmt

    def ParseExpr(self, precedence=Precedence.LOWEST) -> Expr:
        if self.__prefixFunctions.get(self.GetCurToken().type) == None:
            print("no prefix definition for:" +
                  self.GetCurTokenAndStepOnce().literal)
            return NilExpr()
        prefixFn = self.__prefixFunctions.get(self.GetCurToken().type)
        leftExpr = prefixFn()
        while (not self.IsMatchCurToken(TokenType.SEMICOLON) and precedence < self.GetCurTokenPrecedence()):
            if self.__infixFunctions.get(self.GetCurToken().type) == None:
                return leftExpr
            infixFn = self.__infixFunctions[self.GetCurToken().type]
            leftExpr = infixFn(leftExpr)
        return leftExpr

    def ParseIdentifierExpr(self) -> Expr:
        literal=self.Consume(TokenType.IDENTIFIER, "Unexpect Identifier '"+self.GetCurToken().literal+".").literal
        return IdentifierExpr(literal)

    def ParseNumExpr(self) -> Expr:
        numLiteral = self.Consume(
            TokenType.NUMBER, "Expect a number literal.").literal
        return NumExpr(float(numLiteral))

    def ParseStrExpr(self) -> Expr:
        return StrExpr(self.Consume(TokenType.STRING, "Expact a string literal.").literal)

    def ParseNilExpr(self) -> Expr:
        self.Consume(TokenType.NIL, "Expect 'nil' keyword")
        return NilExpr()

    def ParseTrueExpr(self) -> Expr:
        self.Consume(TokenType.TRUE, "Expect 'true' keyword")
        return BoolExpr(True)

    def ParseFalseExpr(self) -> Expr:
        self.Consume(TokenType.FALSE, "Expect 'false' keyword")
        return BoolExpr(False)

    def ParseGroupExpr(self) -> Expr:
        self.Consume(TokenType.LPAREN, "Expect '('.")
        groupExpr = GroupExpr(self.ParseExpr())
        self.Consume(TokenType.RPAREN, "Expect ')'.")
        return groupExpr

    def ParseArrayExpr(self) -> Expr:
        self.Consume(TokenType.LBRACKET, "Expect '['.")
        arrayExpr = ArrayExpr([])
        if (not self.IsMatchCurToken(TokenType.RBRACKET)):
            arrayExpr.elements.append(self.ParseExpr())
            while self.IsMatchCurTokenAndStepOnce(TokenType.COMMA):
                arrayExpr.elements.append(self.ParseExpr())
        self.Consume(TokenType.RBRACKET, "Expect ']'.")
        return arrayExpr

    def ParsePrefixExpr(self) -> Expr:
        prefixExpr = PrefixExpr("", None)
        prefixExpr.op = self.GetCurTokenAndStepOnce().literal
        prefixExpr.right = self.ParseExpr(Precedence.PREFIX)
        return prefixExpr

    def ParseInfixExpr(self, prefixExpr: Expr) -> Expr:
        infixExpr = InfixExpr(None, "", None)
        infixExpr.left = prefixExpr
        opPrece = self.GetCurTokenPrecedence()
        infixExpr.op = self.GetCurTokenAndStepOnce().literal
        infixExpr.right = self.ParseExpr(opPrece)
        return infixExpr

    def ParseIndexExpr(self, prefixExpr: Expr) -> Expr:
        self.Consume(TokenType.LBRACKET, "Expect '['.")
        indexExpr = IndexExpr(None, None)
        indexExpr.ds = prefixExpr
        indexExpr.index = self.ParseExpr(Precedence.INFIX)
        self.Consume(TokenType.RBRACKET, "Expect ']'.")
        return indexExpr

    def ParseRefExpr(self)->Expr:
        self.Consume(TokenType.REF,"Expect 'ref' keyword.")
        refExpr=self.ParseExpr(Precedence.LOWEST)
        if refExpr.Type() != AstType.IDENTIFIER:
            Assert("Invalid reference type, only variable can be referenced.")

        return RefExpr(refExpr)

    def ParseLambdaExpr(self)->Expr:
        self.__isInFunctionOrLambdascope=True

        self.Consume(TokenType.LAMBDA,"Expect 'lambda' keyword.")
        self.Consume(TokenType.LPAREN,"Expect '(' after keyword 'lambda'.")
        parameters: list[IdentifierExpr] = []
        body: ScopeStmt = None
        if (not self.IsMatchCurToken(TokenType.RPAREN)):
           idenExpr = self.ParseIdentifierExpr()
           parameters.append(idenExpr)
           while self.IsMatchCurTokenAndStepOnce(TokenType.COMMA):
               idenExpr = self.ParseIdentifierExpr()
               parameters.append(idenExpr)
        self.Consume(TokenType.RPAREN, "Expect ')' after lambda expr's '('.")
        body = self.ParseScopeStmt()

        self.__isInFunctionOrLambdascope=False

        return LambdaExpr(parameters,body)

    def ParseAnonyStructExpr(self)->Expr:
        memPairs:dict[IdentifierExpr,Expr]={}
        self.Consume(TokenType.LBRACE,"Expect '{'.")
        while not self.IsMatchCurToken(TokenType.RBRACE):
            k=self.ParseIdentifierExpr()
            self.Consume(TokenType.COLON,"Expect ':'")
            v=self.ParseExpr()
            self.IsMatchCurTokenAndStepOnce(TokenType.COMMA)
            memPairs[k]=v

        self.Consume(TokenType.RBRACE,"Expect '}'.")
        return AnonyStructExpr(memPairs)

    def ParseFunctionCallExpr(self, prefixExpr: Expr) -> Expr:
        funcCallExpr = FunctionCallExpr("", [])
        funcCallExpr.name = prefixExpr
        self.Consume(TokenType.LPAREN, "Expect '('.")
        if not self.IsMatchCurToken(TokenType.RPAREN):
            funcCallExpr.arguments.append(self.ParseExpr())
            while self.IsMatchCurTokenAndStepOnce(TokenType.COMMA):
                funcCallExpr.arguments.append(self.ParseExpr())
        self.Consume(TokenType.RPAREN, "Expect ')'.")
        return funcCallExpr

    def ParseStructCallExpr(self, prefixExpr: Expr) -> Expr:
        self.Consume(TokenType.DOT, "Expect '.'.")
        structCallExpr = StructCallExpr(None, None)
        structCallExpr.callee = prefixExpr
        structCallExpr.callMember = self.ParseExpr(Precedence.INFIX)
        return structCallExpr
