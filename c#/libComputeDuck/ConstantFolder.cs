using System.Collections.Generic;

namespace ComputeDuck
{
    public class ConstantFolder
    {
        public List<Stmt> Fold(List<Stmt> stmts)
        {
            for (int i = 0; i < stmts.Count; ++i)
                stmts[i] = FoldStmt(stmts[i]);
            return stmts;
        }

        Stmt FoldStmt(Stmt stmt)
        {
            switch (stmt.type)
            {
                case AstType.RETURN:
                    return FoldReturnStmt((ReturnStmt)stmt);
                case AstType.EXPR:
                    return FoldExprStmt((ExprStmt)stmt);
                case AstType.SCOPE:
                    return FoldScopeStmt((ScopeStmt)stmt);
                case AstType.IF:
                    return FoldIfStmt((IfStmt)stmt);
                case AstType.WHILE:
                    return FoldWhileStmt((WhileStmt)stmt);
                case AstType.STRUCT:
                    return FoldStructStmt((StructStmt)stmt);
                default:
                    return stmt;
            }
        }
        Stmt FoldExprStmt(ExprStmt stmt)
        {
            stmt.expr = FoldExpr(stmt.expr);
            return stmt;
        }
        Stmt FoldIfStmt(IfStmt stmt)
        {
            stmt.condition = FoldExpr(stmt.condition);
            stmt.thenBranch = FoldStmt(stmt.thenBranch);

            if (stmt.elseBranch != null)
                stmt.elseBranch = FoldStmt(stmt.elseBranch);

            if (stmt.condition.type == AstType.BOOL)
            {
                if (((BoolExpr)stmt.condition).value == true)
                    return stmt.thenBranch;
                else
                    return stmt.elseBranch;
            }
            return stmt;
        }
        Stmt FoldScopeStmt(ScopeStmt stmt)
        {
            for (int i = 0; i < stmt.stmts.Count; ++i)
                stmt.stmts[i] = FoldStmt(stmt.stmts[i]);
            return stmt;
        }
        Stmt FoldWhileStmt(WhileStmt stmt)
        {
            stmt.condition = FoldExpr(stmt.condition);
            stmt.body = FoldStmt(stmt.body);
            return stmt;
        }
        Stmt FoldReturnStmt(ReturnStmt stmt)
        {
            stmt.expr = FoldExpr(stmt.expr);
            return stmt;
        }
        Stmt FoldStructStmt(StructStmt stmt)
        {
            for (int i = 0; i < stmt.members.Count; ++i)
                stmt.members[i] = new KeyValuePair<IdentifierExpr, Expr?>(stmt.members[i].Key, FoldExpr(stmt.members[i].Value));
            return stmt;
        }

        Expr FoldExpr(Expr expr)
        {
            switch (expr.type)
            {
                case AstType.NUM:
                    return FoldNumExpr((NumExpr)expr);
                case AstType.STR:
                    return FoldStrExpr((StrExpr)expr);
                case AstType.BOOL:
                    return FoldBoolExpr((BoolExpr)expr);
                case AstType.NIL:
                    return FoldNilExpr((NilExpr)expr);
                case AstType.IDENTIFIER:
                    return FoldIdentifierExpr((IdentifierExpr)expr);
                case AstType.GROUP:
                    return FoldGroupExpr((GroupExpr)expr);
                case AstType.ARRAY:
                    return FoldArrayExpr((ArrayExpr)expr);
                case AstType.INDEX:
                    return FoldIndexExpr((IndexExpr)expr);
                case AstType.PREFIX:
                    return FoldPrefixExpr((PrefixExpr)expr);
                case AstType.INFIX:
                    return FoldInfixExpr((InfixExpr)expr);
                case AstType.FUNCTION_CALL:
                    return FoldFunctionCallExpr((FunctionCallExpr)expr);
                case AstType.STRUCT_CALL:
                    return FoldStructCallExpr((StructCallExpr)expr);
                case AstType.REF:
                    return FoldRefExpr((RefExpr)expr);
                case AstType.FUNCTION:
                    return FoldFunctionExpr((FunctionExpr)expr);
                case AstType.STRUCT:
                    return FoldStructExpr((StructExpr)expr);
                default:
                    return expr;
            }
        }
        Expr FoldInfixExpr(InfixExpr expr)
        {
            expr.left = FoldExpr(expr.left);
            expr.right = FoldExpr(expr.right);

            return ConstantFold(expr);
        }
        Expr FoldNumExpr(NumExpr expr)
        {
            return expr;
        }
        Expr FoldBoolExpr(BoolExpr expr)
        {
            return expr;
        }
        Expr FoldPrefixExpr(PrefixExpr expr)
        {
            expr.right = FoldExpr(expr.right);
            return ConstantFold(expr);
        }
        Expr FoldStrExpr(StrExpr expr)
        {
            return expr;
        }
        Expr FoldNilExpr(NilExpr expr)
        {
            return expr;
        }
        Expr FoldGroupExpr(GroupExpr expr)
        {
            return FoldExpr(expr.expr);
        }
        Expr FoldArrayExpr(ArrayExpr expr)
        {
            for (int i = 0; i < expr.elements.Count; ++i)
                expr.elements[i] = FoldExpr(expr.elements[i]);
            return expr;
        }
        Expr FoldIndexExpr(IndexExpr expr)
        {
            expr.ds = FoldExpr(expr.ds);
            expr.index = FoldExpr(expr.index);
            return expr;
        }
        Expr FoldIdentifierExpr(IdentifierExpr expr)
        {
            return expr;
        }
        Expr FoldFunctionExpr(FunctionExpr expr)
        {
            for (int i = 0; i < expr.parameters.Count; ++i)
                expr.parameters[i] = (IdentifierExpr)FoldIdentifierExpr(expr.parameters[i]);

            expr.body = (ScopeStmt)FoldScopeStmt(expr.body);
            return expr;
        }
        Expr FoldFunctionCallExpr(FunctionCallExpr expr)
        {
            expr.name = FoldExpr(expr.name);
            for (int i = 0; i < expr.arguments.Count; ++i)
                expr.arguments[i] = FoldExpr(expr.arguments[i]);
            return expr;
        }
        Expr FoldStructCallExpr(StructCallExpr expr)
        {
            expr.callee = FoldExpr(expr.callee);
            expr.callMember = FoldExpr(expr.callMember);
            return expr;
        }
        Expr FoldRefExpr(RefExpr expr)
        {
            expr.refExpr = FoldExpr(expr.refExpr);
            return expr;
        }
        Expr FoldStructExpr(StructExpr expr)
        {
            for (int i = 0; i < expr.memberPairs.Count; ++i)
                expr.memberPairs[i] = new KeyValuePair<IdentifierExpr, Expr?>(expr.memberPairs[i].Key, FoldExpr(expr.memberPairs[i].Value));
            return expr;
        }

        Expr ConstantFold(Expr expr)
        {
            if (expr.type == AstType.INFIX)
            {
                var infix = (InfixExpr)expr;
                if (infix.left.type == AstType.NUM && infix.right.type == AstType.NUM)
                {
                    Expr newExpr = null;
                    if (infix.op == "+")
                        newExpr = new NumExpr(((NumExpr)infix.left).value + ((NumExpr)infix.right).value);
                    else if (infix.op == "-")
                        newExpr = new NumExpr(((NumExpr)infix.left).value - ((NumExpr)infix.right).value);
                    else if (infix.op == "*")
                        newExpr = new NumExpr(((NumExpr)infix.left).value * ((NumExpr)infix.right).value);
                    else if (infix.op == "/")
                        newExpr = new NumExpr(((NumExpr)infix.left).value / ((NumExpr)infix.right).value);
                    else if (infix.op == "==")
                        newExpr = new BoolExpr(((NumExpr)infix.left).value == ((NumExpr)infix.right).value);
                    else if (infix.op == "!=")
                        newExpr = new BoolExpr(((NumExpr)infix.left).value != ((NumExpr)infix.right).value);
                    else if (infix.op == ">")
                        newExpr = new BoolExpr(((NumExpr)infix.left).value > ((NumExpr)infix.right).value);
                    else if (infix.op == ">=")
                        newExpr = new BoolExpr(((NumExpr)infix.left).value >= ((NumExpr)infix.right).value);
                    else if (infix.op == "<")
                        newExpr = new BoolExpr(((NumExpr)infix.left).value < ((NumExpr)infix.right).value);
                    else if (infix.op == "<=")
                        newExpr = new BoolExpr(((NumExpr)infix.left).value <= ((NumExpr)infix.right).value);
                    return newExpr;
                }
                else if (infix.left.type == AstType.STR && infix.right.type == AstType.STR)
                {
                    var strExpr = new StrExpr(((StrExpr)infix.left).value + ((StrExpr)infix.right).value);
                    return strExpr;
                }
            }
            else if (expr.type == AstType.PREFIX)
            {
                var prefix = (PrefixExpr)expr;
                if (prefix.right.type == AstType.NUM && prefix.op == "-")
                {
                    var numExpr = new NumExpr(-((NumExpr)prefix.right).value);
                    return numExpr;
                }
                else if (prefix.right.type == AstType.BOOL && prefix.op == "not")
                {
                    var boolExpr = new BoolExpr(!((BoolExpr)prefix.right).value);
                    return boolExpr;
                }
            }

            return expr;
        }
    }
}
