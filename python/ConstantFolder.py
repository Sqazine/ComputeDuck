from Ast import *


class ConstantFolder:

    def fold(self, stmts: list[Stmt]) -> None:
        for s in stmts:
            s = self.__fold_stmt(s)

    def __fold_stmt(self, stmt: Stmt) -> Stmt:
        if stmt.type == AstType.RETURN:
            return self.__fold_return_stmt(stmt)
        elif stmt.type == AstType.EXPR:
            return self.__fold_expr_stmt(stmt)
        elif stmt.type == AstType.SCOPE:
            return self.__fold_scope_stmt(stmt)
        elif stmt.type == AstType.IF:
            return self.__fold_if_stmt(stmt)
        elif stmt.type == AstType.WHILE:
            return self.__fold_while_stmt(stmt)
        elif stmt.type == AstType.STRUCT:
            return self.__fold_struct_stmt(stmt)

    def __fold_expr_stmt(self, stmt: ExprStmt) -> Stmt:
        stmt.expr = self.__fold_expr(stmt.expr)
        return stmt

    def __fold_if_stmt(self, stmt: IfStmt) -> Stmt:
        stmt.condition = self.__fold_expr(stmt.condition)
        stmt.thenBranch = self.__fold_stmt(stmt.thenBranch)
        if stmt.elseBranch:
            stmt.elseBranch = self.__fold_stmt(stmt.elseBranch)
        if stmt.condition.type == AstType.BOOL:
            if stmt.condition.value == True:
                return stmt.thenBranch
            else:
                return stmt.elseBranch
        return stmt

    def __fold_scope_stmt(self, stmt: ScopeStmt) -> Stmt:
        for i in range(0, len(stmt.stmts)):
            stmt.stmts[i] = self.__fold_stmt(stmt.stmts[i])
        return stmt

    def __fold_while_stmt(self, stmt: WhileStmt) -> Stmt:
        stmt.condition = self.__fold_expr(stmt.condition)
        stmt.body = self.__fold_stmt(stmt.body)
        return stmt

    def __fold_return_stmt(self, stmt: ReturnStmt) -> Stmt:
        stmt.expr = self.__fold_expr(stmt.expr)
        return stmt

    def __fold_struct_stmt(self, stmt: StructStmt) -> Stmt:
        for s in stmt.members:
            s = self.__fold_expr(s)
        return stmt

    def __fold_expr(self, expr: Expr) -> Expr:
        if expr.type == AstType.NUM:
            return self.__fold_num_expr(expr)
        elif expr.type == AstType.STR:
            return self.__fold_str_expr(expr)
        elif expr.type == AstType.BOOL:
            return self.__fold_bool_expr(expr)
        elif expr.type == AstType.NIL:
            return self.__fold_nil_expr(expr)
        elif expr.type == AstType.IDENTIFIER:
            return self.__fold_identifier_expr(expr)
        elif expr.type == AstType.GROUP:
            return self.__fold_group_expr(expr)
        elif expr.type == AstType.ARRAY:
            return self.__fold_array_expr(expr)
        elif expr.type == AstType.INDEX:
            return self.__fold_index_expr(expr)
        elif expr.type == AstType.PREFIX:
            return self.__fold_prefix_expr(expr)
        elif expr.type == AstType.INFIX:
            return self.__fold_infix_expr(expr)
        elif expr.type == AstType.FUNCTION_CALL:
            return self.__fold_function_call_expr(expr)
        elif expr.type == AstType.STRUCT_CALL:
            return self.__fold_struct_call_expr(expr)
        elif expr.type == AstType.REF:
            return self.__fold_ref_expr(expr)
        elif expr.type == AstType.FUNCTION:
            return self.__fold_function_expr(expr)
        elif expr.type == AstType.STRUCT:
            return self.__fold_struct_expr(expr)
        else:
            return expr

    def __fold_infix_expr(self, expr: InfixExpr) -> Expr:
        expr.left = self.__fold_expr(expr.left)
        expr.right = self.__fold_expr(expr.right)
        return self.__constant_fold(expr)

    def __fold_num_expr(self, expr: NumExpr) -> Expr:
        return expr

    def __fold_bool_expr(self, expr: BoolExpr) -> Expr:
        return expr

    def __fold_prefix_expr(self, expr: PrefixExpr) -> Expr:
        expr.right = self.__fold_expr(expr.right)
        return self.__constant_fold(expr)

    def __fold_str_expr(self, expr: StrExpr) -> Expr:
        return expr

    def __fold_nil_expr(self, expr: NilExpr) -> Expr:
        return expr

    def __fold_group_expr(self, expr: GroupExpr) -> Expr:
        return self.__fold_expr(expr.expr)

    def __fold_array_expr(self, expr: ArrayExpr) -> Expr:
        for e in expr.elements:
            e = self.__fold_expr(e)
        return expr

    def __fold_index_expr(self, expr: IndexExpr) -> Expr:
        expr.ds = self.__fold_expr(expr.ds)
        expr.index = self.__fold_expr(expr.index)
        return expr

    def __fold_identifier_expr(self, expr: IdentifierExpr) -> Expr:
        return expr

    def __fold_function_expr(self, expr: FunctionExpr) -> Expr:
        for i in range(0, len(expr.parameters)):
            expr.parameters[i] = self.__fold_identifier_expr(
                expr.parameters[i])
        expr.body = self.__fold_scope_stmt(expr.body)
        return expr

    def __fold_function_call_expr(self, expr: FunctionCallExpr) -> Expr:
        expr.name = self.__fold_expr(expr.name)
        for i in range(0, len(expr.arguments)):
            expr.arguments[i] = self.__fold_expr(expr.arguments[i])
        return expr

    def __fold_struct_call_expr(self, expr: StructCallExpr) -> Expr:
        expr.callee = self.__fold_expr(expr.callee)
        expr.callMember = self.__fold_expr(expr.callMember)
        return expr

    def __fold_ref_expr(self, expr: RefExpr) -> Expr:
        expr.refExpr = self.__fold_expr(expr.refExpr)
        return expr

    def __fold_struct_expr(self, expr: StructExpr) -> Expr:
        for k, v in expr.memberPairs.items():
            v = self.__fold_expr(v)
        return expr

    def __constant_fold(self, expr: Expr) -> Expr:
        if expr.type == AstType.INFIX:
            if expr.left.type == AstType.NUM and expr.right.type == AstType.NUM:
                newExpr = None
                if expr.op == "+":
                    newExpr = NumExpr(expr.left.value+expr.right.value)
                elif expr.op == "-":
                    newExpr = NumExpr(expr.left.value-expr.right.value)
                elif expr.op == "*":
                    newExpr = NumExpr(expr.left.value*expr.right.value)
                elif expr.op == "/":
                    newExpr = NumExpr(expr.left.value/expr.right.value)
                elif expr.op == "==":
                    newExpr = BoolExpr(expr.left.value == expr.right.value)
                elif expr.op == "!=":
                    newExpr = BoolExpr(expr.left.value != expr.right.value)
                elif expr.op == ">":
                    newExpr = BoolExpr(expr.left.value > expr.right.value)
                elif expr.op == "<":
                    newExpr = BoolExpr(expr.left.value < expr.right.value)
                elif expr.op == ">=":
                    newExpr = BoolExpr(expr.left.value >= expr.right.value)
                elif expr.op == "<=":
                    newExpr = BoolExpr(expr.left.value <= expr.right.value)
                return newExpr
            elif expr.left.type == AstType.STR and expr.right.type == AstType.STR:
                return StrExpr(expr.left.value+expr.right.value)
        elif expr.type == AstType.PREFIX:
            if expr.right.type == AstType.NUM and expr.op == "-":
                return NumExpr(-expr.right.value)
            elif expr.right.type == AstType.BOOL and expr.op == "!":
                return BoolExpr(not (expr.right.value))
        return expr
