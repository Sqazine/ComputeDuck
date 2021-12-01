from enum import Enum
from typing import Any

from Ast import Stmt
from Ast import Expr

from Token import Token, TokenType
from Utils import Assert
from py.Ast import ExprStmt


class Precedence(Enum):
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

    def __init__(self) -> None:
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

    def IsAtEnd(self) -> bool:
        return self.__curPos >= self.__tokens.count

    def Consume(self, type, errMsg) -> Token:
        if self.IsMatchCurToken(type):
            return self.GetCurTokenAndStepOnce()
        Assert("[line "+self.GetCurToken().line+"]:"+errMsg)
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
        return self.__precedence.get(self.GetCurToken().type, default=Precedence.LOWEST)

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
        exprStmt=ExprStmt(self.ParseExpr())
        self.Consume(TokenType.SEMICOLON,"Expect ';' after expr stmt.")
        return exprStmt

    def ParseVarStmt(self) -> Stmt:
        pass

    def ParseReturnStmt(self) -> Stmt:
       pass

    def ParseIfStmt(self) -> Stmt:
       pass

    def ParseScopeStmt(self) -> Stmt:
       pass

    def ParseWhileStmt(self) -> Stmt:
       pass

    def ParseFunctionStmt(self) -> Stmt:
       pass

    def ParseStructStmt(self) -> Stmt:
        pass

    def ParseExpr(self, precedence=Precedence.LOWEST) -> Expr:
        pass

    def ParseIdentifierExpr(self) -> Expr:
        pass

    def ParseNumExpr(self) -> Expr:
        pass

    def ParseStrExpr(self) -> Expr:
        pass

    def ParseNilExpr(self) -> Expr:
        pass

    def ParseTrueExpr(self) -> Expr:
        pass

    def ParseFalseExpr(self) -> Expr:
        pass

    def ParseGroupExpr(self) -> Expr:
        pass

    def ParseArrayExpr(self) -> Expr:
        pass

    def ParsePrefixExpr(self) -> Expr:
        pass

    def ParseInfixExpr(self, prefixExpr: Expr) -> Expr:
        pass

    def ParseConditionExpr(self, prefixExpr: Expr) -> Expr:
        pass

    def ParseIndexExpr(self, prefixExpr: Expr) -> Expr:
        pass

    def ParseFunctionCallExpr(self, prefixExpr: Expr) -> Expr:
        pass

    def ParseStructCallExpr(self, prefixExpr: Expr) -> Expr:
        pass
