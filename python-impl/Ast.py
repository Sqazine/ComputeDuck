from enum import IntEnum
import abc


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
    REF=10,
    FUNCTION=11,
    ANONY_STRUCT=12,
    FUNCTION_CALL = 13,
    STRUCT_CALL = 14,
    DLL_IMPORT=15
    #stmt
    EXPR = 16,
    RETURN = 17,
    IF = 18,
    SCOPE = 19,
    WHILE = 20,
    STRUCT = 21,


class AstNode:
    type:AstType

    def __init__(self,type) -> None:
        self.type=type

    @abc.abstractmethod
    def __str__(self) -> str:
        pass


class Expr(AstNode):
    def __init__(self,type) -> None:
        super().__init__(type)

    @abc.abstractmethod
    def __str__(self) -> str:
        pass


class NumExpr(Expr):
    value: float = 0.0

    def __init__(self, value) -> None:
        super().__init__(AstType.NUM)
        self.value = value

    def __str__(self) -> str:
        return str(self.value)


class StrExpr(Expr):
    value: str = ""

    def __init__(self, value) -> None:
        super().__init__(AstType.STR)
        self.value = value

    def __str__(self) -> str:
        return "\""+self.value+"\""


class NilExpr(Expr):
    def __init__(self) -> None:
        super().__init__(AstType.NIL)

    def __str__(self) -> str:
        return "nil"


class BoolExpr(Expr):
    value: bool = False

    def __init__(self, value) -> None:
        super().__init__(AstType.BOOL)
        self.value = value

    def __str__(self) -> str:
        return str(self.value)


class IdentifierExpr(Expr):
    literal: str = ""

    def __init__(self, literal) -> None:
        super().__init__(AstType.IDENTIFIER)
        self.literal = literal

    def __str__(self) -> str:
        return self.literal


class ArrayExpr(Expr):
    elements: list[Expr] = []

    def __init__(self, elements) -> None:
        super().__init__(AstType.ARRAY)
        self.elements = elements

    def __str__(self) -> str:
        result = "["
        if len(self.elements) > 0:
            for value in self.elements:
                result += value.__str__()+","
            result = result[0: len(result)-1]+"]"
        return result


class GroupExpr(Expr):
    expr: Expr = None

    def __init__(self, expr) -> None:
        super().__init__(AstType.GROUP)
        self.expr = expr

    def __str__(self) -> str:
        return "("+self.expr.__str__()+")"

class PrefixExpr(Expr):
    op: str = ""
    right: Expr = None

    def __init__(self, op, right) -> None:
        super().__init__(AstType.PREFIX)
        self.op = op
        self.right = right

    def __str__(self) -> str:
        return self.op+self.right.__str__()


class InfixExpr(Expr):
    left: Expr = None
    op: str = ""
    right: Expr = None

    def __init__(self, left, op, right) -> None:
        super().__init__(AstType.INFIX)
        self.left = left
        self.op = op
        self.right = right

    def __str__(self) -> str:
        return self.left.__str__()+self.op+self.right.__str__()


class IndexExpr(Expr):
    ds: Expr = None
    index: Expr = None

    def __init__(self, ds, index) -> None:
        super().__init__(AstType.INDEX)
        self.ds = ds
        self.index = index

    def __str__(self) -> str:
        return self.ds.__str__()+"["+self.index.__str__()+"]"
    
class RefExpr(Expr):
    refExpr:Expr

    def __init__(self,refExpr:Expr) -> None:
        super().__init__(AstType.REF)
        self.refExpr=refExpr

    def __str__(self) -> str:
        return "ref "+self.refExpr.__str__()


class FunctionCallExpr(Expr):
    name: Expr = None
    arguments: list[Expr] = []

    def __init__(self, name, arguments) -> None:
        super().__init__(AstType.FUNCTION_CALL)
        self.name = name
        self.arguments = arguments

    def __str__(self) -> str:
        result = self.name.__str__()+"("
        if len(self.arguments) > 0:
            for value in self.arguments:
                result += value.__str__()+","
            result = result[0: len(result)-1]
        return result+")"


class StructCallExpr(Expr):
    callee: Expr = None
    callMember: Expr = None

    def __init__(self, callee, callmember) -> None:
        super().__init__(AstType.STRUCT_CALL)
        self.callee = callee
        self.callMember = callmember

    def __str__(self) -> str:
        return self.callee.__str__()+"."+self.callMember.__str__()
    
class DllImportExpr(Expr):
    dllPath:str=None

    def __init__(self, dllPath) -> None:
        super().__init__(AstType.DLL_IMPORT)
        self.dllPath=dllPath

    def __str__(self) -> str:
        return "dllimport(\""+self.dllPath+"\")"

class Stmt(AstNode):
    @abc.abstractmethod
    def __str__(self) -> str:
        pass

class ExprStmt(Stmt):
    expr: Expr = None

    def __init__(self, expr) -> None:
        super().__init__(AstType.EXPR)
        self.expr = expr

    def __str__(self) -> str:
        return self.expr.__str__()+";"


class ReturnStmt(Stmt):
    expr: Expr = None

    def __init__(self, expr) -> None:
        super().__init__(AstType.RETURN)
        self.expr = expr

    def __str__(self) -> str:
        return "return "+self.expr.__str__()+";"


class IfStmt(Stmt):
    condition: Expr = None
    thenBranch: Stmt = None
    elseBranch: Stmt = None

    def __init__(self, condition, thenBranch, elseBranch) -> None:
        super().__init__(AstType.IF)
        self.condition = condition
        self.thenBranch = thenBranch
        self.elseBranch = elseBranch

    def __str__(self) -> str:
        result = "if("+self.condition.__str__()+")" + \
            self.thenBranch.__str__()
        if self.elseBranch != None:
            result += "else "+self.elseBranch.__str__()
        return result


class ScopeStmt(Stmt):
    stmts: list[Stmt] = []

    def __init__(self, stmts) -> None:
        super().__init__(AstType.SCOPE)
        self.stmts = stmts

    def __str__(self) -> str:
        result = "{"
        for s in self.stmts:
            result += s.__str__()
        result += "}"
        return result


class FunctionExpr(Expr):
    parameters: list[IdentifierExpr] = []
    body: ScopeStmt = None

    def __init__(self, parameters, body) -> None:
        super().__init__(AstType.FUNCTION)
        self.parameters = parameters
        self.body = body

    def __str__(self) -> str:
        result = "function("
        if len(self.parameters) > 0:
            for param in self.parameters:
                result += param.__str__()+","
            result = result[0:len(result)-1]
        result += ")"
        result += self.body.__str__()
        return result

class AnonyStructExpr(Expr):
    memberPairs: dict[IdentifierExpr,Expr] = {}
    
    def __init__(self,memberPairs=[]) -> None:
        super().__init__(AstType.ANONY_STRUCT)
        self.memberPairs = memberPairs

    def __str__(self) -> str:
        result = "{"
        for k,v in self.memberPairs.items():
            result += k.__str__()+":"+v.__str__()+",\n"
        result += "}"
        return result

class WhileStmt(Stmt):
    condition: Expr = None
    body: Stmt = None

    def __init__(self, condition, body) -> None:
        super().__init__(AstType.WHILE)
        self.condition = condition
        self.body = body

    def __str__(self) -> str:
        return "while("+self.condition.__str__()+")"+self.body.__str__()


class StructStmt(Stmt):
    name: str = ""
    members: dict[IdentifierExpr,Expr] = {}

    def __init__(self, name, members) -> None:
        super().__init__(AstType.STRUCT)
        self.name = name
        self.members = members

    def __str__(self) -> str:
        result = "struct "+self.name+"{"
        for value in self.members:
            result += value.__str__()+"\n"
        return result+"}"