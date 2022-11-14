from typing import List
from Ast import IfStmt, Stmt
from Ast import ExprStmt, FunctionStmt, ReturnStmt, ScopeStmt, StructStmt, VarStmt, WhileStmt
from Ast import ArrayExpr, AstType, BoolExpr, Expr, FunctionCallExpr, GroupExpr, IdentifierExpr, IndexExpr, InfixExpr, LambdaExpr, NilExpr, NumExpr, PrefixExpr, RefExpr, StrExpr, StructCallExpr,AnonyStructExpr


class SemanticAnalyzer:

    def Analyze(self, stmts: List[Stmt]) -> None:
        for s in stmts:
            s=self.__AnalyzeStmt(s)

    def __AnalyzeStmt(self, stmt: Stmt) -> Stmt:
        if stmt.Type() == AstType.RETURN:
            return self.__AnalyzeReturnStmt(stmt)
        elif stmt.Type() == AstType.EXPR:
            return self.__AnalyzeExprStmt(stmt)
        elif stmt.Type() == AstType.VAR:
            return self.__AnalyzeVarStmt(stmt)
        elif stmt.Type() == AstType.SCOPE:
            return self.__AnalyzeScopeStmt(stmt)
        elif stmt.Type() == AstType.IF:
            return self.__AnalyzeIfStmt(stmt)
        elif stmt.Type() == AstType.WHILE:
            return self.__AnalyzeWhileStmt(stmt)
        elif stmt.Type() == AstType.FUNCTION:
            return self.__AnalyzeFunctionStmt(stmt)
        elif stmt.Type() == AstType.STRUCT:
            return self.__AnalyzeStructStmt(stmt)

    def __AnalyzeExprStmt(self, stmt: ExprStmt) -> Stmt:
        stmt.expr = self.__AnalyzeExpr(stmt.expr)
        return stmt

    def __AnalyzeIfStmt(self, stmt: IfStmt) -> Stmt:
        stmt.condition = self.__AnalyzeExpr(stmt.condition)
        stmt.thenBranch = self.__AnalyzeStmt(stmt.thenBranch)
        if stmt.elseBranch:
            stmt.elseBranch = self.__AnalyzeStmt(stmt.elseBranch)

        if stmt.condition.Type() == AstType.BOOL:
            if stmt.condition.value == True:
                return stmt.thenBranch
            else:
                return stmt.elseBranch

        return stmt

    def __AnalyzeScopeStmt(self, stmt: ScopeStmt) -> Stmt:
        for i in range(0,len(stmt.stmts)):
            stmt.stmts[i] = self.__AnalyzeStmt(stmt.stmts[i])
        return stmt

    def __AnalyzeWhileStmt(self, stmt: WhileStmt) -> Stmt:
        stmt.condition = self.__AnalyzeExpr(stmt.condition)
        stmt.body = self.__AnalyzeStmt(stmt.body)
        return stmt

    def __AnalyzeReturnStmt(self, stmt: ReturnStmt) -> Stmt:
        stmt.expr = self.__AnalyzeExpr(stmt.expr)
        return stmt

    def __AnalyzeVarStmt(self, stmt: VarStmt) -> Stmt:
        stmt.value = self.__AnalyzeExpr(stmt.value)
        return stmt

    def __AnalyzeFunctionStmt(self, stmt: FunctionStmt) -> Stmt:
        for e in stmt.parameters:
            e = self.__AnalyzeIdentifierExpr(e)
        stmt.body =self.__AnalyzeScopeStmt(stmt.body)
        return stmt

    def __AnalyzeStructStmt(self, stmt: StructStmt) -> Stmt:
        for s in stmt.members:
            s = self.__AnalyzeVarStmt(s)
        return stmt

    def __AnalyzeExpr(self, expr: Expr) -> Expr:
        if expr.Type() == AstType.NUM:
            return self.__AnalyzeNumExpr(expr)
        elif expr.Type() == AstType.STR:
            return self.__AnalyzeStrExpr(expr)
        elif expr.Type() == AstType.BOOL:
            return self.__AnalyzeBoolExpr(expr)
        elif expr.Type() == AstType.NIL:
            return self.__AnalyzeNilExpr(expr)
        elif expr.Type() == AstType.IDENTIFIER:
            return self.__AnalyzeIdentifierExpr(expr)
        elif expr.Type() == AstType.GROUP:
            return self.__AnalyzeGroupExpr(expr)
        elif expr.Type() == AstType.ARRAY:
            return self.__AnalyzeArrayExpr(expr)
        elif expr.Type() == AstType.INDEX:
            return self.__AnalyzeIndexExpr(expr)
        elif expr.Type() == AstType.PREFIX:
            return self.__AnalyzePrefixExpr(expr)
        elif expr.Type() == AstType.INFIX:
            return self.__AnalyzeInfixExpr(expr)
        elif expr.Type() == AstType.FUNCTION_CALL:
            return self.__AnalyzeFunctionCallExpr(expr)
        elif expr.Type() == AstType.STRUCT_CALL:
            return self.__AnalyzeStructCallExpr(expr)
        elif expr.Type() == AstType.REF:
            return self.__AnalyzeRefExpr(expr)
        elif expr.Type() == AstType.LAMBDA:
            return self.__AnalyzeLambdaExpr(expr)
        elif expr.Type()==AstType.ANONY_STRUCT:
            return self.__AnalyzeAnonyStructExpr(expr)

    def __AnalyzeInfixExpr(self, expr: InfixExpr) -> Expr:
        expr.left = self.__AnalyzeExpr(expr.left)
        expr.right = self.__AnalyzeExpr(expr.right)
        return self.__ConstantFold(expr)

    def __AnalyzeNumExpr(self, expr: NumExpr) -> Expr:
        return expr

    def __AnalyzeBoolExpr(self, expr: BoolExpr) -> Expr:
        return expr

    def __AnalyzePrefixExpr(self, expr: PrefixExpr) -> Expr:
        expr.right = self.__AnalyzeExpr(expr.right)
        return self.__ConstantFold(expr)

    def __AnalyzeStrExpr(self, expr: StrExpr) -> Expr:
        return expr

    def __AnalyzeNilExpr(self, expr: NilExpr) -> Expr:
        return expr

    def __AnalyzeGroupExpr(self, expr: GroupExpr) -> Expr:
        return self.__AnalyzeExpr(expr.expr)

    def __AnalyzeArrayExpr(self, expr: ArrayExpr) -> Expr:
        for e in expr.elements:
            e = self.__AnalyzeExpr(e)
        return expr

    def __AnalyzeIndexExpr(self, expr: IndexExpr) -> Expr:
        expr.ds = self.__AnalyzeExpr(expr.ds)
        expr.index = self.__AnalyzeExpr(expr.index)
        return expr

    def __AnalyzeIdentifierExpr(self, expr: IdentifierExpr) -> Expr:
        return expr

    def __AnalyzeLambdaExpr(self, expr: LambdaExpr) -> Expr:
        for e in expr.parameters:
            e = self.__AnalyzeIdentifierExpr(e)
        expr.body = self.__AnalyzeScopeStmt(expr.body)
        return expr

    def __AnalyzeAnonyStructExpr(self,expr:AnonyStructExpr)->Expr:
        for k,v in expr.memberPairs.items():
            v=self.__AnalyzeExpr(v)
        return expr

    def __AnalyzeFunctionCallExpr(self, expr: FunctionCallExpr) -> Expr:
        expr.name = self.__AnalyzeExpr(expr.name)
        for i in range(0,len(expr.arguments)):
            expr.arguments[i] = self.__AnalyzeExpr(expr.arguments[i] )
        return expr

    def __AnalyzeStructCallExpr(self, expr: StructCallExpr) -> Expr:
        expr.callee = self.__AnalyzeExpr(expr.callee)
        expr.callMember = self.__AnalyzeExpr(expr.callMember)
        return expr

    def __AnalyzeRefExpr(self, expr: RefExpr) -> Expr:
        expr.refExpr = self.__AnalyzeExpr(expr.refExpr)
        return expr

    def __ConstantFold(self, expr: Expr) -> Expr:
        if expr.Type() == AstType.INFIX:
            if expr.left.Type() == AstType.NUM and expr.right.Type() == AstType.NUM:
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
            elif expr.left.Type() == AstType.STR and expr.right.Type() == AstType.STR:
                return StrExpr(expr.left.value+expr.right.value)
        elif expr.Type() == AstType.PREFIX:
            if expr.right.Type() == AstType.NUM and expr.op == "-":
                return NumExpr(-expr.right.value)
            elif expr.right.Type() == AstType.BOOL and expr.op == "!":
                return BoolExpr(not (expr.right.value))

        return expr
