namespace ComputeDuck
{
    public enum ObjectState
    {
        INIT,
        READ,
        WRITE,
        STRUCT_READ,
        STRUCT_WRITE
    }

    public class Compiler
    {
        public Compiler()
        {
            m_RootFrame = new Frame();
        }
        public Frame Compile(List<Stmt> stmts)
        {
            foreach (var s in stmts)
                CompileStmt(s, ref m_RootFrame);
            return m_RootFrame;
        }

        public void ResetStatus()
        {
            m_RootFrame.Clear();
        }
        private void CompileStmt(Stmt stmt, ref Frame frame)
        {
            switch (stmt.Type())
            {
                case AstType.RETURN:
                    CompileReturnStmt((ReturnStmt)stmt, ref frame);
                    break;
                case AstType.EXPR:
                    CompileExprStmt((ExprStmt)stmt, ref frame);
                    break;
                case AstType.VAR:
                    CompileVarStmt((VarStmt)stmt, ref frame);
                    break;
                case AstType.SCOPE:
                    CompileScopeStmt((ScopeStmt)stmt, ref frame);
                    break;
                case AstType.IF:
                    CompileIfStmt((IfStmt)stmt, ref frame);
                    break;
                case AstType.WHILE:
                    CompileWhileStmt((WhileStmt)stmt, ref frame);
                    break;
                case AstType.FUNCTION:
                    CompileFunctionStmt((FunctionStmt)stmt, ref frame);
                    break;
                case AstType.STRUCT:
                    CompileStructStmt((StructStmt)stmt, ref frame);
                    break;
                default:
                    break;
            }
        }
        private void CompileReturnStmt(ReturnStmt stmt, ref Frame frame)
        {
            if (stmt.expr != null)
                CompileExpr(stmt.expr, ref frame);
            frame.AddOpCode((int)OpCode.OP_RETURN);
        }
        private void CompileExprStmt(ExprStmt stmt, ref Frame frame)
        {
            CompileExpr(stmt.expr, ref frame);
        }
        private void CompileVarStmt(VarStmt stmt, ref Frame frame)
        {
            CompileExpr(stmt.value, ref frame);
            CompileExpr(stmt.name, ref frame, ObjectState.INIT);
        }
        private void CompileScopeStmt(ScopeStmt stmt, ref Frame frame)
        {
            frame.AddOpCode((int)OpCode.OP_ENTER_SCOPE);
            foreach (var s in stmt.stmts)
                CompileStmt(s, ref frame);
            frame.AddOpCode((int)OpCode.OP_EXIT_SCOPE);
        }
        private void CompileIfStmt(IfStmt stmt, ref Frame frame)
        {
            CompileExpr(stmt.condition, ref frame);
            frame.AddOpCode((int)OpCode.OP_JUMP_IF_FALSE);
            int jmpIfFalseOffset = frame.AddNum(0);
            frame.AddOpCode(jmpIfFalseOffset);
            CompileStmt(stmt.thenBranch, ref frame);
            frame.AddOpCode((int)OpCode.OP_JUMP);
            int jmpOffset = frame.AddNum(0);
            frame.AddOpCode(jmpOffset);
            frame.m_Nums[jmpIfFalseOffset] = (double)frame.m_Codes.Count - 1.0;
            if (stmt.elseBranch != null)
                CompileStmt(stmt.elseBranch, ref frame);
            frame.m_Nums[jmpOffset] = (double)frame.m_Codes.Count - 1.0;
        }
        private void CompileWhileStmt(WhileStmt stmt, ref Frame frame)
        {
            int jmpAddress = frame.m_Codes.Count - 1;
            CompileExpr(stmt.condition, ref frame);
            frame.AddOpCode((int)OpCode.OP_JUMP_IF_FALSE);
            int jmpIfFalseOffset = frame.AddNum(0);
            frame.AddOpCode(jmpIfFalseOffset);
            CompileStmt(stmt.body, ref frame);
            frame.AddOpCode((int)OpCode.OP_JUMP);
            int offset = frame.AddNum(jmpAddress);
            frame.AddOpCode(offset);
            frame.m_Nums[jmpIfFalseOffset] = (double)frame.m_Codes.Count - 1.0;
        }
        private void CompileFunctionStmt(FunctionStmt stmt, ref Frame frame)
        {
            Frame functionFrame = new Frame(ref frame);
            functionFrame.AddOpCode((int)OpCode.OP_ENTER_SCOPE);
            for (int i = stmt.parameters.Count - 1; i >= 0; --i)
                CompileIdentifierExpr(stmt.parameters[i], ref functionFrame, ObjectState.INIT);
            foreach (var s in stmt.body.stmts)
                CompileStmt(s, ref functionFrame);
            functionFrame.AddOpCode((int)OpCode.OP_EXIT_SCOPE);
            frame.AddFunctionFrame(stmt.name, functionFrame);
        }
        private void CompileStructStmt(StructStmt stmt, ref Frame frame)
        {
            Frame structFrame = new Frame(ref frame);
            structFrame.AddOpCode((int)OpCode.OP_ENTER_SCOPE);
            foreach (var m in stmt.members)
                CompileVarStmt(m, ref structFrame);
            structFrame.AddOpCode((int)OpCode.OP_NEW_STRUCT);
            int offset = structFrame.AddString(stmt.name);
            structFrame.AddOpCode(offset);
            structFrame.AddOpCode((int)OpCode.OP_RETURN);
            frame.AddStructFrame(stmt.name, structFrame);
        }

        private void CompileExpr(Expr expr, ref Frame frame, ObjectState state = ObjectState.READ)
        {
            switch (expr.Type())
            {
                case AstType.NUM:
                    CompileNumExpr((NumExpr)expr, ref frame);
                    break;
                case AstType.STR:
                    CompileStrExpr((StrExpr)expr, ref frame);
                    break;
                case AstType.BOOL:
                    CompileBoolExpr((BoolExpr)expr, ref frame);
                    break;
                case AstType.NIL:
                    CompileNilExpr((NilExpr)expr, ref frame);
                    break;
                case AstType.IDENTIFIER:
                    CompileIdentifierExpr((IdentifierExpr)expr, ref frame, state);
                    break;
                case AstType.GROUP:
                    CompileGroupExpr((GroupExpr)expr, ref frame);
                    break;
                case AstType.ARRAY:
                    CompileArrayExpr((ArrayExpr)expr, ref frame);
                    break;
                case AstType.INDEX:
                    CompileIndexExpr((IndexExpr)expr, ref frame, state);
                    break;
                case AstType.PREFIX:
                    CompilePrefixExpr((PrefixExpr)expr, ref frame);
                    break;
                case AstType.INFIX:
                    CompileInfixExpr((InfixExpr)expr, ref frame);
                    break;
                case AstType.FUNCTION_CALL:
                    CompileFunctionCallExpr((FunctionCallExpr)expr, ref frame);
                    break;
                case AstType.STRUCT_CALL:
                    CompileStructCallExpr((StructCallExpr)expr, ref frame, state);
                    break;
                case AstType.REF:
                    CompileRefExpr((RefExpr)expr, ref frame);
                    break;
                case AstType.LAMBDA:
                    CompileLambdaExpr((LambdaExpr)expr, ref frame);
                    break;
                default:
                    break;
            }
        }
        private void CompileNumExpr(NumExpr expr, ref Frame frame)
        {
            frame.AddOpCode((int)OpCode.OP_NEW_NUM);
            int offset = frame.AddNum(expr.value);
            frame.AddOpCode(offset);
        }
        private void CompileStrExpr(StrExpr expr, ref Frame frame)
        {
            frame.AddOpCode((int)OpCode.OP_NEW_STR);
            int offset = frame.AddString(expr.value);
            frame.AddOpCode(offset);
        }
        private void CompileBoolExpr(BoolExpr expr, ref Frame frame)
        {
            if (expr.value)
                frame.AddOpCode((int)OpCode.OP_NEW_TRUE);
            else
                frame.AddOpCode((int)OpCode.OP_NEW_FALSE);
        }
        private void CompileNilExpr(NilExpr expr, ref Frame frame)
        {
            frame.AddOpCode((int)OpCode.OP_NEW_NIL);
        }
        private void CompileIdentifierExpr(IdentifierExpr expr, ref Frame frame, ObjectState state = ObjectState.READ)
        {
            if (state == ObjectState.READ)
                frame.AddOpCode((int)OpCode.OP_GET_VAR);
            else if (state == ObjectState.WRITE)
                frame.AddOpCode((int)OpCode.OP_SET_VAR);
            else if (state == ObjectState.INIT)
                frame.AddOpCode((int)OpCode.OP_DEFINE_VAR);
            else if (state == ObjectState.STRUCT_READ)
                frame.AddOpCode((int)OpCode.OP_GET_STRUCT_VAR);
            else if (state == ObjectState.STRUCT_WRITE)
                frame.AddOpCode((int)OpCode.OP_SET_STRUCT_VAR);
            int offset = frame.AddString(expr.literal);
            frame.AddOpCode(offset);
        }
        private void CompileGroupExpr(GroupExpr expr, ref Frame frame)
        {
            CompileExpr(expr.expr, ref frame);
        }
        private void CompileArrayExpr(ArrayExpr expr, ref Frame frame)
        {
            foreach (var e in expr.elements)
                CompileExpr(e, ref frame);

            frame.AddOpCode((int)OpCode.OP_NEW_ARRAY);
            int offset = frame.AddNum(expr.elements.Count);
            frame.AddOpCode(offset);
        }
        private void CompileIndexExpr(IndexExpr expr, ref Frame frame, ObjectState state = ObjectState.READ)
        {
            CompileExpr(expr.ds, ref frame);
            CompileExpr(expr.index, ref frame);
            if (state == ObjectState.READ)
                frame.AddOpCode((int)OpCode.OP_GET_INDEX_VAR);
            else if (state == ObjectState.WRITE)
                frame.AddOpCode((int)OpCode.OP_SET_INDEX_VAR);
        }
        private void CompileRefExpr(RefExpr expr, ref Frame frame)
        {
            frame.AddOpCode((int)OpCode.OP_REF);
            int offset = frame.AddString(expr.refExpr.literal);
            frame.AddOpCode(offset);
        }
        private void CompileLambdaExpr(LambdaExpr expr, ref Frame frame)
        {
            Frame lambdaFrame = new Frame();

            lambdaFrame.AddOpCode((int)OpCode.OP_ENTER_SCOPE);

            for (int i = expr.parameters.Count - 1; i >= 0; --i)
                CompileIdentifierExpr(expr.parameters[i], ref lambdaFrame, ObjectState.INIT);

            foreach (var s in expr.body.stmts)
                CompileStmt(s, ref lambdaFrame);

            lambdaFrame.AddOpCode((int)OpCode.OP_EXIT_SCOPE);

            frame.AddOpCode((int)OpCode.OP_NEW_LAMBDA);
            int offset = frame.AddNum(frame.AddLambdaFrame(lambdaFrame));
            frame.AddOpCode(offset);
        }
        private void CompilePrefixExpr(PrefixExpr expr, ref Frame frame)
        {
            CompileExpr(expr.right, ref frame);
            if (expr.op == "-")
                frame.AddOpCode((int)OpCode.OP_NEG);
            else if (expr.op == "not")
                frame.AddOpCode((int)OpCode.OP_NOT);
        }
        private void CompileInfixExpr(InfixExpr expr, ref Frame frame)
        {
            if (expr.op == "=")
            {
                CompileExpr(expr.right, ref frame);
                CompileExpr(expr.left, ref frame, ObjectState.WRITE);
            }
            else
            {
                CompileExpr(expr.right, ref frame);
                CompileExpr(expr.left, ref frame);

                if (expr.op == "+")
                    frame.AddOpCode((int)OpCode.OP_ADD);
                else if (expr.op == "-")
                    frame.AddOpCode((int)OpCode.OP_SUB);
                else if (expr.op == "*")
                    frame.AddOpCode((int)OpCode.OP_MUL);
                else if (expr.op == "/")
                    frame.AddOpCode((int)OpCode.OP_DIV);
                else if (expr.op == "and")
                    frame.AddOpCode((int)OpCode.OP_AND);
                else if (expr.op == "or")
                    frame.AddOpCode((int)OpCode.OP_OR);
                else if (expr.op == ">")
                    frame.AddOpCode((int)OpCode.OP_GREATER);
                else if (expr.op == "<")
                    frame.AddOpCode((int)OpCode.OP_LESS);
                else if (expr.op == ">=")
                    frame.AddOpCode((int)OpCode.OP_GREATER_EQUAL);
                else if (expr.op == "<=")
                    frame.AddOpCode((int)OpCode.OP_LESS_EQUAL);
                else if (expr.op == "==")
                    frame.AddOpCode((int)OpCode.OP_EQUAL);
                else if (expr.op == "!=")
                    frame.AddOpCode((int)OpCode.OP_NOT_EQUAL);
                else
                    Utils.Assert("Unknown binary op:" + expr.op);
            }
        }
        private void CompileFunctionCallExpr(FunctionCallExpr expr, ref Frame frame)
        {

            foreach (var arg in expr.arguments)
                CompileExpr(arg, ref frame);


            //argument count
            frame.AddOpCode((int)OpCode.OP_NEW_NUM);
            int offset = frame.AddNum(expr.arguments.Count);
            frame.AddOpCode(offset);
            if (expr.name.Type() == AstType.IDENTIFIER)
            {
                frame.AddOpCode((int)OpCode.OP_FUNCTION_CALL);
                offset = frame.AddString(((IdentifierExpr)expr.name).literal);
                frame.AddOpCode(offset);
            }
            else if (expr.name.Type() == AstType.STRUCT_CALL)
            {
                var structCallExpr = ((StructCallExpr)expr.name);
                CompileExpr(structCallExpr.callee, ref frame);
                if (structCallExpr.callMember.Type() == AstType.STRUCT_CALL)
                    CompileExpr(((StructCallExpr)structCallExpr.callMember).callee, ref frame, ObjectState.STRUCT_READ);

                frame.AddOpCode((int)OpCode.OP_STRUCT_LAMBDA_CALL);
                offset = frame.AddString(((IdentifierExpr)structCallExpr.callMember).literal);
                frame.AddOpCode(offset);
            }
        }
        private void CompileStructCallExpr(StructCallExpr expr, ref Frame frame, ObjectState state = ObjectState.READ)
        {
            CompileExpr(expr.callee, ref frame);

            if (expr.callMember.Type() == AstType.STRUCT_CALL) //continuous struct call such as a.b.c;
                CompileExpr(((StructCallExpr)expr.callMember).callee, ref frame, ObjectState.STRUCT_READ);

            if (state == ObjectState.READ)
                CompileExpr(expr.callMember, ref frame, ObjectState.STRUCT_READ);
            else if (state == ObjectState.WRITE)
                CompileExpr(expr.callMember, ref frame, ObjectState.STRUCT_WRITE);
        }

        private Frame m_RootFrame;
    }
}