from Ast import *


class ConstantFolder:

    def Fold(self, stmts: list[Stmt]) -> None:
        for s in stmts:
            s = self.__FoldStmt(s)

    def __FoldStmt(self, stmt: Stmt) -> Stmt:
        if stmt.type == AstType.RETURN:
            return self.__FoldReturnStmt(stmt)
        elif stmt.type == AstType.EXPR:
            return self.__FoldExprStmt(stmt)
        elif stmt.type == AstType.SCOPE:
            return self.__FoldScopeStmt(stmt)
        elif stmt.type == AstType.IF:
            return self.__FoldIfStmt(stmt)
        elif stmt.type == AstType.WHILE:
            return self.__FoldWhileStmt(stmt)
        elif stmt.type == AstType.STRUCT:
            return self.__FoldStructStmt(stmt)

    def __FoldExprStmt(self, stmt: ExprStmt) -> Stmt:
        stmt.expr = self.__FoldExpr(stmt.expr)
        return stmt

    def __FoldIfStmt(self, stmt: IfStmt) -> Stmt:
        stmt.condition = self.__FoldExpr(stmt.condition)
        stmt.thenBranch = self.__FoldStmt(stmt.thenBranch)
        if stmt.elseBranch:
            stmt.elseBranch = self.__FoldStmt(stmt.elseBranch)

        if stmt.condition.type == AstType.BOOL:
            if stmt.condition.value == True:
                return stmt.thenBranch
            else:
                return stmt.elseBranch

        return stmt

    def __FoldScopeStmt(self, stmt: ScopeStmt) -> Stmt:
        for i in range(0, len(stmt.stmts)):
            stmt.stmts[i] = self.__FoldStmt(stmt.stmts[i])
        return stmt

    def __FoldWhileStmt(self, stmt: WhileStmt) -> Stmt:
        stmt.condition = self.__FoldExpr(stmt.condition)
        stmt.body = self.__FoldStmt(stmt.body)
        return stmt

    def __FoldReturnStmt(self, stmt: ReturnStmt) -> Stmt:
        stmt.expr = self.__FoldExpr(stmt.expr)
        return stmt

    def __FoldStructStmt(self, stmt: StructStmt) -> Stmt:
        for s in stmt.members:
            s = self.__FoldExpr(s)
        return stmt

    def __FoldExpr(self, expr: Expr) -> Expr:
        if expr.type == AstType.NUM:
            return self.__FoldNumExpr(expr)
        elif expr.type == AstType.STR:
            return self.__FoldStrExpr(expr)
        elif expr.type == AstType.BOOL:
            return self.__FoldBoolExpr(expr)
        elif expr.type == AstType.NIL:
            return self.__FoldNilExpr(expr)
        elif expr.type == AstType.IDENTIFIER:
            return self.__FoldIdentifierExpr(expr)
        elif expr.type == AstType.GROUP:
            return self.__FoldGroupExpr(expr)
        elif expr.type == AstType.ARRAY:
            return self.__FoldArrayExpr(expr)
        elif expr.type == AstType.INDEX:
            return self.__FoldIndexExpr(expr)
        elif expr.type == AstType.PREFIX:
            return self.__FoldPrefixExpr(expr)
        elif expr.type == AstType.INFIX:
            return self.__FoldInfixExpr(expr)
        elif expr.type == AstType.FUNCTION_CALL:
            return self.__FoldFunctionCallExpr(expr)
        elif expr.type == AstType.STRUCT_CALL:
            return self.__FoldStructCallExpr(expr)
        elif expr.type == AstType.REF:
            return self.__FoldRefExpr(expr)
        elif expr.type == AstType.FUNCTION:
            return self.__FoldFunctionExpr(expr)
        elif expr.type == AstType.ANONY_STRUCT:
            return self.__FoldAnonyStructExpr(expr)
        else:
            return expr

    def __FoldInfixExpr(self, expr: InfixExpr) -> Expr:
        expr.left = self.__FoldExpr(expr.left)
        expr.right = self.__FoldExpr(expr.right)
        return self.__ConstantFold(expr)

    def __FoldNumExpr(self, expr: NumExpr) -> Expr:
        return expr

    def __FoldBoolExpr(self, expr: BoolExpr) -> Expr:
        return expr

    def __FoldPrefixExpr(self, expr: PrefixExpr) -> Expr:
        expr.right = self.__FoldExpr(expr.right)
        return self.__ConstantFold(expr)

    def __FoldStrExpr(self, expr: StrExpr) -> Expr:
        return expr

    def __FoldNilExpr(self, expr: NilExpr) -> Expr:
        return expr

    def __FoldGroupExpr(self, expr: GroupExpr) -> Expr:
        return self.__FoldExpr(expr.expr)

    def __FoldArrayExpr(self, expr: ArrayExpr) -> Expr:
        for e in expr.elements:
            e = self.__FoldExpr(e)
        return expr

    def __FoldIndexExpr(self, expr: IndexExpr) -> Expr:
        expr.ds = self.__FoldExpr(expr.ds)
        expr.index = self.__FoldExpr(expr.index)
        return expr

    def __FoldIdentifierExpr(self, expr: IdentifierExpr) -> Expr:
        return expr

    def __FoldFunctionExpr(self, expr: FunctionExpr) -> Expr:
        for i in range(0, len(expr.parameters)):
            expr.parameters[i] = self.__FoldIdentifierExpr(expr.parameters[i])
        expr.body = self.__FoldScopeStmt(expr.body)
        return expr

    def __FoldFunctionCallExpr(self, expr: FunctionCallExpr) -> Expr:
        expr.name = self.__FoldExpr(expr.name)
        for i in range(0, len(expr.arguments)):
            expr.arguments[i] = self.__FoldExpr(expr.arguments[i])
        return expr

    def __FoldStructCallExpr(self, expr: StructCallExpr) -> Expr:
        expr.callee = self.__FoldExpr(expr.callee)
        expr.callMember = self.__FoldExpr(expr.callMember)
        return expr

    def __FoldRefExpr(self, expr: RefExpr) -> Expr:
        expr.refExpr = self.__FoldExpr(expr.refExpr)
        return expr

    def __FoldAnonyStructExpr(self, expr: AnonyStructExpr) -> Expr:
        for k, v in expr.memberPairs.items():
            v = self.__FoldExpr(v)
        return expr

    def __ConstantFold(self, expr: Expr) -> Expr:
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
