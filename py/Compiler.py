from enum import IntEnum
from typing import List
from Utils import Assert
from Ast import Stmt
from Ast import ArrayExpr, BoolExpr, Expr, ExprStmt, FunctionCallExpr, FunctionStmt, GroupExpr, IdentifierExpr, IfStmt, IndexExpr, InfixExpr, NilExpr, NumExpr, PrefixExpr, ReturnStmt, ScopeStmt, StrExpr, StructCallExpr, StructStmt, VarStmt, WhileStmt, RefExpr, LambdaExpr
from SymbolTable import SymbolTable, Symbol
from Chunk import Chunk
from Object import Object
from Ast import AstType
from Chunk import OpCode
from Object import FunctionObject, NilObject, StrObject
from SymbolTable import SymbolScope
from Object import NumObject
from Object import BoolObject


class RWState(IntEnum):
    READ = 0,
    WRITE = 1,


CONSTANT_MAX = 8192


class Compiler:
    __constants: List[Object]=[]
    __constantCount: int
    __scopes: List[List[int]]=[]
    __scopeIndex: int
    __symbolTable: SymbolTable
    __builtinFnIndex = ["print", "println",
                        "sizeof", "insert", "erase", "clock"]

    def __init__(self) -> None:
        self.__symbolTable = None
        self.ResetStatus()

    def Compile(self, stmts: List[Stmt]) -> Chunk:
        for stmt in stmts:
            self.__CompileStmt(stmt)
        return Chunk(self.__CurOpCodes(), self.__constants,self.__constantCount)

    def ResetStatus(self) -> None:
        self.__constants=[Object]*CONSTANT_MAX
        for i in range(0, CONSTANT_MAX):
            self.__constants[i] = NilObject()

        self.__constantCount = 0
        self.__scopeIndex = 0
        self.__scopes = []
        self.__scopes.append([])

        self.__symbolTable = SymbolTable()

        for i in range(0, len(self.__builtinFnIndex)):
            self.__symbolTable.DefineBuiltin(self.__builtinFnIndex[i], i)

    def __CompileStmt(self, stmt: Stmt) -> None:
        if stmt.Type() == AstType.RETURN:
            self.__CompileReturnStmt(stmt)
        elif stmt.Type() == AstType.EXPR:
            self.__CompileExprStmt(stmt)
        elif stmt.Type() == AstType.VAR:
            self.__CompileVarStmt(stmt)
        elif stmt.Type() == AstType.SCOPE:
            self.__CompileScopeStmt(stmt)
        elif stmt.Type() == AstType.IF:
            self.__CompileIfStmt(stmt)
        elif stmt.Type() == AstType.WHILE:
            self.__CompileWhileStmt(stmt)
        elif stmt.Type() == AstType.FUNCTION:
            self.__CompileFunctionStmt(stmt)
        elif stmt.Type() == AstType.STRUCT:
            self.__CompileStructStmt(stmt)

    def __CompileExprStmt(self, stmt: ExprStmt) -> None:
        self.__CompileExpr(stmt.expr)

    def __CompileIfStmt(self, stmt: IfStmt) -> None:
        self.__CompileExpr(stmt.condition)
        self.__Emit(OpCode.OP_JUMP_IF_FALSE)
        jumpIfFalseAddress = self.__Emit(65536)
        self.__CompileStmt(stmt.thenBranch)
        self.__Emit(OpCode.OP_JUMP)
        jumpAddress = self.__Emit(65536)

        self.__ModifyOpCode(jumpIfFalseAddress, len(self.__CurOpCodes())-1)

        if stmt.elseBranch:
            self.__CompileStmt(stmt.elseBranch)

        self.__ModifyOpCode(jumpAddress, len(self.__CurOpCodes())-1)

    def __CompileScopeStmt(self, stmt: ScopeStmt) -> None:
        self.__EnterScope()
        self.__Emit(OpCode.OP_SP_OFFSET)

        idx = self.__Emit(0)

        for s in stmt.stmts:
            self.__CompileStmt(s)

        localVarCount = self.__symbolTable.definitionCount
        self.__CurOpCodes()[idx] = localVarCount

        self.__Emit(OpCode.OP_SP_OFFSET)
        self.__Emit(-localVarCount)

        opCodes = self.__ExitScope()

        self.__CurOpCodes().extend(opCodes)

    def __CompileWhileStmt(self, stmt: WhileStmt) -> None:
        jumpAddress = len(self.__CurOpCodes())-1
        self.__CompileExpr(stmt.condition)

        self.__Emit(OpCode.OP_JUMP_IF_FALSE)
        jumpIfFalseAddress = self.__Emit(65536)

        self.__CompileStmt(stmt.body)

        self.__Emit(OpCode.OP_JUMP)
        self.__Emit(jumpAddress)

        self.__ModifyOpCode(jumpIfFalseAddress, len(self.__CurOpCodes())-1)

    def __CompileReturnStmt(self, stmt: ReturnStmt) -> None:
        if stmt.expr != None:
            self.__CompileExpr(stmt.expr)
            self.__Emit(OpCode.OP_RETURN)
            self.__Emit(1)
        else:
            self.__Emit(OpCode.OP_RETURN)
            self.__Emit(0)

    def __CompileVarStmt(self, stmt: VarStmt) -> None:
        self.__CompileExpr(stmt.value)
        symbol = self.__symbolTable.Define(stmt.name.literal)
        if symbol.scope == SymbolScope.GLOBAL:
            self.__Emit(OpCode.OP_SET_GLOBAL)
        else:
            self.__Emit(OpCode.OP_SET_LOCAL)
            self.__Emit(symbol.isInUpScope)
            self.__Emit(symbol.scopeDepth)

        self.__Emit(symbol.index)

    def __CompileFunctionStmt(self, stmt: FunctionStmt) -> None:
        symbol = self.__symbolTable.Define(stmt.name)

        self.__EnterScope()

        self.__symbolTable.DefineFunction(stmt.name)

        for param in stmt.parameters:
            self.__symbolTable.Define(param.literal)

        for s in stmt.body.stmts:
            self.__CompileStmt(s)

        localVarCount = self.__symbolTable.definitionCount
        opCodes = self.__ExitScope()

        opCodesLen = len(opCodes)
        if opCodesLen == 0 or opCodes[opCodesLen-2] != OpCode.OP_RETURN:
            opCodes.append(OpCode.OP_RETURN)
            opCodes.append(0)

        fn = FunctionObject(opCodes, localVarCount, len(stmt.parameters))

        self.__EmitConstant(self.__AddConstant(fn))

        if symbol.scope == SymbolScope.GLOBAL:
            self.__Emit(OpCode.OP_SET_GLOBAL)
        else:
            self.__Emit(OpCode.OP_SET_LOCAL)
            self.__Emit(symbol.isInUpScope)
            self.__Emit(symbol.scopeDepth)

        self.__Emit(symbol.index)

    def __CompileStructStmt(self, stmt: StructStmt) -> None:
        symbol = self.__symbolTable.Define(stmt.name, True)

        self.__EnterScope()

        for i in range(len(stmt.members)-1, -1, -1):
            self.__CompileExpr(stmt.members[i].value)
            self.__EmitConstant(self.__AddConstant(
                StrObject(stmt.members[i].name.literal)))

        localVarCount = self.__symbolTable.definitionCount

        self.__Emit(OpCode.OP_STRUCT)
        self.__Emit(len(stmt.members))

        opCodes = self.__ExitScope()

        opCodes.append(OpCode.OP_RETURN)
        opCodes.append(1)

        fn = FunctionObject(opCodes, localVarCount)

        self.__EmitConstant(self.__AddConstant(fn))

        if symbol.scope == SymbolScope.GLOBAL:
            self.__Emit(OpCode.OP_SET_GLOBAL)
        else:
            self.__Emit(OpCode.OP_SET_LOCAL)
            self.__Emit(symbol.isInUpScope)
            self.__Emit(symbol.scopeDepth)

        self.__Emit(symbol.index)

    def __CompileExpr(self, expr: Expr, state: RWState = RWState.READ) -> None:
        if expr.Type() == AstType.NUM:
            self.__CompileNumExpr(expr)
        elif expr.Type() == AstType.STR:
            self.__CompileStrExpr(expr)
        elif expr.Type() == AstType.BOOL:
            self.__CompileBoolExpr(expr)
        elif expr.Type() == AstType.NIL:
            self.__CompileNilExpr(expr)
        elif expr.Type() == AstType.IDENTIFIER:
            self.__CompileIdentifierExpr(expr,state)
        elif expr.Type() == AstType.GROUP:
            self.__CompileGroupExpr(expr)
        elif expr.Type() == AstType.ARRAY:
            self.__CompileArrayExpr(expr)
        elif expr.Type() == AstType.INDEX:
            self.__CompileIndexExpr(expr)
        elif expr.Type() == AstType.PREFIX:
            self.__CompilePrefixExpr(expr)
        elif expr.Type() == AstType.INFIX:
            self.__CompileInfixExpr(expr)
        elif expr.Type() == AstType.FUNCTION_CALL:
            self.__CompileFunctionCallExpr(expr)
        elif expr.Type() == AstType.STRUCT_CALL:
            self.__CompileStructCallExpr(expr, state)
        elif expr.Type() == AstType.REF:
            self.__CompileRefExpr(expr)
        elif expr.Type() == AstType.LAMBDA:
            self.__CompileLambdaExpr(expr)

    def __CompileInfixExpr(self, expr: InfixExpr) -> None:
        if expr.op == "=":
            self.__CompileExpr(expr.right)
            self.__CompileExpr(expr.left,RWState.WRITE)
        else:
            self.__CompileExpr(expr.right)
            self.__CompileExpr(expr.left)

            if expr.op == "+":
                self.__Emit(OpCode.OP_ADD)
            elif expr.op == "-":
                self.__Emit(OpCode.OP_SUB)
            elif expr.op == "*":
                self.__Emit(OpCode.OP_MUL)
            elif expr.op == "/":
                self.__Emit(OpCode.OP_DIV)
            elif expr.op == ">":
                self.__Emit(OpCode.OP_GREATER)
            elif expr.op == "<":
                self.__Emit(OpCode.OP_LESS)
            elif expr.op == ">=":
                self.__Emit(OpCode.OP_LESS)
                self.__Emit(OpCode.OP_NOT)
            elif expr.op == "<=":
                self.__Emit(OpCode.OP_GREATER)
                self.__Emit(OpCode.OP_NOT)
            elif expr.op == "==":
                self.__Emit(OpCode.OP_EQUAL)
            elif expr.op == "!=":
                self.__Emit(OpCode.OP_EQUAL)
                self.__Emit(OpCode.OP_NOT)
            elif expr.op == "and":
                self.__Emit(OpCode.OP_AND)
            elif expr.op == "or":
                self.__Emit(OpCode.OP_OR)

    def __CompileNumExpr(self, expr: NumExpr) -> None:
        value = NumObject(expr.value)
        pos = self.__AddConstant(value)
        self.__EmitConstant(pos)

    def __CompileBoolExpr(self, expr: BoolExpr) -> None:
        if expr.value:
            self.__EmitConstant(self.__AddConstant(BoolObject(True)))
        else:
            self.__EmitConstant(self.__AddConstant(BoolObject(False)))

    def __CompilePrefixExpr(self, expr: PrefixExpr) -> None:
        self.__CompileExpr(expr.right)
        if expr.op == "-":
            self.__Emit(OpCode.OP_MINUS)
        elif expr.op == "not":
            self.__Emit(OpCode.OP_NOT)
        else:
            Assert("Unrecognized prefix op")

    def __CompileStrExpr(self, expr: StrExpr) -> None:
        obj = StrObject(expr.value)
        pos = self.__AddConstant(obj)
        self.__EmitConstant(pos)

    def __CompileNilExpr(self, expr: NilExpr) -> None:
        self.__EmitConstant(self.__AddConstant(NilObject()))

    def __CompileGroupExpr(self, expr: GroupExpr) -> None:
        self.__CompileExpr(expr.expr)

    def __CompileArrayExpr(self, expr: ArrayExpr) -> None:
        for e in expr.elements:
            self.__CompileExpr(e)
        
        self.__Emit(OpCode.OP_ARRAY)
        self.__Emit(len(expr.elements))

    def __CompileIndexExpr(self, expr: IndexExpr) -> None:
        self.__CompileExpr(expr.ds)
        self.__CompileExpr(expr.index)
        self.__Emit(OpCode.OP_INDEX)

    def __CompileIdentifierExpr(self, expr: IdentifierExpr, state: RWState) -> None:
        isFound,symbol= self.__symbolTable.Resolve(expr.literal)
        if isFound == False:
            Assert("Undefined variable:"+expr.Stringify())

        if state == RWState.READ:
            self.__LoadSymbol(symbol)
        else:
            if symbol.scope == SymbolScope.GLOBAL:
                self.__Emit(OpCode.OP_SET_GLOBAL)
                self.__Emit(symbol.index)
            elif symbol.scope == SymbolScope.LOCAL:
                self.__Emit(OpCode.OP_SET_LOCAL)
                self.__Emit(symbol.isInUpScope)
                self.__Emit(symbol.scopeDepth)
                self.__Emit(symbol.index)

    def __CompileLambdaExpr(self, expr: LambdaExpr) -> None:
        self.__EnterScope()

        self.__symbolTable.DefineFunction(expr.name)

        for param in expr.parameters:
            self.__symbolTable.Define(param.literal)

        for s in expr.body.stmts:
            self.__CompileStmt(s)

        localVarCount = self.__symbolTable.definitionCount
        opCodes = self.__ExitScope()

        opCodesLen = len(opCodes)
        if opCodesLen == 0 or opCodes[opCodesLen-2] != OpCode.OP_RETURN:
            opCodes.append(OpCode.OP_RETURN)
            opCodes.append(0)

        fn = FunctionObject(opCodes, localVarCount, len(expr.parameters))

        self.__EmitConstant(self.__AddConstant(fn))

    def __CompileFunctionCallExpr(self, expr: FunctionCallExpr) -> None:
        self.__CompileExpr(expr.name)

        for argu in expr.arguments:
            self.__CompileExpr(argu)

        self.__Emit(OpCode.OP_FUNCTION_CALL)
        self.__Emit(len(expr.arguments))

    def __CompileStructCallExpr(self, expr: StructCallExpr, state: RWState) -> None:
        if expr.callMember.Type() == AstType.FUNCTION_CALL and state == RWState.WRITE:
            Assert("Cannot assign to a struct's function call expr")

        self.__CompileExpr(expr.callee)

        if expr.callMember.Type() == AstType.IDENTIFIER:
            self.__EmitConstant(self.__AddConstant(
                StrObject(expr.callMember.literal)))
        elif expr.callMember.Type() == AstType.FUNCTION_CALL:
            self.__EmitConstant(self.__AddConstant(
                StrObject(expr.callMember.name.literal)))

        if state == RWState.READ:
            self.__Emit(OpCode.OP_GET_STRUCT)

            if expr.callMember.Type() == AstType.FUNCTION_CALL:
                funcCall = expr.callMember
                for argu in funcCall.arguments:
                    self.__CompileExpr(argu)

                self.__Emit(OpCode.OP_FUNCTION_CALL)
                self.__Emit(len(funcCall.arguments))
        else:
            self.__Emit(OpCode.OP_SET_STRUCT)

    def __CompileRefExpr(self, expr: RefExpr) -> None:
        isFound,symbol = self.__symbolTable.Resolve(expr.refExpr.literal)
        if isFound == False:
            Assert("Undefined variable:"+expr.Stringify())

        if symbol.scope == SymbolScope.GLOBAL:
            self.__Emit(OpCode.OP_REF_GLOBAL)
            self.__Emit(symbol.index)
        elif symbol.scope == SymbolScope.LOCAL:
            self.__Emit(OpCode.OP_REF_LOCAL)
            self.__Emit(symbol.isInUpScope)
            self.__Emit(symbol.scopeDepth)
            self.__Emit(symbol.index)
            

    def __EnterScope(self) -> None:
        self.__symbolTable = SymbolTable(self.__symbolTable)
        self.__scopes.append([])
        self.__scopeIndex += 1

    def __ExitScope(self) -> List[int]:
        opCodes = self.__CurOpCodes()
        self.__scopes.pop()
        self.__scopeIndex -= 1
        self.__symbolTable = self.__symbolTable.enclosing
        return opCodes

    def __CurOpCodes(self) -> List[int]:
        return self.__scopes[self.__scopeIndex]

    def __Emit(self, opcode: int) -> int:
        self.__CurOpCodes().append(opcode)
        return len(self.__CurOpCodes())-1

    def __EmitConstant(self, pos: int) -> int:
        self.__Emit(OpCode.OP_CONSTANT)
        self.__Emit(pos)
        return len(self.__CurOpCodes())-1

    def __ModifyOpCode(self, pos: int, opcode: int) -> None:
        self.__CurOpCodes()[pos] = opcode

    def __AddConstant(self, object: Object) -> int:
        self.__constants[self.__constantCount] = object
        self.__constantCount += 1
        return self.__constantCount-1

    def __LoadSymbol(self, symbol: Symbol) -> None:
        if symbol.scope == SymbolScope.GLOBAL:
            self.__Emit(OpCode.OP_GET_GLOBAL)
            self.__Emit(symbol.index)
        elif symbol.scope == SymbolScope.LOCAL:
            self.__Emit(OpCode.OP_GET_LOCAL)
            self.__Emit(symbol.isInUpScope)
            self.__Emit(symbol.scopeDepth)
            self.__Emit(symbol.index)
        elif symbol.scope == SymbolScope.BUILTIN:
            self.__Emit(OpCode.OP_GET_BUILTIN)
            self.__Emit(symbol.index)
        elif symbol.scope == SymbolScope.FUNCTION:
            self.__Emit(OpCode.OP_GET_CURRENT_FUNCTION)

        if symbol.isStructSymbol:
            self.__Emit(OpCode.OP_FUNCTION_CALL)
            self.__Emit(0)
