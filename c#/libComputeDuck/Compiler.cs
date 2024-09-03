using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
namespace ComputeDuck
{
    using OpCodes = List<int>;
    public enum RWState
    {

        READ,
        WRITE
    }

    public class Compiler
    {
        private List<Chunk> m_ScopeChunk;

        private SymbolTable m_SymbolTable;

        public Compiler()
        {
            ResetStatus();
        }

        public FunctionObject Compile(List<Stmt> stmts)
        {
            ResetStatus();

            foreach (var stmt in stmts)
                CompileStmt(stmt);

            return new FunctionObject(CurChunk());
        }

        public void ResetStatus()
        {
            m_ScopeChunk=new List<Chunk>();
            m_ScopeChunk.Add(new Chunk());

            m_SymbolTable = new SymbolTable();

            RegisterBuiltins();
        }


        void CompileStmt(Stmt stmt)
        {
            switch (stmt.type)
            {
                case AstType.RETURN:
                    CompileReturnStmt((ReturnStmt)stmt);
                    break;
                case AstType.EXPR:
                    CompileExprStmt((ExprStmt)stmt);
                    break;
                case AstType.SCOPE:
                    CompileScopeStmt((ScopeStmt)stmt);
                    break;
                case AstType.IF:
                    CompileIfStmt((IfStmt)stmt);
                    break;
                case AstType.WHILE:
                    CompileWhileStmt((WhileStmt)stmt);
                    break;
                case AstType.STRUCT:
                    CompileStructStmt((StructStmt)stmt);
                    break;
                default:
                    break;
            }
        }
        void CompileExprStmt(ExprStmt stmt)
        {
            CompileExpr(stmt.expr);
        }
        void CompileIfStmt(IfStmt stmt)
        {
            CompileExpr(stmt.condition);
            Emit((int)OpCode.OP_JUMP_IF_FALSE);

            var jumpIfFalseAddress = Emit(65536);

            CompileStmt(stmt.thenBranch);

            Emit((int)OpCode.OP_JUMP);
            var jumpAddress = Emit(65536);

            ModifyOpCode(jumpIfFalseAddress, CurChunk().opCodes.Count - 1);

            if (stmt.elseBranch != null)
                CompileStmt(stmt.elseBranch);

            ModifyOpCode(jumpAddress, CurChunk().opCodes.Count - 1);
        }
        void CompileScopeStmt(ScopeStmt stmt)
        {
            Emit((int)OpCode.OP_SP_OFFSET);

            var idx = Emit(0);

            foreach (var s in stmt.stmts)
                CompileStmt(s);

            var localVarCount = m_SymbolTable.definitionCount;
            CurChunk().opCodes[(int)idx] = localVarCount;

            Emit((int)OpCode.OP_SP_OFFSET);
            Emit(-localVarCount);
        }
        void CompileWhileStmt(WhileStmt stmt)
        {
            EnterScope()

            var jumpAddress = CurChunk().opCodes.Count - 1;
            CompileExpr(stmt.condition);

            Emit((int)OpCode.OP_JUMP_IF_FALSE);
            var jumpIfFalseAddress = Emit(65536);

            CompileStmt(stmt.body);

            Emit((int)OpCode.OP_JUMP);
            Emit(jumpAddress);

            ModifyOpCode(jumpIfFalseAddress, CurChunk().opCodes.Count - 1);

            ExitScope();
        }
        void CompileReturnStmt(ReturnStmt stmt)
        {
            if (stmt.expr != null)
            {
                CompileExpr(stmt.expr);
                Emit((int)OpCode.OP_RETURN);
                Emit(1);
            }
            else
            {
                Emit((int)OpCode.OP_RETURN);
                Emit(0);
            }
        }
        void CompileStructStmt(StructStmt stmt)
        {
            var symbol = m_SymbolTable.Define(stmt.name, true);

            m_ScopeChunk.Add(new Chunk());

            for (int i = stmt.members.Count - 1; i >= 0; --i)
            {
                CompileExpr(stmt.members[i].Value);
                EmitConstant(new StrObject(stmt.members[i].Key.literal));
            }

            var localVarCount = m_SymbolTable.definitionCount;

            Emit((int)OpCode.OP_STRUCT);
            Emit((int)stmt.members.Count);

            var chunk = m_ScopeChunk[m_ScopeChunk.Count - 1];
            m_ScopeChunk.RemoveAt(m_ScopeChunk.Count - 1);

            chunk.opCodes.Add((int)OpCode.OP_RETURN);
            chunk.opCodes.Add(1);

            var fn = new FunctionObject(chunk, localVarCount);

            EmitConstant(fn);

            StoreSymbol(symbol);
        }
        void CompileExpr(Expr expr, RWState state = RWState.READ)
        {
            switch (expr.type)
            {
                case AstType.NUM:
                    CompileNumExpr((NumExpr)expr);
                    break;
                case AstType.STR:
                    CompileStrExpr((StrExpr)expr);
                    break;
                case AstType.BOOL:
                    CompileBoolExpr((BoolExpr)expr);
                    break;
                case AstType.NIL:
                    CompileNilExpr((NilExpr)expr);
                    break;
                case AstType.IDENTIFIER:
                    CompileIdentifierExpr((IdentifierExpr)expr, state);
                    break;
                case AstType.GROUP:
                    CompileGroupExpr((GroupExpr)expr);
                    break;
                case AstType.ARRAY:
                    CompileArrayExpr((ArrayExpr)expr);
                    break;
                case AstType.INDEX:
                    CompileIndexExpr((IndexExpr)expr,state);
                    break;
                case AstType.PREFIX:
                    CompilePrefixExpr((PrefixExpr)expr);
                    break;
                case AstType.INFIX:
                    CompileInfixExpr((InfixExpr)expr);
                    break;
                case AstType.FUNCTION_CALL:
                    CompileFunctionCallExpr((FunctionCallExpr)expr);
                    break;
                case AstType.STRUCT_CALL:
                    CompileStructCallExpr((StructCallExpr)expr, state);
                    break;
                case AstType.REF:
                    CompileRefExpr((RefExpr)expr);
                    break;
                case AstType.FUNCTION:
                    CompileFunctionExpr((FunctionExpr)expr);
                    break;
                case AstType.STRUCT:
                    CompileStructExpr((StructExpr)expr);
                    break;
                case AstType.DLL_IMPORT:
                    CompileDllImportExpr((DllImportExpr)expr);
                    break;
                default:
                    break;
            }
        }
        void CompileInfixExpr(InfixExpr expr)
        {
            if (expr.op == "=")
            {
                if (expr.left.type == AstType.IDENTIFIER && expr.right.type == AstType.FUNCTION)
                    m_SymbolTable.Define(((IdentifierExpr)expr.left).literal);
                CompileExpr(expr.right);
                CompileExpr(expr.left, RWState.WRITE);
            }
            else
            {
                CompileExpr(expr.right);
                CompileExpr(expr.left);

                if (expr.op == "+")
                    Emit((int)OpCode.OP_ADD);
                else if (expr.op == "-")
                    Emit((int)OpCode.OP_SUB);
                else if (expr.op == "*")
                    Emit((int)OpCode.OP_MUL);
                else if (expr.op == "/")
                    Emit((int)OpCode.OP_DIV);
                else if (expr.op == ">")
                    Emit((int)OpCode.OP_GREATER);
                else if (expr.op == "<")
                    Emit((int)OpCode.OP_LESS);
                else if (expr.op == "&")
                    Emit((int)OpCode.OP_BIT_AND);
                else if (expr.op == "|")
                    Emit((int)OpCode.OP_BIT_OR);
                else if (expr.op == "^")
                    Emit((int)OpCode.OP_BIT_XOR);
                else if (expr.op == ">=")
                {
                    Emit((int)OpCode.OP_LESS);
                    Emit((int)OpCode.OP_NOT);
                }
                else if (expr.op == "<=")
                {
                    Emit((int)OpCode.OP_GREATER);
                    Emit((int)OpCode.OP_NOT);
                }
                else if (expr.op == "==")
                    Emit((int)OpCode.OP_EQUAL);
                else if (expr.op == "!=")
                {
                    Emit((int)OpCode.OP_EQUAL);
                    Emit((int)OpCode.OP_NOT);
                }
                else if (expr.op == "and")
                    Emit((int)OpCode.OP_AND);
                else if (expr.op == "or")
                    Emit((int)OpCode.OP_OR);
            }
        }
        void CompileNumExpr(NumExpr expr)
        {
            var obj = new NumObject(expr.value);
            EmitConstant(obj);
        }
        void CompileBoolExpr(BoolExpr expr)
        {
            if (expr.value)
                EmitConstant(new BoolObject(true));
            else
                EmitConstant(new BoolObject(false));
        }
        void CompilePrefixExpr(PrefixExpr expr)
        {
            CompileExpr(expr.right);
            if (expr.op == "-")
                Emit((int)OpCode.OP_MINUS);
            else if (expr.op == "not")
                Emit((int)OpCode.OP_NOT);
            else if (expr.op == "~")
                Emit((int)OpCode.OP_BIT_NOT);
            else
                Utils.Assert("Unrecognized prefix op");
        }
        void CompileStrExpr(StrExpr expr)
        {
            var obj = new StrObject(expr.value);
            EmitConstant(obj);
        }
        void CompileNilExpr(NilExpr expr)
        {
            EmitConstant(new NilObject());
        }
        void CompileGroupExpr(GroupExpr expr)
        {
            CompileExpr(expr.expr);
        }
        void CompileArrayExpr(ArrayExpr expr)
        {
            foreach (var e in expr.elements)
                CompileExpr(e);
            Emit((int)OpCode.OP_ARRAY);
            Emit(expr.elements.Count);
        }
        void CompileIndexExpr(IndexExpr expr,RWState state)
        {
            CompileExpr(expr.ds);
            CompileExpr(expr.index);
            if(state==RWState.WRITE)
                Emit((int)OpCode.OP_SET_INDEX);
            else
                Emit((int)OpCode.OP_GET_INDEX);
        }
        void CompileIdentifierExpr(IdentifierExpr expr, RWState state)
        {
            var (isFound,symbol) = m_SymbolTable.Resolve(expr.literal);
            if (state == RWState.READ)
            {
                if (!isFound)
                    Utils.Assert("Undefined variable:" + expr.Stringify());
                LoadSymbol(symbol);
            }
            else
            {
                if (!isFound)
                    symbol = m_SymbolTable.Define(expr.literal);
                StoreSymbol(symbol);
            }
        }
        void CompileFunctionExpr(FunctionExpr expr)
        {
            EnterScope();
            m_ScopeChunk.Add(new Chunk());

            foreach (var param in expr.parameters)
                m_SymbolTable.Define(param.literal);

            foreach (var s in expr.body.stmts)
                CompileStmt(s);

            var localVarCount = m_SymbolTable.definitionCount;

            ExitScope();

            var chunk = m_ScopeChunk[m_ScopeChunk.Count - 1];
            m_ScopeChunk.RemoveAt(m_ScopeChunk.Count - 1);

            if (chunk.opCodes.Count == 0 || chunk.opCodes[chunk.opCodes.Count - 2] != (int)OpCode.OP_RETURN)
            {
                chunk.opCodes.Add((int)OpCode.OP_RETURN);
                chunk.opCodes.Add(0);
            }

            var fn = new FunctionObject(chunk, localVarCount, expr.parameters.Count);
            EmitConstant(fn);
        }
        void CompileFunctionCallExpr(FunctionCallExpr expr)
        {
            CompileExpr(expr.name);

            foreach (var argu in expr.arguments)
                CompileExpr(argu);

            Emit((int)OpCode.OP_FUNCTION_CALL);
            Emit((int)expr.arguments.Count);
        }
        void CompileStructCallExpr(StructCallExpr expr, RWState state = RWState.READ)
        {
            if (expr.callMember.type == AstType.FUNCTION_CALL && state == RWState.WRITE)
                Utils.Assert("Cannot assign to a struct's function call expr.");
            CompileExpr(expr.callee);

            if (expr.callMember.type == AstType.IDENTIFIER)
                EmitConstant(new StrObject(((IdentifierExpr)expr.callMember).literal));
            else if (expr.callMember.type == AstType.FUNCTION_CALL)
                EmitConstant(new StrObject(((IdentifierExpr)expr.callMember).literal));

            if (state == RWState.READ)
            {
                Emit((int)OpCode.OP_GET_STRUCT);

                if (expr.callMember.type == AstType.FUNCTION_CALL)
                {
                    var funcCall = (FunctionCallExpr)expr.callMember;
                    foreach (var argu in funcCall.arguments)
                        CompileExpr(argu);
                    Emit((int)OpCode.OP_FUNCTION_CALL);
                    Emit((int)funcCall.arguments.Count);
                }
            }
            else
                Emit((int)OpCode.OP_SET_STRUCT);
        }
        void CompileRefExpr(RefExpr expr)
        {
            if (expr.refExpr.type == AstType.INDEX)
            {
                CompileExpr(((IndexExpr)expr.refExpr).index);
                var (isFound,symbol) = m_SymbolTable.Resolve(((IndexExpr)expr.refExpr).ds.Stringify());

                if (!isFound)
                    Utils.Assert("Undefined variable:" + expr.Stringify());

                switch (symbol?.scope)
                {
                    case SymbolScope.GLOBAL:
                        Emit((int)OpCode.OP_REF_INDEX_GLOBAL);
                        Emit(symbol.index);
                        break;
                    case SymbolScope.LOCAL:
                        Emit((int)OpCode.OP_REF_INDEX_LOCAL);
                        Emit(symbol.scopeDepth);
                        Emit(symbol.index);
                        Emit(symbol.isUpValue);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                var (isFound, symbol) = m_SymbolTable.Resolve(expr.refExpr.Stringify());
                if (!isFound)
                    Utils.Assert("Undefined variable:" + expr.Stringify());

                switch (symbol?.scope)
                {
                    case SymbolScope.GLOBAL:
                        Emit((int)OpCode.OP_REF_GLOBAL);
                        Emit(symbol.index);
                        break;
                    case SymbolScope.LOCAL:
                        Emit((int)OpCode.OP_REF_LOCAL);
                        Emit(symbol.scopeDepth);
                        Emit(symbol.index);
                        Emit(symbol.isUpValue);
                        break;
                    default:
                        break;
                }
            }
        }
        void CompileStructExpr(StructExpr expr)
        {
            foreach (var memberPair in expr.memberPairs)
            {
                CompileExpr(memberPair.Value);
                EmitConstant(new StrObject(memberPair.Key.literal));
            }

            Emit((int)OpCode.OP_STRUCT);
            Emit(expr.memberPairs.Count);
        }
        void CompileDllImportExpr(DllImportExpr expr)
        {
            var dllPath = expr.dllPath;

            Utils.RegisterDLLs(dllPath);

            EmitConstant(new StrObject(dllPath));
            Emit((int)OpCode.OP_DLL_IMPORT);

            RegisterBuiltins();
        }

        void EnterScope()
        {
            m_SymbolTable = new SymbolTable(m_SymbolTable);
        }

        void ExitScope()
        {
            m_SymbolTable = m_SymbolTable.enclosing;
        }

        Chunk CurChunk()
        {
            return m_ScopeChunk[m_ScopeChunk.Count - 1];
        }

        uint Emit(int opcode)
        {
            CurChunk().opCodes.Add(opcode);
            return (uint)CurChunk().opCodes.Count - 1;
        }
        uint EmitConstant(Object obj)
        {
            CurChunk().constants.Add(obj);
            var pos= (uint)CurChunk().constants.Count - 1;
            Emit((int)OpCode.OP_CONSTANT);
            Emit((int)pos);
            return (uint)CurChunk().opCodes.Count - 1;
        }

        void ModifyOpCode(uint pos, int opcode)
        {
            CurChunk().opCodes[(int)pos] = opcode;
        }

        void RegisterBuiltins()
        {
            foreach (var k in BuiltinManager.GetInstance().m_Builtins.Keys)
                m_SymbolTable.DefineBuiltin(k);
        }
        
        void LoadSymbol(Symbol symbol)
        {
            switch (symbol.scope)
            {
                case SymbolScope.GLOBAL:
                    Emit((int)OpCode.OP_GET_GLOBAL);
                    Emit(symbol.index);
                    break;
                case SymbolScope.LOCAL:
                    Emit((int)OpCode.OP_GET_LOCAL);
                    Emit(symbol.scopeDepth);
                    Emit(symbol.index);
                    Emit(symbol.isUpValue);
                    break;
                case SymbolScope.BUILTIN:
                    EmitConstant(new StrObject(symbol.name));
                    Emit((int)OpCode.OP_GET_BUILTIN);
                    break;
                default:
                    break;
            }

            if (symbol.isStructSymbol)
            {
                Emit((int)OpCode.OP_FUNCTION_CALL);
                Emit(0);
            }
        }

        void StoreSymbol(Symbol symbol)
        {
            switch (symbol.scope)
            {
                case SymbolScope.GLOBAL:
                    Emit((int)OpCode.OP_SET_GLOBAL);
                    Emit(symbol.index);
                    break;
                case SymbolScope.LOCAL:
                    Emit((int)OpCode.OP_SET_LOCAL);
                    Emit(symbol.scopeDepth);
                    Emit(symbol.index);
                    Emit(symbol.isUpValue);
                    break;
                default:
                    break;
            }
        }
    }
}