from enum import Enum
from typing import Any

from Ast import Stmt
from Ast import Expr

from Token import Token, TokenType
from Utils import Assert


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
        return self.__curPos>=self.__tokens.count

    def Consume(self, type, errMsg) -> Token:
        pass

    def GetCurToken(self) -> Token:
        pass

    def GetCurTokenAndStepOnce(self) -> Token:
        pass

    def GetCurTokenPrecedence(self) -> Token:
        pass

    def GetNextToken(self) -> Token:
        pass

    def GetNextTokenAndStepOnce(self) -> Token:
        pass

    def GetNextTokenPrecedence(self) -> Token:
        pass

    def IsMatchCurToken(self, type) -> bool:
        pass

    def IsMatchCurTokenAndStepOnce(self, type) -> bool:
        pass

    def IsMatchNextToken(self, type) -> bool:
        pass

    def IsMatchNextTokenAndStepOnce(self, type) -> bool:
        pass

    def ParseStmt(self) -> Stmt:
        pass

    def ParseExprStmt(self) -> Stmt:
        pass

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
