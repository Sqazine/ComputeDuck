from enum import IntEnum
from Utils import Assert
from Frame import Frame
from Ast import Stmt
from py.Ast import ArrayExpr, AstType, BoolExpr, Expr, ExprStmt, FunctionCallExpr, FunctionStmt, GroupExpr, IdentifierExpr, IfStmt, IndexExpr, InfixExpr, NilExpr, NumExpr, PrefixExpr, ReturnStmt, ScopeStmt, StrExpr, StructCallExpr, StructStmt, VarStmt, WhileStmt
from py.Frame import OpCode


class ObjectState(IntEnum):
    INIT = 0,
    READ = 1,
    WRITE = 2,
    STRUCT_READ = 3,
    STRUCT_WRITE = 4


class Compiler:
    __rootFrame: Frame

    def __init__(self) -> None:
        pass

    def Compile(self, stmts: list[Stmt]) -> Frame:
        for s in stmts:
            self.CompileStmt(s,self.__rootFrame)
        return self.__rootFrame

    def ResetStatus(self) -> None:
        self.__rootFrame.Clear()

    def CompileStmt(self, stmt: Stmt, frame: Frame):
        if stmt.Type()==AstType.RETURN:
            self.CompileReturnStmt(((ReturnStmt)(stmt)),frame)
        elif stmt.Type()==AstType.EXPR:
            self.CompileExprStmt((ExprStmt)(stmt),frame)
        elif stmt.Type()==AstType.VAR:
            self.CompileVarStmt((VarStmt)(stmt),frame)
        elif stmt.Type()==AstType.SCOPE:
            self.CompileScopeStmt((ScopeStmt)(stmt),frame)
        elif stmt.Type()==AstType.IF:
            self.CompileIfStmt((IfStmt)(stmt),frame)
        elif stmt.Type()==AstType.WHILE:
            self.CompileWhileStmt((WhileStmt)(stmt),frame)
        elif stmt.Type()==AstType.FUNCTION:
            self.CompileFunctionStmt((WhileStmt)(stmt),frame)
        elif stmt.Type()==AstType.STRUCT:
            self.CompileStructStmt((StructStmt)(stmt),frame)

    def CompileReturnStmt(self, stmt: ReturnStmt, frame: Frame):
        if stmt.expr!=None:
             self.CompileExpr(stmt.expr,frame)
        frame.AddOpCode(OpCode.OP_RETURN)

    def CompileExprStmt(self, stmt: ExprStmt, frame: Frame):
        self.CompileExpr(stmt.expr,frame)

    def CompileVarStmt(self, stmt: VarStmt, frame: Frame):
        self.CompileExpr(stmt.value,frame)
        self.CompileExpr(stmt.name,frame,ObjectState.INIT)

    def CompileScopeStmt(self, stmt: ScopeStmt, frame: Frame):
        frame.AddOpCode(OpCode.OP_ENTER_SCOPE)
        for s in stmt.stmts:
            self.CompileStmt(s,frame)
        frame.AddOpCode(OpCode.OP_EXIT_SCOPE)

    def CompileIfStmt(self, stmt: IfStmt, frame: Frame):
        self.CompileExpr(stmt.condition,frame)
        frame.AddOpCode(OpCode.OP_JUMP_IF_FALSE)
        jmpIfFalseOffset=frame.AddNum(0)
        frame.AddOpCode(jmpIfFalseOffset)
        self.CompileStmt(stmt.thenBranch,frame)
        frame.AddOpCode(OpCode.OP_JUMP)
        jmpOffset=frame.AddNum(0)
        frame.AddOpCode(jmpOffset)
        
        frame.nums[jmpIfFalseOffset]=(float)(frame.GetOpCodeSize())-1.0
        
        if stmt.elseBranch:
            self.CompileStmt(stmt.elseBranch,frame)
            
        frame.nums[jmpOffset]=(float)(frame.GetOpCodeSize())-1.0

    def CompileWhileStmt(self, stmt: WhileStmt, frame: Frame):
        jmpAddress=frame.GetOpCodeSize()-1
        self.CompileExpr(stmt.condition,frame)
        
        frame.AddOpCode(OpCode.OP_JUMP_IF_FALSE)
        jmpIfFalseOffset=frame.AddNum(0)
        frame.AddOpCode(jmpIfFalseOffset)
        
        self.CompileStmt(stmt.body,frame)
        
        frame.AddOpCode(OpCode.OP_JUMP)
        offset=frame.AddNum(jmpAddress)
        frame.AddOpCode(offset)
        
        frame.nums[jmpIfFalseOffset]=(float)(frame.GetOpCodeSize())-1.0

    def CompileFunctionStmt(self, stmt: FunctionStmt, frame: Frame):
        functionFrame=Frame(frame)
        functionFrame.AddOpCode(OpCode.OP_ENTER_SCOPE)
        
        for i in range(len(stmt.parameters),-1,-1):
            self.CompileIdentifierExpr(stmt.parameters[i],functionFrame,ObjectState.INIT)
            
        self.CompileScopeStmt(stmt.body,functionFrame)
        
        functionFrame.AddOpCode(OpCode.OP_EXIT_SCOPE)
        
        frame.AddFunctionFrame(stmt.name,functionFrame)

    def CompileStructStmt(self, stmt: StructStmt, frame: Frame):
        structFrame=Frame(frame)
        structFrame.AddOpCode(OpCode.OP_ENTER_SCOPE)
        
        for m in stmt.members:
            self.CompileVarStmt(m,structFrame)
            
        structFrame.AddOpCode(OpCode.OP_NEW_STRUCT)
        offset=structFrame.AddString(stmt.name)
        structFrame.AddOpCode(offset)
        
        structFrame.AddOpCode(OpCode.OP_RETURN)
        frame.AddStructFrame(stmt.name,structFrame)

    def CompileExpr(self, expr: Expr, frame: Frame, state: ObjectState = ObjectState.READ):
        if expr.Type()==AstType.NUM:
            self.CompileNumExpr((NumExpr)(expr),frame)
        elif expr.Type()==AstType.STR:
            self.CompileStrExpr((NumExpr)(expr),frame)
        elif expr.Type()==AstType.BOOL:
            self.CompileBoolExpr((BoolExpr)(expr),frame)
        elif expr.Type()==AstType.NIL:
            self.CompileNilExpr((NilExpr)(expr),frame)
        elif expr.Type()==AstType.IDENTIFIER:
            self.CompileIdentifierExpr((IdentifierExpr)(expr),frame)
        elif expr.Type()==AstType.GROUP:
            self.CompileGroupExpr((GroupExpr)(expr),frame)
        elif expr.Type()==AstType.ARRAY:
            self.CompileArrayExpr((ArrayExpr)(expr),frame)
        elif expr.Type()==AstType.INDEX:
            self.CompileIndexExpr((IndexExpr)(expr),frame)
        elif expr.Type()==AstType.PREFIX:
            self.CompilePrefixExpr((PrefixExpr)(expr),frame)
        elif expr.Type()==AstType.INFIX:
            self.CompileInfixExpr((InfixExpr)(expr),frame)
        elif expr.Type()==AstType.FUNCTION_CALL:
            self.CompileFunctionCallExpr((FunctionCallExpr)(expr),frame)
        elif expr.Type()==AstType.STRUCT_CALL:
            self.CompileStructCallExpr((StructCallExpr)(expr),frame)

    def CompileNumExpr(self, expr: NumExpr, frame: Frame):
        frame.AddOpCode(OpCode.OP_NEW_NUM)
        offset=frame.AddNum(expr.value)
        frame.AddOpCode(offset)

    def CompileStrExpr(self, expr: StrExpr, frame: Frame):
        frame.AddOpCode(OpCode.OP_NEW_STR)
        offset=frame.AddString(expr.value)
        frame.AddOpCode(offset)

    def CompileBoolExpr(self, expr: BoolExpr, frame: Frame):
        if expr.value:
            frame.AddOpCode(OpCode.OP_NEW_TRUE)
        else:
            frame.AddOpCode(OpCode.OP_NEW_FALSE)

    def CompileNilExpr(self, expr: NilExpr, frame: Frame):
        frame.AddOpCode(OpCode.OP_NEW_NIL)

    def CompileIdentifierExpr(self, expr: IdentifierExpr, frame: Frame,state:ObjectState=ObjectState.READ):
        if state==ObjectState.READ:
            frame.AddOpCode(OpCode.OP_GET_VAR)
        elif state==ObjectState.WRITE:
            frame.AddOpCode(OpCode.OP_SET_VAR)
        elif state==ObjectState.INIT:
            frame.AddOpCode(OpCode.OP_DEFINE_VAR)
        elif state==ObjectState.STRUCT_READ:
            frame.AddOpCode(OpCode.OP_GET_STRUCT_VAR)
        elif state==ObjectState.STRUCT_WRITE:
            frame.AddOpCode(OpCode.OP_SET_STRUCT_VAR)
            
        offset=frame.AddString(expr.literal)
        frame.AddOpCode(offset)

    def CompileGroupExpr(self, expr: GroupExpr, frame: Frame):
        self.CompileExpr(expr.expr,frame)

    def CompileArrayExpr(self, expr: ArrayExpr, frame: Frame):
        pass

    def CompileIndexExpr(self, expr: IndexExpr, frame: Frame,state:ObjectState=ObjectState.READ):
        pass

    def CompilePrefixExpr(self, expr: PrefixExpr, frame: Frame):
        pass

    def CompileInfixExpr(self, expr: InfixExpr, frame: Frame):
        pass

    def CompileFunctionCallExpr(self, expr: FunctionCallExpr, frame: Frame):
        pass

    def CompileStructCallExpr(self, expr: StructCallExpr, frma: Frame,state:ObjectState=ObjectState.READ):
        pass
