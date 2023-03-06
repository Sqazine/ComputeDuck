from enum import IntEnum
from Utils import *
from Ast import *
from SymbolTable import *
from Object import *
from Chunk import *
from BuiltinManager import *

class RWState(IntEnum):
    READ = 0,
    WRITE = 1,


class Compiler:
    __constants: list[Object]=[]
    __scopes: list[list[int]]=[]
    __symbolTable: SymbolTable
    __builtinFunctionNames:list[str]=[]
    __builtinVariableNames:list[str]=[]

    def __init__(self) -> None:
        self.__symbolTable = None
        self.ResetStatus()

    def Compile(self, stmts: list[Stmt]) -> Chunk:
        for stmt in stmts:
            self.__CompileStmt(stmt)

        self.__Emit(OpCode.OP_RETURN)
        self.__Emit(0)

        return Chunk(self.__CurOpCodes(), self.__constants)

    def ResetStatus(self) -> None:
        self.__scopes = []
        self.__scopes.append([])

        self.__symbolTable = SymbolTable()

        for i in range(0, len(gBuiltinManager.builtinFunctionNames)):
            self.__builtinFunctionNames.append(gBuiltinManager.builtinFunctionNames[i])
            self.__symbolTable.DefineBuiltinFunction(gBuiltinManager.builtinFunctionNames[i], i)

        for i in range(0,len(gBuiltinManager.builtinVariableNames)):
            self.__builtinVariableNames.append(gBuiltinManager.builtinVariableNames[i])
            self.__symbolTable.DefineBuiltinVariable(gBuiltinManager.builtinVariableNames[i], i)

    def __CompileStmt(self, stmt: Stmt) -> None:
        if stmt.type == AstType.RETURN:
            self.__CompileReturnStmt(stmt)
        elif stmt.type == AstType.EXPR:
            self.__CompileExprStmt(stmt)
        elif stmt.type == AstType.SCOPE:
            self.__CompileScopeStmt(stmt)
        elif stmt.type == AstType.IF:
            self.__CompileIfStmt(stmt)
        elif stmt.type == AstType.WHILE:
            self.__CompileWhileStmt(stmt)
        elif stmt.type == AstType.STRUCT:
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

        self.__ExitScope()

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


    def __CompileStructStmt(self, stmt: StructStmt) -> None:
        symbol = self.__symbolTable.Define(stmt.name, True)

        self.__EnterScope()
        self.__scopes.append([])

        for k,v in stmt.members.items():
            self.__CompileExpr(v)
            self.__EmitConstant(self.__AddConstant(StrObject(k.literal)))

        localVarCount = self.__symbolTable.definitionCount

        self.__Emit(OpCode.OP_STRUCT)
        self.__Emit(len(stmt.members))

        self.__ExitScope()

        opCodes=self.__scopes.pop()

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
        if expr.type == AstType.NUM:
            self.__CompileNumExpr(expr)
        elif expr.type == AstType.STR:
            self.__CompileStrExpr(expr)
        elif expr.type == AstType.BOOL:
            self.__CompileBoolExpr(expr)
        elif expr.type == AstType.NIL:
            self.__CompileNilExpr(expr)
        elif expr.type == AstType.IDENTIFIER:
            self.__CompileIdentifierExpr(expr,state)
        elif expr.type == AstType.GROUP:
            self.__CompileGroupExpr(expr)
        elif expr.type == AstType.ARRAY:
            self.__CompileArrayExpr(expr)
        elif expr.type == AstType.INDEX:
            self.__CompileIndexExpr(expr)
        elif expr.type == AstType.PREFIX:
            self.__CompilePrefixExpr(expr)
        elif expr.type == AstType.INFIX:
            self.__CompileInfixExpr(expr)
        elif expr.type == AstType.FUNCTION_CALL:
            self.__CompileFunctionCallExpr(expr)
        elif expr.type == AstType.STRUCT_CALL:
            self.__CompileStructCallExpr(expr, state)
        elif expr.type == AstType.REF:
            self.__CompileRefExpr(expr)
        elif expr.type==AstType.FUNCTION:
            self.__CompileFunctionExpr(expr)
        elif expr.type==AstType.ANONY_STRUCT:
            self.__CompileAnonyStructExpr(expr)
        elif expr.type==AstType.DLL_IMPORT:
            self.CompileDllImportExpr(expr)

    def __CompileInfixExpr(self, expr: InfixExpr) -> None:
        if expr.op == "=":
            if expr.left.type==AstType.IDENTIFIER and expr.right.type==AstType.FUNCTION:
                self.__symbolTable.Define(expr.left.literal)
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

        if state == RWState.READ:
            if isFound == False:
                Assert("Undefined variable:"+expr.__str__())
            self.__LoadSymbol(symbol)
        else:
            if isFound==False:
                symbol=self.__symbolTable.Define(expr.literal)

            if symbol.scope == SymbolScope.GLOBAL:
                self.__Emit(OpCode.OP_SET_GLOBAL)
                self.__Emit(symbol.index)
            elif symbol.scope == SymbolScope.LOCAL:
                self.__Emit(OpCode.OP_SET_LOCAL)
                self.__Emit(symbol.isInUpScope)
                self.__Emit(symbol.scopeDepth)
                self.__Emit(symbol.index)

    def __CompileFunctionExpr(self, stmt: FunctionExpr) -> None:
        self.__EnterScope()

        self.__scopes.append([])

        for param in stmt.parameters:
            self.__symbolTable.Define(param.literal)

        for s in stmt.body.stmts:
            self.__CompileStmt(s)

        localVarCount = self.__symbolTable.definitionCount
        
        self.__ExitScope()

        opCodes=self.__scopes.pop()

        #for non return  or empty stmt in function scope:add a return to return nothing
        opCodesLen = len(opCodes)
        if opCodesLen == 0 or opCodes[opCodesLen-2] != OpCode.OP_RETURN:
            opCodes.append(OpCode.OP_RETURN)
            opCodes.append(0)

        fn = FunctionObject(opCodes, localVarCount, len(stmt.parameters))

        self.__EmitConstant(self.__AddConstant(fn))


    def __CompileFunctionCallExpr(self, expr: FunctionCallExpr) -> None:
        self.__CompileExpr(expr.name)

        for argu in expr.arguments:
            self.__CompileExpr(argu)

        self.__Emit(OpCode.OP_FUNCTION_CALL)
        self.__Emit(len(expr.arguments))

    def __CompileStructCallExpr(self, expr: StructCallExpr, state: RWState) -> None:
        if expr.callMember.type == AstType.FUNCTION_CALL and state == RWState.WRITE:
            Assert("Cannot assign to a struct's function call expr")

        self.__CompileExpr(expr.callee)

        if expr.callMember.type == AstType.IDENTIFIER:
            self.__EmitConstant(self.__AddConstant(
                StrObject(expr.callMember.literal)))
        elif expr.callMember.type == AstType.FUNCTION_CALL:
            self.__EmitConstant(self.__AddConstant(
                StrObject(expr.callMember.name.literal)))

        if state == RWState.READ:
            self.__Emit(OpCode.OP_GET_STRUCT)

            if expr.callMember.type == AstType.FUNCTION_CALL:
                funcCall = expr.callMember
                for argu in funcCall.arguments:
                    self.__CompileExpr(argu)

                self.__Emit(OpCode.OP_FUNCTION_CALL)
                self.__Emit(len(funcCall.arguments))
        else:
            self.__Emit(OpCode.OP_SET_STRUCT)

    def __CompileRefExpr(self, expr: RefExpr) -> None:

        if expr.refExpr.type==AstType.INDEX:
            self.__CompileExpr(expr.refExpr.index)
            isFound,symbol = self.__symbolTable.Resolve(expr.refExpr.ds.__str__())
            if isFound == False:
                Assert("Undefined variable:"+expr.__str__())

            if symbol.scope == SymbolScope.GLOBAL:
                self.__Emit(OpCode.OP_REF_INDEX_GLOBAL)
                self.__Emit(symbol.index)
            elif symbol.scope == SymbolScope.LOCAL:
                self.__Emit(OpCode.OP_REF_INDEX_LOCAL)
                self.__Emit(symbol.isInUpScope)
                self.__Emit(symbol.scopeDepth)
                self.__Emit(symbol.index)
        else:
            isFound,symbol = self.__symbolTable.Resolve(expr.refExpr.__str__())
            if isFound==False:
                Assert("Undefined variable:"+expr.__str__())

            if symbol.scope == SymbolScope.GLOBAL:
                self.__Emit(OpCode.OP_REF_GLOBAL)
                self.__Emit(symbol.index)
            elif symbol.scope == SymbolScope.LOCAL:
                self.__Emit(OpCode.OP_REF_LOCAL)
                self.__Emit(symbol.isInUpScope)
                self.__Emit(symbol.scopeDepth)
                self.__Emit(symbol.index)

    def __CompileAnonyStructExpr(self,expr:AnonyStructExpr)->None:
        self.__EnterScope()

        for k,v in expr.memberPairs.items():
            self.__CompileExpr(v)
            self.__EmitConstant(self.__AddConstant(StrObject(k.literal)))

        self.__Emit(OpCode.OP_STRUCT)
        self.__Emit(len(expr.memberPairs))

        self.__ExitScope()

    def __CompileDllImportExpr(self,expr:DllImportExpr)->None:
        pass

    def __EnterScope(self) -> None:
        self.__symbolTable = SymbolTable(self.__symbolTable)

    def __ExitScope(self) -> list[int]:
        self.__symbolTable = self.__symbolTable.enclosing

    def __CurOpCodes(self) -> list[int]:
        return self.__scopes[-1]

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
        self.__constants.append(object)
        return len(self.__constants)-1

    def __LoadSymbol(self, symbol: Symbol) -> None:
        if symbol.scope == SymbolScope.GLOBAL:
            self.__Emit(OpCode.OP_GET_GLOBAL)
            self.__Emit(symbol.index)
        elif symbol.scope == SymbolScope.LOCAL:
            self.__Emit(OpCode.OP_GET_LOCAL)
            self.__Emit(symbol.isInUpScope)
            self.__Emit(symbol.scopeDepth)
            self.__Emit(symbol.index)
        elif symbol.scope == SymbolScope.BUILTIN_FUNCTION:
            self.__Emit(OpCode.OP_GET_BUILTIN_FUNCTION)
            self.__Emit(symbol.index)
        elif symbol.scope == SymbolScope.BUILTIN_VARIABLE:
            self.__Emit(OpCode.OP_GET_BUILTIN_VARIABLE)

        if symbol.isStructSymbol:
            self.__Emit(OpCode.OP_FUNCTION_CALL)
            self.__Emit(0)
