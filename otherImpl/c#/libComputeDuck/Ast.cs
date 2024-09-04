using System.Collections.Generic;

namespace ComputeDuck
{
    public enum AstType
    {
        NUM,
        STR,
        NIL,
        BOOL,
        IDENTIFIER,
        GROUP,
        ARRAY,
        PREFIX,
        INFIX,
        INDEX,
        REF,
        FUNCTION,
        FUNCTION_CALL,
        STRUCT_CALL,
        DLL_IMPORT,
        
        EXPR,
        RETURN,
        IF,
        SCOPE,
        WHILE,

        STRUCT,
    }


    public abstract class AstNode
    {
        public AstNode(AstType type)
        {
            this.type = type;
        }

        public abstract string Stringify();

        public AstType type;
    }

    public abstract class Expr : AstNode
    {
        public Expr(AstType type)
         : base(type)
        {
        }
    }

    public class NumExpr : Expr
    {
        public NumExpr()
        : base(AstType.NUM)
        {
            this.value = 0.0;
        }

        public NumExpr(double value)
        : base(AstType.NUM)
        {
            this.value = value;
        }
        public override string Stringify() { return value.ToString(); }
        public double value;
    }

    public class StrExpr : Expr
    {
        public StrExpr()
         : base(AstType.STR)
        {
            this.value = "";
        }
        public StrExpr(string value)
         : base(AstType.STR)
        {
            this.value = value;
        }
        public override string Stringify() { return "\"" + value + "\""; }
        public string value;
    }

    public class NilExpr : Expr
    {
        public NilExpr()
        : base(AstType.NIL)
        { }
        public override string Stringify() { return "nil"; }
    }

    public class BoolExpr : Expr
    {
        public BoolExpr()
        : base(AstType.BOOL)
        {
            this.value = false;
        }
        public BoolExpr(bool value)
         : base(AstType.BOOL)
        {
            this.value = value;
        }

        public override string Stringify() { return value ? "true" : "false"; }
        public bool value;
    }

    public class IdentifierExpr : Expr
    {
        public IdentifierExpr()
        : base(AstType.IDENTIFIER)
        {
            this.literal = "";
        }
        public IdentifierExpr(string literal)
        : base(AstType.IDENTIFIER)
        {
            this.literal = literal;
        }
        public override string Stringify() { return literal; }
        public string literal;
    }

    public class ArrayExpr : Expr
    {
        public ArrayExpr()
        : base(AstType.ARRAY)
        {
            elements = new List<Expr>();
        }
        public ArrayExpr(List<Expr> elements)
        : base(AstType.ARRAY)
        {
            this.elements = elements;
        }
        public override string Stringify()
        {
            string result = "[";
            if (!(elements.Count == 0))
            {
                foreach (var e in elements)
                    result += e.Stringify() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += "]";
            return result;
        }

        public List<Expr> elements;
    }

    public class GroupExpr : Expr
    {
        public GroupExpr()
        : base(AstType.GROUP)
        {
            expr = null;
        }
        public GroupExpr(Expr expr)
        : base(AstType.GROUP)
        {
            this.expr = expr;
        }
        public override string Stringify() { return "(" + expr?.Stringify() + ")"; }
        public Expr? expr;
    }

    public class PrefixExpr : Expr
    {
        public PrefixExpr()
        : base(AstType.PREFIX)
        {
            this.op = ""; this.right = null;
        }
        public PrefixExpr(string op, Expr right)
        : base(AstType.PREFIX)
        {
            this.op = op; this.right = right;
        }

        public override string Stringify() { return op + right?.Stringify(); }
        public string op;
        public Expr? right;
    }

    public class InfixExpr : Expr
    {
        public InfixExpr()
        : base(AstType.INFIX)
        {
            this.op = "";
            this.left = null;
            this.right = null;
        }
        public InfixExpr(string op, Expr left, Expr right)
        : base(AstType.INFIX)
        {
            this.op = op;
            this.left = left;
            this.right = right;
        }

        public override string Stringify() { return this.left?.Stringify() + op + right?.Stringify(); }
        public string op;
        public Expr? left;
        public Expr? right;
    }

    public class IndexExpr : Expr
    {
        public IndexExpr()
        : base(AstType.INDEX)
        {
            this.ds = null;
            this.index = null;
        }
        public IndexExpr(Expr ds, Expr index)
        : base(AstType.INDEX)
        {
            this.ds = ds; this.index = index;
        }
        public override string Stringify() { return this.ds?.Stringify() + "[" + this.index?.Stringify() + "]"; }
        public Expr? ds;
        public Expr? index;
    }

    public class RefExpr : Expr
    {
        public RefExpr()
        : base(AstType.REF)
        {
            this.refExpr = null;
        }
        public RefExpr(Expr refExpr)
        : base(AstType.REF)
        {
            this.refExpr = refExpr;
        }
        public override string Stringify() { return "ref " + refExpr?.Stringify(); }
        public Expr? refExpr;
    }

    public class FunctionCallExpr : Expr
    {

        public FunctionCallExpr()
        : base(AstType.FUNCTION_CALL)
        {
            this.name = null;
            this.arguments = new List<Expr?>();
        }
        public FunctionCallExpr(Expr name, List<Expr?> arguments)
        : base(AstType.FUNCTION_CALL)
        {
            this.name = name;
            this.arguments = arguments;
        }

        public override string Stringify()
        {
            string result = name?.Stringify() + "(";

            if (arguments.Count != 0)
            {
                foreach (var arg in arguments)
                    result += arg?.Stringify() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += ")";
            return result;
        }

        public Expr? name;
        public List<Expr?> arguments;

    }

    public class StructCallExpr : Expr
    {
        public StructCallExpr()
        : base(AstType.STRUCT_CALL)
        {
            this.callee = null;
            this.callMember = null;
        }
        public StructCallExpr(Expr callee, Expr callmember)
        : base(AstType.STRUCT_CALL)
        {
            this.callee = callee;
            this.callMember = callmember;
        }

        public override string Stringify() { return callee?.Stringify() + "." + callMember?.Stringify(); }

        public Expr? callee;
        public Expr? callMember;
    }

    public class DllImportExpr : Expr
    {
        public DllImportExpr()
        : base(AstType.DLL_IMPORT)
        {
            this.dllPath = "";
        }

        public DllImportExpr(string dllPath)
       : base(AstType.DLL_IMPORT)
        {
            this.dllPath = dllPath;
        }

        public override string Stringify()
        {
            return "dllimport(\"" + dllPath + "\")";
        }
        public string dllPath;
    }

    public abstract class Stmt : AstNode
    {
        public Stmt(AstType type)
        : base(type)
        {
        }
    }

    public class ExprStmt : Stmt
    {
        public ExprStmt()
        : base(AstType.EXPR)
        {
            this.expr = null;
        }
        public ExprStmt(Expr expr)
         : base(AstType.EXPR)
        {
            this.expr = expr;
        }

        public override string Stringify() { return expr?.Stringify() + ";"; }

        public Expr? expr;
    }

    public class ReturnStmt : Stmt
    {
        public ReturnStmt()
         : base(AstType.RETURN)
        {
            this.expr = null;
        }
        public ReturnStmt(Expr expr) : base(AstType.RETURN)
        {
            this.expr = expr;
        }

        public override string Stringify() { return "return " + expr?.Stringify() + ";"; }
        public Expr? expr;
    }

    public class IfStmt : Stmt
    {
        public IfStmt()
        : base(AstType.IF)
        {
            this.condition = null;
            this.thenBranch = null;
            this.elseBranch = null;
        }
        public IfStmt(Expr condition, Stmt thenBranch, Stmt elseBranch)
        : base(AstType.IF)
        {
            this.condition = condition;
            this.thenBranch = thenBranch;
            this.elseBranch = elseBranch;
        }

        public override string Stringify()
        {
            string result = "if(" + condition?.Stringify() + ")" + thenBranch?.Stringify();
            if (elseBranch != null)
                result += "else " + elseBranch?.Stringify();
            return result;
        }
        public Expr? condition;
        public Stmt? thenBranch;
        public Stmt? elseBranch;
    }

    public class ScopeStmt : Stmt
    {
        public ScopeStmt()
        : base(AstType.SCOPE)
        {
            this.stmts = new List<Stmt>();
        }
        public ScopeStmt(List<Stmt> stmts)
         : base(AstType.SCOPE)
        {
            this.stmts = stmts;
        }

        public override string Stringify()
        {
            string result = "{";
            foreach (var stmt in stmts)
                result += stmt.Stringify();
            return result + "}";
        }

        public List<Stmt> stmts;
    }

    public class FunctionExpr : Expr
    {
        public FunctionExpr() : base(AstType.FUNCTION)
        {
            this.parameters = new List<IdentifierExpr>();
            this.body = null;
        }
        public FunctionExpr(List<IdentifierExpr> parameters, ScopeStmt body) : base(AstType.FUNCTION)
        {
            this.parameters = parameters;
            this.body = body;
        }


        public override string Stringify()
        {
            string result = "function(";
            if (parameters.Count != 0)
            {
                foreach (var param in parameters)
                    result += param.Stringify() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += ")" + body?.Stringify();
            return result + "}";
        }

        public List<IdentifierExpr> parameters;
        public ScopeStmt? body;
    }

    public class StructExpr : Expr
    {
        public StructExpr()
        : base(AstType.STRUCT)
        {
            memberPairs = new List<KeyValuePair<IdentifierExpr, Expr?>>();
        }

        public StructExpr(List<KeyValuePair<IdentifierExpr, Expr?>> memberPairs)
       : base(AstType.STRUCT)
        {
            this.memberPairs = memberPairs;
        }


        public override string Stringify()
        {
            string result = "{";
            foreach (var member in memberPairs)
                result += member.Key.Stringify() + ":" + member.Value?.Stringify() + ",\n";
            result += "}";
            return result;
        }

        public List<KeyValuePair<IdentifierExpr, Expr?>> memberPairs;
    }

    public class WhileStmt : Stmt
    {
        public WhileStmt()
        : base(AstType.WHILE)
        {

        }
        public WhileStmt(Expr condition, Stmt body)
            : base(AstType.WHILE)
        {
            this.condition = condition; this.body = body;
        }
        public override string Stringify() { return "while(" + condition?.Stringify() + ")" + body?.Stringify(); }
        public Expr? condition;
        public Stmt? body;
    }

    public class StructStmt : Stmt
    {
        public StructStmt()
        : base(AstType.STRUCT)
        {
            name = ""; members = new List<KeyValuePair<IdentifierExpr, Expr?>>();
        }
        public StructStmt(string name, List<KeyValuePair<IdentifierExpr, Expr?>> members)
          : base(AstType.STRUCT)
        {
            this.name = name;
            this.members = members;
        }
        public override string Stringify()
        {
            string result = "struct " + name + "{";

            foreach (var member in members)
                result += member.Key.Stringify() + ":" + member.Value?.Stringify() + "\n";

            return result + "}";
        }

        public string name;
        public List<KeyValuePair<IdentifierExpr, Expr?>> members;
    }
}