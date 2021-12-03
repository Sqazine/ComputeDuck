from enum import IntEnum
import abc
from typing import List


class AstType(IntEnum):
    #expr
    NUM = 0,
    STR = 1,
    NIL = 2,
    BOOL = 3,
    IDENTIFIER = 4,
    GROUP = 5,
    ARRAY = 6,
    PREFIX = 7,
    INFIX = 8,
    INDEX = 9,
    FUNCTION_CALL = 10,
    STRUCT_CALL = 11,
    #stmt
    VAR = 12,
    EXPR = 13,
    RETURN = 14,
    IF = 15,
    SCOPE = 16,
    FUNCTION = 17,
    WHILE = 18,
    STRUCT = 19,


class AstNode:
    @abc.abstractmethod
    def Stringify(self) -> str:
        pass

    @abc.abstractmethod
    def Type(self) -> AstType:
        pass


class Expr(AstNode):
    @abc.abstractmethod
    def Stringify(self) -> str:
        pass

    @abc.abstractmethod
    def Type(self) -> AstType:
        pass


class NumExpr(Expr):
    value: float = 0.0

    def __init__(self, value) -> None:
        self.value = value

    def Stringify(self) -> str:
        return str(self.value)

    def Type(self) -> AstType:
        return AstType.NUM


class StrExpr(Expr):
    value: str = ""

    def __init__(self, value) -> None:
        self.value = value

    def Stringify(self) -> str:
        return "\""+self.value+"\""

    def Type(self) -> AstType:
        return AstType.STR


class NilExpr(Expr):
    def Stringify(self) -> str:
        return "nil"

    def Type(self) -> AstType:
        return AstType.NIL


class BoolExpr(Expr):
    value: bool = False

    def __init__(self, value) -> None:
        self.value = value

    def Stringify(self) -> str:
        return str(self.value)

    def Type(self) -> AstType:
        return AstType.BOOL


class IdentifierExpr(Expr):
    literal: str = ""

    def __init__(self, literal) -> None:
        self.literal = literal

    def Stringify(self) -> str:
        return self.literal

    def Type(self) -> AstType:
        return AstType.IDENTIFIER


class ArrayExpr(Expr):
    elements: list[Expr] = []

    def __init__(self, elements) -> None:
        self.elements = elements

    def Stringify(self) -> str:
        result = "["
        if self.elements.count > 0:
            for value in self.elements:
                result += value.Stringify()+","
            result = result[0: len(result)-1]
        return result

    def Type(self) -> AstType:
        return AstType.ARRAY


class GroupExpr(Expr):
    expr: Expr = None

    def __init__(self, expr) -> None:
        self.expr = expr

    def Stringify(self) -> str:
        return "("+self.expr.Stringify()+")"

    def Type(self) -> AstType:
        return AstType.GROUP


class PrefixExpr(Expr):
    op: str = ""
    right: Expr = None

    def __init__(self, op, right) -> None:
        self.op = op
        self.right = right

    def Stringify(self) -> str:
        return self.op+self.right.Stringify()

    def Type(self) -> AstType:
        return AstType.PREFIX


class InfixExpr(Expr):
    left: Expr = None
    op: str = ""
    right: Expr = None

    def __init__(self, left, op, right) -> None:
        self.left = left
        self.op = op
        self.right = right

    def Stringify(self) -> str:
        return self.left.Stringify()+self.op+self.right.Stringify()

    def Type(self) -> AstType:
        return AstType.INFIX


class IndexExpr(Expr):
    ds: Expr = None
    index: Expr = None

    def __init__(self, ds, index) -> None:
        self.ds = ds
        self.index = index

    def Stringify(self) -> str:
        return self.ds.Stringify()+"["+self.index.Stringify()+"]"

    def Type(self) -> AstType:
        return AstType.INDEX


class FunctionCallExpr(Expr):
    name: str = ""
    arguments: list[Expr] = []

    def __init__(self, name, arguments) -> None:
        self.name = name
        self.arguments = arguments

    def Stringify(self) -> str:
        result = self.name+"("
        if len(self.arguments) > 0:
            for value in self.arguments:
                result += value.Stringify()+","
            result = result[0: len(result)-1]
        return result+")"

    def Type(self) -> AstType:
        return AstType.FUNCTION_CALL


class StructCallExpr(Expr):
    callee: Expr = None
    callMember: Expr = None

    def __init__(self, callee, callmember) -> None:
        self.callee = callee
        self.callMember = callmember

    def Stringify(self) -> str:
        return self.callee.Stringify()+"."+self.callMember.Stringify()

    def Type(self) -> AstType:
        return AstType.STRUCT_CALL


class Stmt(AstNode):
    @abc.abstractmethod
    def Stringify(self) -> str:
        pass

    @abc.abstractmethod
    def Type(self) -> AstType:
        pass


class ExprStmt(Stmt):
    expr: Expr = None

    def __init__(self, expr) -> None:
        self.expr = expr

    def Stringify(self) -> str:
        return self.expr.Stringify()+";"

    def Type(self) -> AstType:
        return AstType.EXPR


class VarStmt(Stmt):
    value: Expr = None
    name: IdentifierExpr = None

    def __init__(self, name, value) -> None:
        self.name = name
        self.value = value

    def Stringify(self) -> str:
        return "var "+self.name.Stringify()+" = "+self.value.Stringify()+";"

    def Type(self) -> AstType:
        return AstType.VAR


class ReturnStmt(Stmt):
    expr: Expr = None

    def __init__(self, expr) -> None:
        self.expr = expr

    def Stringify(self) -> str:
        return "return "+self.expr.Stringify()+";"

    def Type(self) -> AstType:
        return AstType.RETURN


class IfStmt(Stmt):
    condition: Expr = None
    thenBranch: Stmt = None
    elseBranch: Stmt = None

    def __init__(self, condition, thenBranch, elseBranch) -> None:
        self.condition = condition
        self.thenBranch = thenBranch
        self.elseBranch = elseBranch

    def Stringify(self) -> str:
        result = "if("+self.condition.Stringify()+")" + \
            self.thenBranch.Stringify()
        if self.elseBranch != None:
            result += "else "+self.elseBranch.Stringify()

    def Type(self) -> AstType:
        return AstType.IF


class ScopeStmt(Stmt):
    stmts: list[Stmt] = []

    def __init__(self, stmts) -> None:
        self.stmts = stmts

    def Stringify(self) -> str:
        result = "{"
        for value in self.stmts:
            result += value.Stringify()
        result += "}"
        return result

    def Type(self) -> AstType:
        return AstType.SCOPE


class FunctionStmt(Stmt):
    name: str = ""
    parameters: list[IdentifierExpr] = []
    body: ScopeStmt = None

    def __init__(self, name,parameters, body) -> None:
        self.name = name
        self.parameters = parameters
        self.body = body

    def Stringify(self) -> str:
        result = "fn "+self.name+"("
        if len(self.parameters) > 0:
            for param in self.parameters:
                result += param.Stringify()+","
            result = result[0:len(result)-1]
        result += ")"
        result += self.body.Stringify()
        return result

    def Type(self) -> AstType:
        return AstType.FUNCTION


class WhileStmt(Stmt):
    condition: Expr = None
    body: Stmt = None

    def __init__(self, condition, body) -> None:
        self.condition = condition
        self.body = body

    def Stringify(self) -> str:
        return "while("+self.condition.Stringify()+")"+self.body.Stringify()

    def Type(self) -> AstType:
        return AstType.WHILE


class StructStmt(Stmt):
    name: str = ""
    members: list[VarStmt] = []

    def __init__(self, name, members) -> None:
        self.name = name
        self.members = members

    def Stringify(self) -> str:
        result = "struct "+self.name+"{"
        for value in self.members:
            result += value.Stringify()+"\n"
        return result+"}"

    def Type(self) -> AstType:
        return AstType.STRUCT
