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
        private List<Object> m_Constants;

        private List<OpCodes> m_Scopes;

        private SymbolTable m_SymbolTable;

        private List<string> m_BuiltinFunctionNames;
        private List<string> m_BuiltinVariableNames;

        public Compiler()
        {
            ResetStatus();
        }

        public Chunk Compile(List<Stmt> stmts)
        {
            foreach (var stmt in stmts)
                CompileStmt(stmt);
            Emit((int)OpCode.OP_RETURN);
            Emit(0);

            return new Chunk(CurOpCodes(), m_Constants);
        }


        public void ResetStatus()
        {
            m_Constants = new List<Object>();
            m_Scopes = new List<OpCodes>();
            m_Scopes.Add(new OpCodes());

            m_SymbolTable = new SymbolTable();

            m_BuiltinFunctionNames = new List<string>();
            m_BuiltinVariableNames = new List<string>();

            for (int i = 0; i < BuiltinManager.GetInstance().m_BuiltinFunctionNames.Count; ++i)
            {
                m_BuiltinFunctionNames.Add(BuiltinManager.GetInstance().m_BuiltinFunctionNames[i]);
                m_SymbolTable.DefineBuiltinFunction(BuiltinManager.GetInstance().m_BuiltinFunctionNames[i], i);
            }

            for (int i = 0; i < BuiltinManager.GetInstance().m_BuiltinVariableNames.Count; ++i)
            {
                m_BuiltinVariableNames.Add(BuiltinManager.GetInstance().m_BuiltinVariableNames[i]);
                m_SymbolTable.DefineBuiltinVariable(BuiltinManager.GetInstance().m_BuiltinVariableNames[i], i);
            }
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

            ModifyOpCode(jumpIfFalseAddress, CurOpCodes().Count - 1);

            if (stmt.elseBranch != null)
                CompileStmt(stmt.elseBranch);

            ModifyOpCode(jumpAddress, CurOpCodes().Count - 1);
        }
        void CompileScopeStmt(ScopeStmt stmt)
        {
            EnterScope();
            Emit((int)OpCode.OP_SP_OFFSET);

            var idx = Emit(0);

            foreach (var s in stmt.stmts)
                CompileStmt(s);

            var localVarCount = m_SymbolTable.definitionCount;
            CurOpCodes()[(int)idx] = localVarCount;

            Emit((int)OpCode.OP_SP_OFFSET);
            Emit(-localVarCount);

            ExitScope();
        }
        void CompileWhileStmt(WhileStmt stmt)
        {
            var jumpAddress = CurOpCodes().Count - 1;
            CompileExpr(stmt.condition);

            Emit((int)OpCode.OP_JUMP_IF_FALSE);
            var jumpIfFalseAddress = Emit(65536);

            CompileStmt(stmt.body);

            Emit((int)OpCode.OP_JUMP);
            Emit(jumpAddress);

            ModifyOpCode(jumpIfFalseAddress, CurOpCodes().Count - 1);
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
            EnterScope();

            m_Scopes.Add(new OpCodes());

            for (int i = stmt.members.Count - 1; i >= 0; --i)
            {
                CompileExpr(stmt.members[i].Value);
                EmitConstant(AddConstant(new StrObject(stmt.members[i].Key.literal)));
            }

            var localVarCount = m_SymbolTable.definitionCount;

            Emit((int)OpCode.OP_STRUCT);
            Emit((int)stmt.members.Count);

            ExitScope();

            var opCodes = m_Scopes[m_Scopes.Count - 1];
            m_Scopes.RemoveAt(m_Scopes.Count - 1);

            opCodes.Add((int)OpCode.OP_RETURN);
            opCodes.Add(1);

            var fn = new FunctionObject(opCodes, localVarCount);

            EmitConstant(AddConstant(fn));

            if (symbol.scope == SymbolScope.GLOBAL)
                Emit((int)OpCode.OP_SET_GLOBAL);
            else
            {
                Emit((int)OpCode.OP_SET_LOCAL);
                Emit(symbol.isInUpScope);
                Emit(symbol.scopeDepth);
            }
            Emit(symbol.index);
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
                    CompileIndexExpr((IndexExpr)expr);
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
                case AstType.ANONY_STRUCT:
                    CompileAnonyStructExpr((AnonyStructExpr)expr);
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
            var pos = AddConstant(obj);
            EmitConstant(pos);
        }
        void CompileBoolExpr(BoolExpr expr)
        {
            if (expr.value)
                EmitConstant(AddConstant(new BoolObject(true)));
            else
                EmitConstant(AddConstant(new BoolObject(false)));
        }
        void CompilePrefixExpr(PrefixExpr expr)
        {
            CompileExpr(expr.right);
            if (expr.op == "-")
                Emit((int)OpCode.OP_MINUS);
            else if (expr.op == "not")
                Emit((int)OpCode.OP_NOT);
            else
                Utils.Assert("Unrecognized prefix op");
        }
        void CompileStrExpr(StrExpr expr)
        {
            var obj = new StrObject(expr.value);
            var pos = AddConstant(obj);
            EmitConstant(pos);
        }
        void CompileNilExpr(NilExpr expr)
        {
            EmitConstant(AddConstant(new NilObject()));
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
        void CompileIndexExpr(IndexExpr expr)
        {
            CompileExpr(expr.ds);
            CompileExpr(expr.index);
            Emit((int)OpCode.OP_INDEX);
        }
        void CompileIdentifierExpr(IdentifierExpr expr, RWState state)
        {
            Symbol symbol = new Symbol();
            bool isFound = m_SymbolTable.Resolve(expr.literal, out symbol);
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
                switch (symbol.scope)
                {
                    case SymbolScope.GLOBAL:
                        Emit((int)OpCode.OP_SET_GLOBAL);
                        Emit(symbol.index);
                        break;
                    case SymbolScope.LOCAL:
                        Emit((int)OpCode.OP_SET_LOCAL);
                        Emit(symbol.isInUpScope);
                        Emit(symbol.scopeDepth);
                        Emit(symbol.index);
                        break;
                    default:
                        break;
                }
            }
        }
        void CompileFunctionExpr(FunctionExpr expr)
        {
            EnterScope();
            m_Scopes.Add(new OpCodes());

            foreach (var param in expr.parameters)
                m_SymbolTable.Define(param.literal);

            foreach (var s in expr.body.stmts)
                CompileStmt(s);

            var localVarCount = m_SymbolTable.definitionCount;

            ExitScope();

            var opCodes = m_Scopes[m_Scopes.Count - 1];
            m_Scopes.RemoveAt(m_Scopes.Count - 1);

            if (opCodes.Count == 0 || opCodes[opCodes.Count - 2] != (int)OpCode.OP_RETURN)
            {
                opCodes.Add((int)OpCode.OP_RETURN);
                opCodes.Add(0);
            }

            var fn = new FunctionObject(opCodes, localVarCount, expr.parameters.Count);

            EmitConstant(AddConstant(fn));
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
                EmitConstant(AddConstant(new StrObject(((IdentifierExpr)expr.callMember).literal)));
            else if (expr.callMember.type == AstType.FUNCTION_CALL)
                EmitConstant(AddConstant(new StrObject(((IdentifierExpr)expr.callMember).literal)));

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
            Symbol symbol = new Symbol();

            if (expr.refExpr.type == AstType.INDEX)
            {
                CompileExpr(((IndexExpr)expr.refExpr).index);
                bool isFound = m_SymbolTable.Resolve(((IndexExpr)expr.refExpr).ds.Stringify(), out symbol);

                if (!isFound)
                    Utils.Assert("Undefined variable:" + expr.Stringify());

                switch (symbol.scope)
                {
                    case SymbolScope.GLOBAL:
                        Emit((int)OpCode.OP_REF_INDEX_GLOBAL);
                        Emit(symbol.index);
                        break;
                    case SymbolScope.LOCAL:
                        Emit((int)OpCode.OP_REF_INDEX_LOCAL);
                        Emit(symbol.isInUpScope);
                        Emit(symbol.scopeDepth);
                        Emit(symbol.index);
                        break;
                    default:
                        break;
                }
            }
            else
            {
                bool isFound = m_SymbolTable.Resolve(expr.refExpr.Stringify(), out symbol);
                if (!isFound)
                    Utils.Assert("Undefined variable:" + expr.Stringify());

                switch (symbol.scope)
                {
                    case SymbolScope.GLOBAL:
                        Emit((int)OpCode.OP_REF_GLOBAL);
                        Emit(symbol.index);
                        break;
                    case SymbolScope.LOCAL:
                        Emit((int)OpCode.OP_REF_LOCAL);
                        Emit(symbol.isInUpScope);
                        Emit(symbol.scopeDepth);
                        Emit(symbol.index);
                        break;
                    default:
                        break;
                }
            }
        }
        void CompileAnonyStructExpr(AnonyStructExpr expr)
        {
            EnterScope();

            foreach (var memberPair in expr.memberPairs)
            {
                CompileExpr(memberPair.Value);
                EmitConstant(AddConstant(new StrObject(memberPair.Key.literal)));
            }

            Emit((int)OpCode.OP_STRUCT);
            Emit(expr.memberPairs.Count);

            ExitScope();
        }
        void CompileDllImportExpr(DllImportExpr expr)
        {
            var dllPath = expr.dllPath;

            try
            {
                var loc = this.GetType().Assembly.Location;
                if (loc.Contains('\\'))//windows
                    loc = loc.Substring(0, loc.LastIndexOf('\\')+1);
                else
                    loc = loc.Substring(0, loc.LastIndexOf('/')+1);

                var fullPath = loc + dllPath;

                Assembly asm = Assembly.LoadFrom(fullPath);
                string str = asm.GetName().ToString();
                var className = str.Split(",")[0];
                className = className.Remove(0, 8);//remove prefix "library-";
               
                AssemblyName[] names=asm.GetReferencedAssemblies();
                Utils.LoadAssembly(names,loc);
               
                Type type = asm.GetType(className);
                MethodInfo meth = type.GetMethod("RegisterBuiltins");
                meth.Invoke(null, null);

                List<string> newAddedBuiltinFunctionNames = new List<string>();
                List<string> newAddedBuiltinVariableNames = new List<string>();

                foreach (var name in BuiltinManager.GetInstance().m_BuiltinFunctionNames)
                {
                    bool found = false;
                    foreach (var recName in m_BuiltinFunctionNames)
                    {
                        if (name == recName)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found == false)
                        newAddedBuiltinFunctionNames.Add(name);
                }

                foreach (var name in BuiltinManager.GetInstance().m_BuiltinVariableNames)
                {
                    bool found = false;
                    foreach (var recName in m_BuiltinVariableNames)
                    {
                        if (name == recName)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found == false)
                        newAddedBuiltinVariableNames.Add(name);
                }

                var legacyBuiltinFuncCount = m_BuiltinFunctionNames.Count;
                for (int i = 0; i < newAddedBuiltinFunctionNames.Count; ++i)
                {
                    m_BuiltinFunctionNames.Add(newAddedBuiltinFunctionNames[i]);
                    m_SymbolTable.DefineBuiltinFunction(newAddedBuiltinFunctionNames[i], legacyBuiltinFuncCount + i);
                }

                var legacyVariableCount = m_BuiltinVariableNames.Count;
                for (int i = 0; i < newAddedBuiltinVariableNames.Count; ++i)
                {
                    m_BuiltinFunctionNames.Add(newAddedBuiltinVariableNames[i]);
                    m_SymbolTable.DefineBuiltinFunction(newAddedBuiltinVariableNames[i], legacyVariableCount + i);
                }
            }
            catch (FileNotFoundException e)
            {
                Utils.Assert("Failed to load dll library:" + dllPath);
            }
        }

        void EnterScope()
        {
            m_SymbolTable = new SymbolTable(m_SymbolTable);
        }
        void ExitScope()
        {
            m_SymbolTable = m_SymbolTable.enclosing;
        }

        OpCodes CurOpCodes()
        {
            return m_Scopes[m_Scopes.Count - 1];
        }

        uint Emit(int opcode)
        {
            CurOpCodes().Add(opcode);
            return (uint)CurOpCodes().Count - 1;
        }
        uint EmitConstant(uint pos)
        {
            Emit((int)OpCode.OP_CONSTANT);
            Emit((int)pos);
            return (uint)CurOpCodes().Count - 1;
        }

        void ModifyOpCode(uint pos, int opcode)
        {
            CurOpCodes()[(int)pos] = opcode;
        }

        uint AddConstant(Object obj)
        {
            m_Constants.Add(obj);
            return (uint)m_Constants.Count - 1;
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
                    Emit(symbol.isInUpScope);
                    Emit(symbol.scopeDepth);
                    Emit(symbol.index);
                    break;
                case SymbolScope.BUILTIN_FUNCTION:
                    Emit((int)OpCode.OP_GET_BUILTIN_FUNCTION);
                    Emit(symbol.index);
                    break;
                case SymbolScope.BUILTIN_VARIABLE:
                    Emit((int)OpCode.OP_GET_BUILTIN_VARIABLE);
                    Emit(symbol.index);
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
    }
}