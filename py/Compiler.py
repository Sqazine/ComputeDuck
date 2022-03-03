from enum import IntEnum
from Utils import Assert
from Frame import Frame
from Ast import Stmt
from Ast import ArrayExpr, AstType, BoolExpr, Expr, ExprStmt, FunctionCallExpr, FunctionStmt, GroupExpr, IdentifierExpr, IfStmt, IndexExpr, InfixExpr, NilExpr, NumExpr, PrefixExpr, ReturnStmt, ScopeStmt, StrExpr, StructCallExpr, StructStmt, VarStmt, WhileStmt, RefExpr
from Frame import OpCode


class ObjectState(IntEnum):
    INIT = 0,
    READ = 1,
    WRITE = 2,
    STRUCT_READ = 3,
    STRUCT_WRITE = 4


class Compiler:
    __rootFrame: Frame

    def __init__(self) -> None:
        self.__rootFrame = Frame()

    def Compile(self, stmts: list[Stmt]) -> Frame:
        for s in stmts:
            self.CompileStmt(s, self.__rootFrame)
        return self.__rootFrame

    def ResetStatus(self) -> None:
        self.__rootFrame.Clear()

    def CompileStmt(self, stmt: Stmt, frame: Frame):
        if stmt.Type() == AstType.RETURN:
            self.CompileReturnStmt(stmt, frame)
        elif stmt.Type() == AstType.EXPR:
            self.CompileExprStmt(stmt, frame)
        elif stmt.Type() == AstType.VAR:
            self.CompileVarStmt(stmt, frame)
        elif stmt.Type() == AstType.SCOPE:
            self.CompileScopeStmt(stmt, frame)
        elif stmt.Type() == AstType.IF:
            self.CompileIfStmt(stmt, frame)
        elif stmt.Type() == AstType.WHILE:
            self.CompileWhileStmt(stmt, frame)
        elif stmt.Type() == AstType.FUNCTION:
            self.CompileFunctionStmt(stmt, frame)
        elif stmt.Type() == AstType.STRUCT:
            self.CompileStructStmt(stmt, frame)

    def CompileReturnStmt(self, stmt: ReturnStmt, frame: Frame):
        if stmt.expr != None:
            self.CompileExpr(stmt.expr, frame)
        frame.AddOpCode(OpCode.OP_RETURN)

    def CompileExprStmt(self, stmt: ExprStmt, frame: Frame):
        self.CompileExpr(stmt.expr, frame)

    def CompileVarStmt(self, stmt: VarStmt, frame: Frame):
        self.CompileExpr(stmt.value, frame)
        self.CompileExpr(stmt.name, frame, ObjectState.INIT)

    def CompileScopeStmt(self, stmt: ScopeStmt, frame: Frame):
        frame.AddOpCode(OpCode.OP_ENTER_SCOPE)
        for s in stmt.stmts:
            self.CompileStmt(s, frame)
        frame.AddOpCode(OpCode.OP_EXIT_SCOPE)

    def CompileIfStmt(self, stmt: IfStmt, frame: Frame):
        self.CompileExpr(stmt.condition, frame)
        frame.AddOpCode(OpCode.OP_JUMP_IF_FALSE)
        jmpIfFalseOffset = frame.AddNum(0)
        frame.AddOpCode(jmpIfFalseOffset)
        self.CompileStmt(stmt.thenBranch, frame)
        frame.AddOpCode(OpCode.OP_JUMP)
        jmpOffset = frame.AddNum(0)
        frame.AddOpCode(jmpOffset)

        frame.nums[jmpIfFalseOffset] = (float)(frame.GetOpCodeSize())-1.0

        if stmt.elseBranch:
            self.CompileStmt(stmt.elseBranch, frame)

        frame.nums[jmpOffset] = (float)(frame.GetOpCodeSize())-1.0

    def CompileWhileStmt(self, stmt: WhileStmt, frame: Frame):
        jmpAddress = frame.GetOpCodeSize()-1
        self.CompileExpr(stmt.condition, frame)

        frame.AddOpCode(OpCode.OP_JUMP_IF_FALSE)
        jmpIfFalseOffset = frame.AddNum(0)
        frame.AddOpCode(jmpIfFalseOffset)

        self.CompileStmt(stmt.body, frame)

        frame.AddOpCode(OpCode.OP_JUMP)
        offset = frame.AddNum(jmpAddress)
        frame.AddOpCode(offset)

        frame.nums[jmpIfFalseOffset] = (float)(frame.GetOpCodeSize())-1.0

    def CompileFunctionStmt(self, stmt: FunctionStmt, frame: Frame):
        functionFrame = Frame(frame)
        functionFrame.AddOpCode(OpCode.OP_ENTER_SCOPE)

        for i in range(len(stmt.parameters)-1, -1, -1):
            self.CompileIdentifierExpr(
                stmt.parameters[i], functionFrame, ObjectState.INIT)

        for s in stmt.body.stmts:
            self.CompileStmt(s, functionFrame)

        functionFrame.AddOpCode(OpCode.OP_EXIT_SCOPE)

        frame.AddFunctionFrame(stmt.name, functionFrame)

    def CompileStructStmt(self, stmt: StructStmt, frame: Frame):
        structFrame = Frame(frame)
        structFrame.AddOpCode(OpCode.OP_ENTER_SCOPE)

        for m in stmt.members:
            self.CompileVarStmt(m, structFrame)

        structFrame.AddOpCode(OpCode.OP_NEW_STRUCT)
        offset = structFrame.AddString(stmt.name)
        structFrame.AddOpCode(offset)

        structFrame.AddOpCode(OpCode.OP_RETURN)
        frame.AddStructFrame(stmt.name, structFrame)

    def CompileExpr(self, expr: Expr, frame: Frame, state: ObjectState = ObjectState.READ):
        if expr.Type() == AstType.NUM:
            self.CompileNumExpr(expr, frame)
        elif expr.Type() == AstType.STR:
            self.CompileStrExpr(expr, frame)
        elif expr.Type() == AstType.BOOL:
            self.CompileBoolExpr(expr, frame)
        elif expr.Type() == AstType.NIL:
            self.CompileNilExpr(expr, frame)
        elif expr.Type() == AstType.IDENTIFIER:
            self.CompileIdentifierExpr(expr, frame, state)
        elif expr.Type() == AstType.GROUP:
            self.CompileGroupExpr(expr, frame)
        elif expr.Type() == AstType.ARRAY:
            self.CompileArrayExpr(expr, frame)
        elif expr.Type() == AstType.INDEX:
            self.CompileIndexExpr(expr, frame, state)
        elif expr.Type() == AstType.PREFIX:
            self.CompilePrefixExpr(expr, frame)
        elif expr.Type() == AstType.INFIX:
            self.CompileInfixExpr(expr, frame)
        elif expr.Type() == AstType.FUNCTION_CALL:
            self.CompileFunctionCallExpr(expr, frame)
        elif expr.Type() == AstType.STRUCT_CALL:
            self.CompileStructCallExpr(expr, frame, state)
        elif expr.Type() == AstType.REF:
            self.CompileRefExpr(expr, frame)

    def CompileNumExpr(self, expr: NumExpr, frame: Frame):
        frame.AddOpCode(OpCode.OP_NEW_NUM)
        offset = frame.AddNum(expr.value)
        frame.AddOpCode(offset)

    def CompileStrExpr(self, expr: StrExpr, frame: Frame):
        frame.AddOpCode(OpCode.OP_NEW_STR)
        offset = frame.AddString(expr.value)
        frame.AddOpCode(offset)

    def CompileBoolExpr(self, expr: BoolExpr, frame: Frame):
        if expr.value:
            frame.AddOpCode(OpCode.OP_NEW_TRUE)
        else:
            frame.AddOpCode(OpCode.OP_NEW_FALSE)

    def CompileNilExpr(self, expr: NilExpr, frame: Frame):
        frame.AddOpCode(OpCode.OP_NEW_NIL)

    def CompileIdentifierExpr(self, expr: IdentifierExpr, frame: Frame, state: ObjectState = ObjectState.READ):
        if state == ObjectState.READ:
            frame.AddOpCode(OpCode.OP_GET_VAR)
        elif state == ObjectState.WRITE:
            frame.AddOpCode(OpCode.OP_SET_VAR)
        elif state == ObjectState.INIT:
            frame.AddOpCode(OpCode.OP_DEFINE_VAR)
        elif state == ObjectState.STRUCT_READ:
            frame.AddOpCode(OpCode.OP_GET_STRUCT_VAR)
        elif state == ObjectState.STRUCT_WRITE:
            frame.AddOpCode(OpCode.OP_SET_STRUCT_VAR)

        offset = frame.AddString(expr.literal)
        frame.AddOpCode(offset)

    def CompileGroupExpr(self, expr: GroupExpr, frame: Frame):
        self.CompileExpr(expr.expr, frame)

    def CompileArrayExpr(self, expr: ArrayExpr, frame: Frame):
        for e in expr.elements:
            self.CompileExpr(e, frame)
        frame.AddOpCode(OpCode.OP_NEW_ARRAY)
        offset = frame.AddNum(len(expr.elements))
        frame.AddOpCode(offset)

    def CompileIndexExpr(self, expr: IndexExpr, frame: Frame, state: ObjectState = ObjectState.READ):
        self.CompileExpr(expr.ds, frame)
        self.CompileExpr(expr.index, frame)

        if state == ObjectState.READ:
            frame.AddOpCode(OpCode.OP_GET_INDEX_VAR)
        elif state == ObjectState.WRITE:
            frame.AddOpCode(OpCode.OP_SET_INDEX_VAR)

    def CompilePrefixExpr(self, expr: PrefixExpr, frame: Frame):
        self.CompileExpr(expr.right, frame)
        if expr.op == "-":
            frame.AddOpCode(OpCode.OP_NEG)
        elif expr.op == "not":
            frame.AddOpCode(OpCode.OP_NOT)

    def CompileInfixExpr(self, expr: InfixExpr, frame: Frame):
        if expr.op == "=":
            self.CompileExpr(expr.right, frame)
            self.CompileExpr(expr.left, frame, ObjectState.WRITE)
        else:
            self.CompileExpr(expr.right, frame)
            self.CompileExpr(expr.left, frame)

            if expr.op == "+":
                frame.AddOpCode(OpCode.OP_ADD)
            elif expr.op == "-":
                frame.AddOpCode(OpCode.OP_SUB)
            elif expr.op == "*":
                frame.AddOpCode(OpCode.OP_MUL)
            elif expr.op == "/":
                frame.AddOpCode(OpCode.OP_DIV)
            elif expr.op == "and":
                frame.AddOpCode(OpCode.OP_AND)
            elif expr.op == "or":
                frame.AddOpCode(OpCode.OP_OR)
            elif expr.op == ">":
                frame.AddOpCode(OpCode.OP_GREATER)
            elif expr.op == "<":
                frame.AddOpCode(OpCode.OP_LESS)
            elif expr.op == ">=":
                frame.AddOpCode(OpCode.OP_GREATER_EQUAL)
            elif expr.op == "<=":
                frame.AddOpCode(OpCode.OP_LESS_EQUAL)
            elif expr.op == "==":
                frame.AddOpCode(OpCode.OP_EQUAL)
            elif expr.op == "!=":
                frame.AddOpCode(OpCode.OP_NOT_EQUAL)
            else:
                Assert("Unknown binary op:"+expr.op)

    def CompileRefExpr(self, expr: RefExpr, frame: Frame):
        frame.AddOpCode(OpCode.OP_REF)
        offset=frame.AddString(expr.refExpr.literal)
        frame.AddOpCode(offset)

    def CompileFunctionCallExpr(self, expr: FunctionCallExpr, frame: Frame):
        for arg in expr.arguments:
            self.CompileExpr(arg, frame)
        #argument count
        frame.AddOpCode(OpCode.OP_NEW_NUM)
        offset = frame.AddNum(len(expr.arguments))
        frame.AddOpCode(offset)

        frame.AddOpCode(OpCode.OP_FUNCTION_CALL)
        offset = frame.AddString(expr.name)
        frame.AddOpCode(offset)

    def CompileStructCallExpr(self, expr: StructCallExpr, frame: Frame, state: ObjectState = ObjectState.READ):
        self.CompileExpr(expr.callee, frame)

        if expr.callMember.Type() == AstType.STRUCT_CALL:
            self.CompileExpr(
                ((StructCallExpr)(expr.callMember).callee), frame, ObjectState.STRUCT_READ)

        if state == ObjectState.READ:
            self.CompileExpr(expr.callMember, frame, ObjectState.STRUCT_READ)
        elif state == ObjectState.WRITE:
            self.CompileExpr(expr.callMember, frame, ObjectState.STRUCT_WRITE)
