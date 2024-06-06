from enum import IntEnum
from typing import Any
from Token import Token, TokenType
from Utils import error
from Ast import *
from ConstantFolder import *

class Precedence(IntEnum):
    LOWEST = 0,  # ,
    ASSIGN = 1,		# =
    OR = 2,			# or
    AND = 3,		# and
    BIT_OR = 3,		# |
    BIT_XOR = 4,	# ^
    BIT_AND = 5,	# &
    EQUAL = 6,		# == !=
    COMPARE = 7,  # < <= > >=
    ADD_PLUS = 8,  # + -
    MUL_DIV = 9,  # * /
    PREFIX = 10,		# not -
    INFIX = 11,		# [] () .


class Parser:
    __curPos: int = 0
    __tokens: list[Token] = []
    __prefixFunctions: dict[TokenType, Any] = {}
    __infixFunctions: dict[TokenType, Any] = {}
    __precedence: dict[TokenType, Any] = {}
    __functionScopeDepth = 0
    __constantFolder = ConstantFolder()

    def __init__(self) -> None:
        self.__curPos: int = 0
        self.__tokens: list[Token] = []
        self.__prefixFunctions: dict[TokenType, Any] = {}
        self.__infixFunctions: dict[TokenType, Any] = {}
        self.__precedence: dict[TokenType, Any] = {}

        self.__prefixFunctions = {
            TokenType.IDENTIFIER: self.__parse_identifier_expr,
            TokenType.NUMBER: self.__parse_num_expr,
            TokenType.STRING: self.__parse_str_expr,
            TokenType.NIL: self.__parse_nil_expr,
            TokenType.TRUE: self.__parse_true_expr,
            TokenType.FALSE: self.__parse_false_expr,
            TokenType.MINUS: self.__parse_prefix_expr,
            TokenType.NOT: self.__parse_prefix_expr,
            TokenType.LPAREN: self.__parse_group_expr,
            TokenType.LBRACKET: self.__parse_array_expr,
            TokenType.REF: self.__parse_ref_expr,
            TokenType.LBRACE: self.__parse_struct_expr,
            TokenType.FUNCTION: self.__parse_function_expr,
            TokenType.DLLIMPORT: self.__parse_dll_import_expr,
            TokenType.TILDE:self.__parse_prefix_expr,
        }

        self.__infixFunctions = {
            TokenType.EQUAL: self.__parse_infix_expr,
            TokenType.EQUAL_EQUAL: self.__parse_infix_expr,
            TokenType.BANG_EQUAL: self.__parse_infix_expr,
            TokenType.LESS: self.__parse_infix_expr,
            TokenType.LESS_EQUAL: self.__parse_infix_expr,
            TokenType.GREATER: self.__parse_infix_expr,
            TokenType.GREATER_EQUAL: self.__parse_infix_expr,
            TokenType.PLUS: self.__parse_infix_expr,
            TokenType.MINUS: self.__parse_infix_expr,
            TokenType.ASTERISK: self.__parse_infix_expr,
            TokenType.SLASH: self.__parse_infix_expr,
            TokenType.LPAREN: self.__parse_function_call_expr,
            TokenType.LBRACKET: self.__parse_index_expr,
            TokenType.AND: self.__parse_infix_expr,
            TokenType.OR: self.__parse_infix_expr,
            TokenType.DOT: self.__parse_struct_call_expr,
            TokenType.AMPERSAND:self.__parse_infix_expr,
            TokenType.VBAR:self.__parse_infix_expr,
            TokenType.CARET:self.__parse_infix_expr
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
          	TokenType.DOT: Precedence.INFIX,
          	TokenType.AMPERSAND: Precedence.BIT_AND,
          	TokenType.VBAR: Precedence.BIT_OR,
          	TokenType.CARET: Precedence.BIT_XOR,
        }

    def parse(self, tokens: list[Token]) -> list[Stmt]:
        self.__curPos = 0
        self.__tokens = tokens
        self.__functionScopeDepth = 0

        stmts: list[Stmt] = []
        while (not self.__is_match_cur_token(TokenType.END)):
            stmts.append(self.__parse_stmt())

        self.__constantFolder.fold(stmts)

        return stmts

    def __is_at_end(self) -> bool:
        return self.__curPos >= len(self.__tokens)

    def __consume(self, type, errMsg) -> Token:
        if self.__is_match_cur_token(type):
            return self.__get_cur_token_and_step_once()
        error("[line "+str(self.__get_cur_token().line)+"]:"+errMsg)

    def __get_cur_token(self) -> Token:
        if not self.__is_at_end():
            return self.__tokens[self.__curPos]
        return self.__tokens[-1]

    def __get_cur_token_and_step_once(self) -> Token:
        if not self.__is_at_end():
            result = self.__tokens[self.__curPos]
            self.__curPos += 1
            return result
        return self.__tokens[-1]

    def __get_cur_token_precedence(self) -> Token:
        if self.__precedence.get(self.__get_cur_token().type) == None:
            return Precedence.LOWEST
        return self.__precedence.get(self.__get_cur_token().type)
    
    def __is_match_cur_token(self, type) -> bool:
        return self.__get_cur_token().type == type

    def __is_match_cur_token_and_step_once(self, type) -> bool:
        if self.__is_match_cur_token(type):
            self.__curPos += 1
            return True
        return False

    def __parse_stmt(self) -> Stmt:
        if self.__is_match_cur_token(TokenType.RETURN):
            return self.__parse_return_stmt()
        elif self.__is_match_cur_token(TokenType.IF):
            return self.__parse_if_stmt()
        elif self.__is_match_cur_token(TokenType.LBRACE):
            return self.__parse_scope_stmt()
        elif self.__is_match_cur_token(TokenType.WHILE):
            return self.__parse_while_stmt()
        elif self.__is_match_cur_token(TokenType.STRUCT):
            return self.__parse_struct_stmt()
        else:
            return self.__parse_exprStmt()

    def __parse_exprStmt(self) -> Stmt:
        exprStmt = ExprStmt(self.__parse_expr())
        self.__consume(TokenType.SEMICOLON, "Expect ';' after expr stmt.")
        return exprStmt

    def __parse_return_stmt(self) -> Stmt:
        if self.__functionScopeDepth == 0:
            error("Return statement only available in function or lambda")

        self.__consume(TokenType.RETURN, "Expecr 'return' keyword")
        expr = None
        if not self.__is_match_cur_token(TokenType.SEMICOLON):
           expr = self.__parse_expr()
        self.__consume(TokenType.SEMICOLON, "Expect ';' after return stmt.")
        return ReturnStmt(expr)

    def __parse_if_stmt(self) -> Stmt:
       self.__consume(TokenType.IF, "Expect 'if' key word.")
       self.__consume(TokenType.LPAREN, "Expect '(' after 'if'.")

       condition = self.__parse_expr()
       self.__consume(TokenType.RPAREN, "Expect ')' after if condition")

       thenBranch = self.__parse_stmt()

       elseBranch = None
       if self.__is_match_cur_token_and_step_once(TokenType.ELSE):
           elseBranch = self.__parse_stmt()

       return IfStmt(condition, thenBranch, elseBranch)

    def __parse_scope_stmt(self) -> Stmt:
       self.__consume(TokenType.LBRACE, "Expect '{'.")
       scopeStmt = ScopeStmt([])
       while (not self.__is_match_cur_token(TokenType.RBRACE)):
           scopeStmt.stmts.append(self.__parse_stmt())
       self.__consume(TokenType.RBRACE, "Expect '}'.")
       return scopeStmt

    def __parse_while_stmt(self) -> Stmt:
        self.__consume(TokenType.WHILE, "Expect 'while' keyword.")
        self.__consume(TokenType.LPAREN, "Expect '(' after 'while'.")

        condition = self.__parse_expr()

        self.__consume(TokenType.RPAREN,"Expect ')' after while stmt's condition")

        body = self.__parse_stmt()

        return WhileStmt(condition, body)

    def __parse_struct_stmt(self) -> Stmt:
        self.__consume(TokenType.STRUCT, "Expect 'struct keyword'")
        structStmt = StructStmt("", {})
        structStmt.name = self.__parse_identifier_expr().__str__()
        self.__consume(TokenType.LBRACE, "Expect '{' after struct name")
        while not self.__is_match_cur_token(TokenType.RBRACE):
            k = self.__parse_identifier_expr()
            v = NilExpr()

            if self.__is_match_cur_token(TokenType.COLON):
                self.__consume(TokenType.COLON, "Expect ':'")
                v = self.__parse_expr()

            self.__is_match_cur_token_and_step_once(TokenType.COMMA)
            structStmt.members[k] = v

        self.__consume(TokenType.RBRACE, "Expect '}' after struct's '{'")

        return structStmt

    def __parse_expr(self, precedence=Precedence.LOWEST) -> Expr:
        if self.__prefixFunctions.get(self.__get_cur_token().type) == None:
            print("no prefix definition for:" +
                  self.__get_cur_token_and_step_once().literal)
            return NilExpr()
        prefixFn = self.__prefixFunctions.get(self.__get_cur_token().type)
        leftExpr = prefixFn()
        while (not self.__is_match_cur_token(TokenType.SEMICOLON) and precedence < self.__get_cur_token_precedence()):
            if self.__infixFunctions.get(self.__get_cur_token().type) == None:
                return leftExpr
            infixFn = self.__infixFunctions[self.__get_cur_token().type]
            leftExpr = infixFn(leftExpr)
        return leftExpr

    def __parse_identifier_expr(self) -> Expr:
        literal = self.__consume(
            TokenType.IDENTIFIER, "Unexpect Identifier '"+self.__get_cur_token().literal+".").literal
        return IdentifierExpr(literal)

    def __parse_num_expr(self) -> Expr:
        numLiteral = self.__consume(
            TokenType.NUMBER, "Expect a number literal.").literal
        return NumExpr(float(numLiteral))

    def __parse_str_expr(self) -> Expr:
        return StrExpr(self.__consume(TokenType.STRING, "Expact a string literal.").literal)

    def __parse_nil_expr(self) -> Expr:
        self.__consume(TokenType.NIL, "Expect 'nil' keyword")
        return NilExpr()

    def __parse_true_expr(self) -> Expr:
        self.__consume(TokenType.TRUE, "Expect 'true' keyword")
        return BoolExpr(True)

    def __parse_false_expr(self) -> Expr:
        self.__consume(TokenType.FALSE, "Expect 'false' keyword")
        return BoolExpr(False)

    def __parse_group_expr(self) -> Expr:
        self.__consume(TokenType.LPAREN, "Expect '('.")
        groupExpr = GroupExpr(self.__parse_expr())
        self.__consume(TokenType.RPAREN, "Expect ')'.")
        return groupExpr

    def __parse_array_expr(self) -> Expr:
        self.__consume(TokenType.LBRACKET, "Expect '['.")
        arrayExpr = ArrayExpr([])
        if (not self.__is_match_cur_token(TokenType.RBRACKET)):
            arrayExpr.elements.append(self.__parse_expr())
            while self.__is_match_cur_token_and_step_once(TokenType.COMMA):
                arrayExpr.elements.append(self.__parse_expr())
        self.__consume(TokenType.RBRACKET, "Expect ']'.")
        return arrayExpr

    def __parse_prefix_expr(self) -> Expr:
        prefixExpr = PrefixExpr("", None)
        prefixExpr.op = self.__get_cur_token_and_step_once().literal
        prefixExpr.right = self.__parse_expr(Precedence.PREFIX)
        return prefixExpr

    def __parse_infix_expr(self, prefixExpr: Expr) -> Expr:
        infixExpr = InfixExpr(None, "", None)
        infixExpr.left = prefixExpr
        opPrece = self.__get_cur_token_precedence()
        infixExpr.op = self.__get_cur_token_and_step_once().literal
        infixExpr.right = self.__parse_expr(opPrece)
        return infixExpr

    def __parse_index_expr(self, prefixExpr: Expr) -> Expr:
        self.__consume(TokenType.LBRACKET, "Expect '['.")
        indexExpr = IndexExpr(None, None)
        indexExpr.ds = prefixExpr
        indexExpr.index = self.__parse_expr(Precedence.INFIX)
        self.__consume(TokenType.RBRACKET, "Expect ']'.")
        return indexExpr

    def __parse_ref_expr(self) -> Expr:
        self.__consume(TokenType.REF, "Expect 'ref' keyword.")
        refExpr = self.__parse_expr(Precedence.LOWEST)
        return RefExpr(refExpr)

    def __parse_function_expr(self) -> Stmt:

        self.__functionScopeDepth += 1

        self.__consume(TokenType.FUNCTION, "Expect 'function' keyword")
        funcExpr = FunctionExpr([], None)
        self.__consume(TokenType.LPAREN, "Expect '(' after function name")

        if (not self.__is_match_cur_token(TokenType.RPAREN)):
            idenExpr = self.__parse_identifier_expr()
            funcExpr.parameters.append(idenExpr)
            while self.__is_match_cur_token_and_step_once(TokenType.COMMA):
                idenExpr = self.__parse_identifier_expr()
                funcExpr.parameters.append(idenExpr)
        self.__consume(TokenType.RPAREN, "Expect ')' after function expr's '('")
        funcExpr.body = self.__parse_scope_stmt()

        self.__functionScopeDepth -= 1

        return funcExpr

    def __parse_struct_expr(self) -> Expr:
        memPairs: dict[IdentifierExpr, Expr] = {}
        self.__consume(TokenType.LBRACE, "Expect '{'.")
        while not self.__is_match_cur_token(TokenType.RBRACE):
            k = self.__parse_identifier_expr()
            self.__consume(TokenType.COLON, "Expect ':'")
            v = self.__parse_expr()
            self.__is_match_cur_token_and_step_once(TokenType.COMMA)
            memPairs[k] = v

        self.__consume(TokenType.RBRACE, "Expect '}'.")
        return StructExpr(memPairs)

    def __parse_function_call_expr(self, prefixExpr: Expr) -> Expr:
        funcCallExpr = FunctionCallExpr("", [])
        funcCallExpr.name = prefixExpr
        self.__consume(TokenType.LPAREN, "Expect '('.")
        if not self.__is_match_cur_token(TokenType.RPAREN):
            funcCallExpr.arguments.append(self.__parse_expr())
            while self.__is_match_cur_token_and_step_once(TokenType.COMMA):
                funcCallExpr.arguments.append(self.__parse_expr())
        self.__consume(TokenType.RPAREN, "Expect ')'.")
        return funcCallExpr

    def __parse_struct_call_expr(self, prefixExpr: Expr) -> Expr:
        self.__consume(TokenType.DOT, "Expect '.'.")
        structCallExpr = StructCallExpr(None, None)
        structCallExpr.callee = prefixExpr
        structCallExpr.callMember = self.__parse_expr(Precedence.INFIX)
        return structCallExpr

    def __parse_dll_import_expr(self) -> Expr:
        self.__consume(TokenType.DLLIMPORT, "Expect 'dllimport' keyword")
        self.__consume(TokenType.LPAREN, "Expect '(' after 'dllimport' keyword")

        path = self.__consume(TokenType.STRING, "Expect dll path").literal

        self.__consume(TokenType.RPAREN, "Expect ')' after dllimoport expr")

        return DllImportExpr(path)
