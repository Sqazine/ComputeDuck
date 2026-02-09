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
    __scope_chunk:list[Chunk]=[]
    __symbolTable: SymbolTable

    def __init__(self) -> None:
        self.__symbolTable = None
        self.reset_status()

    def compile(self, stmts: list[Stmt]) -> FunctionObject:

        self.reset_status()

        for stmt in stmts:
            self.__compile_stmt(stmt)
        return FunctionObject(self.__scope_chunk.pop())

    def reset_status(self) -> None:
        self.__scope_chunk = [Chunk()]

        self.__symbolTable = SymbolTable()

        self.__register_builtins()

    def __compile_stmt(self, stmt: Stmt) -> None:
        if stmt.type == AstType.RETURN:
            self.__compile_return_stmt(stmt)
        elif stmt.type == AstType.EXPR:
            self.__compile_expr_stmt(stmt)
        elif stmt.type == AstType.SCOPE:
            self.__compile_scope_stmt(stmt)
        elif stmt.type == AstType.IF:
            self.__compile_if_stmt(stmt)
        elif stmt.type == AstType.WHILE:
            self.__compile_while_stmt(stmt)
        elif stmt.type == AstType.STRUCT:
            self.__compile_struct_stmt(stmt)

    def __compile_expr_stmt(self, stmt: ExprStmt) -> None:
        self.__compile_expr(stmt.expr)

    def __compile_if_stmt(self, stmt: IfStmt) -> None:
        self.__compile_expr(stmt.condition)
        self.__emit(OpCode.OP_JUMP_IF_FALSE)
        jumpIfFalseAddress = self.__emit(65536)
        self.__compile_stmt(stmt.thenBranch)
        self.__emit(OpCode.OP_JUMP)
        jumpAddress = self.__emit(65536)

        self.__modify_opcode(jumpIfFalseAddress, len(self.__cur_chunk().opCodes)-1)

        if stmt.elseBranch:
            self.__compile_stmt(stmt.elseBranch)

        self.__modify_opcode(jumpAddress, len(self.__cur_chunk().opCodes)-1)

    def __compile_scope_stmt(self, stmt: ScopeStmt) -> None:
        self.__symbolTable.EnterScope()
        
        for s in stmt.stmts:
            self.__compile_stmt(s)

        self.__symbolTable.ExitScope()

    def __compile_while_stmt(self, stmt: WhileStmt) -> None:
        self.__enter_scope()
        jumpAddress = len(self.__cur_chunk().opCodes)-1
        self.__compile_expr(stmt.condition)

        self.__emit(OpCode.OP_JUMP_IF_FALSE)
        jumpIfFalseAddress = self.__emit(65536)

        self.__compile_stmt(stmt.body)

        self.__emit(OpCode.OP_JUMP)
        self.__emit(jumpAddress)

        self.__modify_opcode(jumpIfFalseAddress, len(self.__cur_chunk().opCodes)-1)

        self.__exit_scope()

    def __compile_return_stmt(self, stmt: ReturnStmt) -> None:
        if stmt.expr != None:
            self.__compile_expr(stmt.expr)
            self.__emit(OpCode.OP_RETURN)
            self.__emit(1)
        else:
            self.__emit(OpCode.OP_RETURN)
            self.__emit(0)

    def __compile_struct_stmt(self, stmt: StructStmt) -> None:
        symbol = self.__symbolTable.Define(stmt.name, True)

        self.__scope_chunk.append(Chunk())

        for k, v in stmt.members.items():
            self.__compile_expr(v)
            self.__emit_constant(StrObject(k.literal))

        localVarCount = self.__symbolTable.definitionCount

        self.__emit(OpCode.OP_STRUCT)
        self.__emit(len(stmt.members))

        chunk = self.__scope_chunk.pop()

        chunk.opCodes.append(OpCode.OP_RETURN)
        chunk.opCodes.append(1)

        fn = FunctionObject(chunk, localVarCount)

        self.__emit_constant(fn)

        self.__store_symbol(symbol)

    def __compile_expr(self, expr: Expr, state: RWState = RWState.READ) -> None:
        if expr.type == AstType.NUM:
            self.__compile_num_expr(expr)
        elif expr.type == AstType.STR:
            self.__compile_str_expr(expr)
        elif expr.type == AstType.BOOL:
            self.__compile_bool_expr(expr)
        elif expr.type == AstType.NIL:
            self.__compile_nil_expr(expr)
        elif expr.type == AstType.IDENTIFIER:
            self.__compile_identifier_expr(expr, state)
        elif expr.type == AstType.GROUP:
            self.__compile_group_expr(expr)
        elif expr.type == AstType.ARRAY:
            self.__compile_array_expr(expr)
        elif expr.type == AstType.INDEX:
            self.__compile_index_expr(expr,state)
        elif expr.type == AstType.PREFIX:
            self.__compile_prefix_expr(expr)
        elif expr.type == AstType.INFIX:
            self.__compile_infix_expr(expr)
        elif expr.type == AstType.FUNCTION_CALL:
            self.__compile_function_call_expr(expr)
        elif expr.type == AstType.STRUCT_CALL:
            self.__compile_struct_call_expr(expr, state)
        elif expr.type == AstType.REF:
            self.__compile_ref_expr(expr)
        elif expr.type == AstType.FUNCTION:
            self.__compile_function_expr(expr)
        elif expr.type == AstType.STRUCT:
            self.__compile_struct_expr(expr)
        elif expr.type == AstType.DLL_IMPORT:
            self.__compile_dll_import_expr(expr)

    def __compile_infix_expr(self, expr: InfixExpr) -> None:
        if expr.op == "=":
            if expr.left.type == AstType.IDENTIFIER and expr.right.type == AstType.FUNCTION:
                self.__symbolTable.Define(expr.left.literal)
            self.__compile_expr(expr.right)
            self.__compile_expr(expr.left, RWState.WRITE)
        else:
            self.__compile_expr(expr.right)
            self.__compile_expr(expr.left)

            if expr.op == "+":
                self.__emit(OpCode.OP_ADD)
            elif expr.op == "-":
                self.__emit(OpCode.OP_SUB)
            elif expr.op == "*":
                self.__emit(OpCode.OP_MUL)
            elif expr.op == "/":
                self.__emit(OpCode.OP_DIV)
            elif expr.op == ">":
                self.__emit(OpCode.OP_GREATER)
            elif expr.op == "<":
                self.__emit(OpCode.OP_LESS)
            elif expr.op == "&":
                self.__emit(OpCode.OP_BIT_AND)
            elif expr.op == "|":
                self.__emit(OpCode.OP_BIT_OR)
            elif expr.op == "^":
                self.__emit(OpCode.OP_BIT_XOR)
            elif expr.op == ">=":
                self.__emit(OpCode.OP_LESS)
                self.__emit(OpCode.OP_NOT)
            elif expr.op == "<=":
                self.__emit(OpCode.OP_GREATER)
                self.__emit(OpCode.OP_NOT)
            elif expr.op == "==":
                self.__emit(OpCode.OP_EQUAL)
            elif expr.op == "!=":
                self.__emit(OpCode.OP_EQUAL)
                self.__emit(OpCode.OP_NOT)
            elif expr.op == "and":
                self.__emit(OpCode.OP_AND)
            elif expr.op == "or":
                self.__emit(OpCode.OP_OR)

    def __compile_num_expr(self, expr: NumExpr) -> None:
        value = NumObject(expr.value)
        self.__emit_constant(value)

    def __compile_bool_expr(self, expr: BoolExpr) -> None:
        if expr.value:
            self.__emit_constant(BoolObject(True))
        else:
            self.__emit_constant(BoolObject(False))

    def __compile_prefix_expr(self, expr: PrefixExpr) -> None:
        self.__compile_expr(expr.right)
        if expr.op == "-":
            self.__emit(OpCode.OP_MINUS)
        elif expr.op == "not":
            self.__emit(OpCode.OP_NOT)
        elif expr.op == "~":
            self.__emit(OpCode.OP_BIT_NOT)
        else:
            error("Unrecognized prefix op")

    def __compile_str_expr(self, expr: StrExpr) -> None:
        obj = StrObject(expr.value)
        self.__emit_constant(obj)

    def __compile_nil_expr(self, expr: NilExpr) -> None:
        self.__emit_constant(NilObject())

    def __compile_group_expr(self, expr: GroupExpr) -> None:
        self.__compile_expr(expr.expr)

    def __compile_array_expr(self, expr: ArrayExpr) -> None:
        for e in expr.elements:
            self.__compile_expr(e)

        self.__emit(OpCode.OP_ARRAY)
        self.__emit(len(expr.elements))

    def __compile_index_expr(self, expr: IndexExpr,state:RWState) -> None:
        self.__compile_expr(expr.ds)
        self.__compile_expr(expr.index)
        if state==RWState.WRITE:
            self.__emit(OpCode.OP_SET_INDEX)
        else:
            self.__emit(OpCode.OP_GET_INDEX)

    def __compile_identifier_expr(self, expr: IdentifierExpr, state: RWState) -> None:
        isFound, symbol = self.__symbolTable.Resolve(expr.literal)

        if state == RWState.READ:
            if isFound == False:
                error("Undefined variable:"+expr.__str__())
            self.__load_symbol(symbol)
        else:
            if isFound == False:
                symbol = self.__symbolTable.Define(expr.literal)
                self.__define_symbol(symbol)
            else:
                self.__store_symbol(symbol)

    def __compile_function_expr(self, stmt: FunctionExpr) -> None:
        self.__enter_scope()

        self.__scope_chunk.append(Chunk())

        for param in stmt.parameters:
            self.__symbolTable.Define(param.literal)

        self.__compile_stmt(stmt.body)

        localVarCount = self.__symbolTable.definitionCount

        self.__exit_scope()

        chunk = self.__scope_chunk.pop()

        # for non return  or empty stmt in function scope:add a return to return nothing
        opCodesLen = len(chunk.opCodes)
        if opCodesLen == 0 or chunk.opCodes[opCodesLen-2] != OpCode.OP_RETURN:
            chunk.opCodes.append(OpCode.OP_RETURN)
            chunk.opCodes.append(0)

        fn = FunctionObject(chunk, localVarCount, len(stmt.parameters))
        self.__emit_constant(fn)

    def __compile_function_call_expr(self, expr: FunctionCallExpr) -> None:
        self.__compile_expr(expr.name)

        for argu in expr.arguments:
            self.__compile_expr(argu)

        self.__emit(OpCode.OP_FUNCTION_CALL)
        self.__emit(len(expr.arguments))

    def __compile_struct_call_expr(self, expr: StructCallExpr, state: RWState) -> None:
        if expr.callMember.type == AstType.FUNCTION_CALL and state == RWState.WRITE:
            error("Cannot assign to a struct's function call expr")

        self.__compile_expr(expr.callee)

        if expr.callMember.type == AstType.IDENTIFIER:
            self.__emit_constant(StrObject(expr.callMember.literal))
        elif expr.callMember.type == AstType.FUNCTION_CALL:
            self.__emit_constant(StrObject(expr.callMember.name.literal))

        if state == RWState.READ:
            self.__emit(OpCode.OP_GET_STRUCT)

            if expr.callMember.type == AstType.FUNCTION_CALL:
                funcCall = expr.callMember
                for argu in funcCall.arguments:
                    self.__compile_expr(argu)

                self.__emit(OpCode.OP_FUNCTION_CALL)
                self.__emit(len(funcCall.arguments))
        else:
            self.__emit(OpCode.OP_SET_STRUCT)

    def __compile_ref_expr(self, expr: RefExpr) -> None:

        if expr.refExpr.type == AstType.INDEX:
            self.__compile_expr(expr.refExpr.index)
            isFound, symbol = self.__symbolTable.Resolve(
                expr.refExpr.ds.__str__())
            if isFound == False:
                error("Undefined variable:"+expr.__str__())

            if symbol.scope == SymbolScope.GLOBAL:
                self.__emit(OpCode.OP_REF_INDEX_GLOBAL)
                self.__emit(symbol.index)
            elif symbol.scope == SymbolScope.LOCAL:
                self.__emit(OpCode.OP_REF_INDEX_LOCAL)
                self.__emit(symbol.scopeDepth)
                self.__emit(symbol.index)
                self.__emit(symbol.isUpValue)
        else:
            isFound, symbol = self.__symbolTable.Resolve(
                expr.refExpr.__str__())
            if isFound == False:
                error("Undefined variable:"+expr.__str__())

            if symbol.scope == SymbolScope.GLOBAL:
                self.__emit(OpCode.OP_REF_GLOBAL)
                self.__emit(symbol.index)
            elif symbol.scope == SymbolScope.LOCAL:
                self.__emit(OpCode.OP_REF_LOCAL)
                self.__emit(symbol.scopeDepth)
                self.__emit(symbol.index)
                self.__emit(symbol.isUpValue)

    def __compile_struct_expr(self, expr: StructExpr) -> None:
        for k, v in expr.memberPairs.items():
            self.__compile_expr(v)
            self.__emit_constant(StrObject(k.literal))

        self.__emit(OpCode.OP_STRUCT)
        self.__emit(len(expr.memberPairs))

    def __compile_dll_import_expr(self, expr: DllImportExpr) -> None:
        dllPath = "library-" + expr.dllPath

        register_dlls(dllPath)

        self.__emit_constant(StrObject(dllPath))
        self.__emit(OpCode.OP_DLL_IMPORT)

        self.__register_builtins()

    def __enter_scope(self) -> None:
        self.__symbolTable = SymbolTable(self.__symbolTable)

    def __exit_scope(self) -> list[int]:
        self.__symbolTable = self.__symbolTable.enclosing

    def __cur_chunk(self) -> Chunk:
        return self.__scope_chunk[-1]

    def __emit(self, opcode: int) -> int:
        self.__cur_chunk().opCodes.append(opcode)
        return len(self.__cur_chunk().opCodes)-1

    def __emit_constant(self, object: Object) -> int:
        self.__cur_chunk().constants.append(object)
        pos = len(self.__cur_chunk().constants)-1

        self.__emit(OpCode.OP_CONSTANT)
        self.__emit(pos)
        return len(self.__cur_chunk().opCodes)-1

    def __modify_opcode(self, pos: int, opcode: int) -> None:
        self.__cur_chunk().opCodes[pos] = opcode

    def __define_symbol(self, symbol: Symbol) -> None:
        if symbol.scope == SymbolScope.GLOBAL:
            self.__emit(OpCode.OP_DEF_GLOBAL)
            self.__emit(symbol.index)
        elif symbol.scope == SymbolScope.LOCAL:
            self.__emit(OpCode.OP_DEF_LOCAL)
            self.__emit(symbol.scopeDepth)
            self.__emit(symbol.index)

    def __load_symbol(self, symbol: Symbol) -> None:
        if symbol.scope == SymbolScope.GLOBAL:
            self.__emit(OpCode.OP_GET_GLOBAL)
            self.__emit(symbol.index)
        elif symbol.scope == SymbolScope.LOCAL:
            self.__emit(OpCode.OP_GET_LOCAL)
            self.__emit(symbol.scopeDepth)
            self.__emit(symbol.index)
            self.__emit(symbol.isUpValue)
        elif symbol.scope == SymbolScope.BUILTIN:
            self.__emit_constant(StrObject(symbol.name))
            self.__emit(OpCode.OP_GET_BUILTIN)

        if symbol.isStructSymbol:
            self.__emit(OpCode.OP_FUNCTION_CALL)
            self.__emit(0)

    def __store_symbol(self, symbol: Symbol) -> None:
        if symbol.scope == SymbolScope.GLOBAL:
            self.__emit(OpCode.OP_SET_GLOBAL)
            self.__emit(symbol.index)
        elif symbol.scope == SymbolScope.LOCAL:
            self.__emit(OpCode.OP_SET_LOCAL)
            self.__emit(symbol.scopeDepth)
            self.__emit(symbol.index)
            self.__emit(symbol.isUpValue)

    def __register_builtins(self):
        for k in gBuiltinManager.builtinObjects.keys():
            self.__symbolTable.DefineBuiltin(k)
