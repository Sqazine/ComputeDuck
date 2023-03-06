from enum import IntEnum
from typing import Any
from Token import Token, TokenType
from Utils import Assert
from Ast import *
from ConstantFolder import *

import platform


class Precedence(IntEnum):
    LOWEST = 0,  # ,
    ASSIGN = 1,		# =
    OR = 2,			# or
    AND = 3,		# and
    EQUAL = 4,		# == !=
    COMPARE = 5,  # < <= > >=
    ADD_PLUS = 6,  # + -
    MUL_DIV = 7,  # * /
    PREFIX = 8,		# not -
    INFIX = 9,		# [] () .


class Parser:
    __curPos: int = 0
    __tokens: list[Token] = []
    __prefixFunctions: dict[TokenType, Any] = {}
    __infixFunctions: dict[TokenType, Any] = {}
    __precedence: dict[TokenType, Any] = {}
    __functionOrFunctionScopeDepth=0
    __constantFolder=ConstantFolder()

    def __init__(self) -> None:
        self.__curPos: int = 0
        self.__tokens: list[Token] = []
        self.__prefixFunctions: dict[TokenType, Any] = {}
        self.__infixFunctions: dict[TokenType, Any] = {}
        self.__precedence: dict[TokenType, Any] = {}
        
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
            TokenType.LBRACE:self.ParseAnonyStructExpr,
            TokenType.FUNCTION:self.ParseFunctionExpr,
            TokenType.DLLIMPORT:self.ParseDllImportExpr
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
        self.__functionOrFunctionScopeDepth=0

        stmts: list[Stmt] = []
        while (not self.IsMatchCurToken(TokenType.END)):
            stmts.append(self.ParseStmt())

        self.__constantFolder.Fold(stmts)

        return stmts

    def IsAtEnd(self) -> bool:
        return self.__curPos >= len(self.__tokens)

    def Consume(self, type, errMsg) -> Token:
        if self.IsMatchCurToken(type):
            return self.GetCurTokenAndStepOnce()
        Assert("[line "+str(self.GetCurToken().line)+"]:"+errMsg)

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
        if self.IsMatchCurToken(TokenType.RETURN):
            return self.ParseReturnStmt()
        elif self.IsMatchCurToken(TokenType.IF):
            return self.ParseIfStmt()
        elif self.IsMatchCurToken(TokenType.LBRACE):
            return self.ParseScopeStmt()
        elif self.IsMatchCurToken(TokenType.WHILE):
            return self.ParseWhileStmt()
        elif self.IsMatchCurToken(TokenType.STRUCT):
            return self.ParseStructStmt()
        else:
            return self.ParseExprStmt()

    def ParseExprStmt(self) -> Stmt:
        exprStmt = ExprStmt(self.ParseExpr())
        self.Consume(TokenType.SEMICOLON, "Expect ';' after expr stmt.")
        return exprStmt

    def ParseReturnStmt(self) -> Stmt:

        if self.__functionOrFunctionScopeDepth==0:
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

    def ParseStructStmt(self) -> Stmt:
        self.Consume(TokenType.STRUCT, "Expect 'struct keyword'")
        structStmt = StructStmt("", {})
        structStmt.name = self.ParseIdentifierExpr().__str__()
        self.Consume(TokenType.LBRACE, "Expect '{' after struct name")
        while not self.IsMatchCurToken(TokenType.RBRACE):
            k=self.ParseIdentifierExpr()
            v=NilExpr()

            if self.IsMatchCurToken(TokenType.COLON):
                self.Consume(TokenType.COLON,"Expect ':'")
                v=self.ParseExpr()

            self.IsMatchCurTokenAndStepOnce(TokenType.COMMA)
            structStmt.members[k]=v

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
        return RefExpr(refExpr)
    
    
    def ParseFunctionExpr(self) -> Stmt:

        self.__functionOrFunctionScopeDepth+=1

        self.Consume(TokenType.FUNCTION, "Expect 'function' keyword")
        funcExpr = FunctionExpr([], None)
        self.Consume(TokenType.LPAREN, "Expect '(' after function name")
 
        if (not self.IsMatchCurToken(TokenType.RPAREN)):
            idenExpr = self.ParseIdentifierExpr()
            funcExpr.parameters.append(idenExpr)
            while self.IsMatchCurTokenAndStepOnce(TokenType.COMMA):
                idenExpr = self.ParseIdentifierExpr()
                funcExpr.parameters.append(idenExpr)
        self.Consume(TokenType.RPAREN, "Expect ')' after function expr's '('")
        funcExpr.body = self.ParseScopeStmt()

        self.__functionOrFunctionScopeDepth-=1

        return funcExpr
    
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

    def ParseDllImportExpr(self)->Expr:
        self.Consume(TokenType.DLLIMPORT,"Expect 'dllimport' keyword")
        self.Consume(TokenType.LPAREN,"Expect '(' after 'dllimport' keyword")

        path=self.Consume(TokenType.STRING,"Expect dll path").literal

        self.Consume(TokenType.RPAREN,"Expect ')' after dllimoport expr")

        sysstr=platform.system()

        if path.find(".")==-1:
            if sysstr=="Windows":
                path+=".dll"
            elif sysstr=="Linux":
                path+=".so"
        
        return DllImportExpr(path)
