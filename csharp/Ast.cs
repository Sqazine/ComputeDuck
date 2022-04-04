namespace ComputeDuck
{
    public enum AstType
    {
        //expr
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
        LAMBDA,
        FUNCTION_CALL,
        STRUCT_CALL,
        //stmt
        VAR,
        EXPR,
        RETURN,
        IF,
        SCOPE,
        FUNCTION,
        WHILE,
        STRUCT,
    }


    public abstract class AstNode
    {
        public abstract string Stringify();
        public abstract AstType Type();
    }

    public abstract class Expr : AstNode
    {
    }

    public class NumExpr : Expr
    {
        public NumExpr() { this.value = 0.0; }
        public NumExpr(double value) { this.value = value; }
        public override string Stringify() { return value.ToString(); }
        public override AstType Type() { return AstType.NUM; }
        public double value;
    }

    public class StrExpr : Expr
    {
        public StrExpr() { this.value = ""; }
        public StrExpr(string value) { this.value = value; }
        public override string Stringify() { return "\"" + value + "\""; }
        public override AstType Type() { return AstType.STR; }
        public string value;
    }

    public class NilExpr : Expr
    {
        public NilExpr() { }
        public override string Stringify() { return "nil"; }
        public override AstType Type() { return AstType.NIL; }
    }

    public class BoolExpr : Expr
    {
        public BoolExpr() { this.value = false; }
        public BoolExpr(bool value) { this.value = value; }
        public override string Stringify() { return value ? "true" : "false"; }
        public override AstType Type() { return AstType.BOOL; }
        public bool value;
    }

    public class IdentifierExpr : Expr
    {
        public IdentifierExpr() { this.literal = ""; }
        public IdentifierExpr(string literal) { this.literal = literal; }
        public override string Stringify() { return literal; }
        public override AstType Type() { return AstType.IDENTIFIER; }
        public string literal;
    }

    public class ArrayExpr : Expr
    {
        public ArrayExpr() { elements = new List<Expr>(); }
        public ArrayExpr(List<Expr> elements) { this.elements = elements; }
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
        public override AstType Type() { return AstType.ARRAY; }

        public List<Expr> elements;
    }

    public class GroupExpr : Expr
    {
        public GroupExpr() { expr = null; }
        public GroupExpr(Expr expr) { this.expr = expr; }
        public override string Stringify() { return "(" + expr.Stringify() + ")"; }
        public override AstType Type() { return AstType.GROUP; }
        public Expr expr;
    }

    public class PrefixExpr : Expr
    {
        public PrefixExpr() { this.op = ""; this.right = null; }
        public PrefixExpr(string op, Expr right) { this.op = op; this.right = right; }

        public override string Stringify() { return op + right.Stringify(); }
        public override AstType Type() { return AstType.PREFIX; }
        public string op;
        public Expr right;
    }

    public class InfixExpr : Expr
    {
        public InfixExpr() { this.op = ""; this.left = null; this.right = null; }
        public InfixExpr(string op, Expr left, Expr right) { this.op = op; this.left = left; this.right = right; }

        public override string Stringify() { return this.left.Stringify() + op + right.Stringify(); }
        public override AstType Type() { return AstType.INFIX; }
        public string op;
        public Expr left;
        public Expr right;
    }

    public class IndexExpr : Expr
    {
        public IndexExpr() { this.ds = null; this.index = null; }
        public IndexExpr(Expr ds, Expr index) { this.ds = ds; this.index = index; }
        public override string Stringify() { return this.ds.Stringify() + "[" + this.index.Stringify() + "]"; }
        public override AstType Type() { return AstType.INFIX; }
        public Expr ds;
        public Expr index;
    }

    public class RefExpr : Expr
    {
        public RefExpr() { this.refExpr = null; }
        public RefExpr(IdentifierExpr refExpr) { this.refExpr = refExpr; }
        public override string Stringify() { return "ref " + refExpr.Stringify(); }
        public override AstType Type() { return AstType.REF; }
        public IdentifierExpr refExpr;
    }

    public class FunctionCallExpr : Expr
    {

        public FunctionCallExpr() { this.name = null; this.arguments = new List<Expr>(); }
        public FunctionCallExpr(Expr name, List<Expr> arguments) { this.name = name; this.arguments = arguments; }

        public override string Stringify()
        {
            string result = name.Stringify() + "(";

            if (arguments.Count != 0)
            {
                foreach (var arg in arguments)
                    result += arg.Stringify() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += ")";
            return result;
        }
        public override AstType Type() { return AstType.FUNCTION_CALL; }

        public Expr name;
        public List<Expr> arguments;

    }

    public class StructCallExpr : Expr
    {
        public StructCallExpr() { this.callee = null; this.callMember = null; }
        public StructCallExpr(Expr callee, Expr callmember) { this.callee = callee; this.callMember = callmember; }

        public override string Stringify() { return callee.Stringify() + "." + callMember.Stringify(); }
        public override AstType Type() { return AstType.STRUCT_CALL; }

        public Expr callee;
        public Expr callMember;
    }

    public abstract class Stmt : AstNode
    {

    }

    public class ExprStmt : Stmt
    {
        public ExprStmt() { this.expr = null; }
        public ExprStmt(Expr expr) { this.expr = expr; }

        public override string Stringify() { return expr.Stringify() + ";"; }
        public override AstType Type() { return AstType.EXPR; }

        public Expr expr;
    }

    public class VarStmt : Stmt
    {
        public VarStmt() { this.name = null; this.value = null; }
        public VarStmt(IdentifierExpr name, Expr value) { this.name = name; this.value = value; }

        public override string Stringify() { return "var " + name.Stringify() + "=" + value.Stringify() + ";"; }
        public override AstType Type() { return AstType.VAR; }

        public IdentifierExpr name;
        public Expr value;
    }


    public class ReturnStmt : Stmt
    {
        public ReturnStmt() { this.expr = null; }
        public ReturnStmt(Expr expr) { this.expr = expr; }

        public override string Stringify() { return "return " + expr.Stringify() + ";"; }
        public override AstType Type() { return AstType.RETURN; }
        public Expr expr;
    }

    public class IfStmt : Stmt
    {
        public IfStmt() { this.condition = null; this.thenBranch = null; this.elseBranch = null; }
        public IfStmt(Expr condition, Stmt thenBranch, Stmt elseBranch) { this.condition = null; this.thenBranch = null; this.elseBranch = null; }

        public override string Stringify()
        {
            string result = "if(" + condition.Stringify() + ")" + thenBranch.Stringify();
            if (elseBranch != null)
                result += "else " + elseBranch.Stringify();
            return result;
        }
        public override AstType Type() { return AstType.IF; }
        public Expr condition;
        public Stmt thenBranch;
        public Stmt elseBranch;
    }

    public class ScopeStmt : Stmt
    {
        public ScopeStmt() { this.stmts = new List<Stmt>(); }
        public ScopeStmt(List<Stmt> stmts) { this.stmts = stmts; }

        public override string Stringify()
        {
            string result = "{";
            foreach (var stmt in stmts)
                result += stmt.Stringify();
            return result + "}";
        }
        public override AstType Type() { return AstType.SCOPE; }

        public List<Stmt> stmts;
    }

    public class FunctionStmt : Stmt
    {
        public FunctionStmt() { this.name = ""; this.parameters = new List<IdentifierExpr>(); this.body = new ScopeStmt(); }
        public FunctionStmt(string name, List<IdentifierExpr> parameters, ScopeStmt body) { this.name = name; this.parameters = parameters; this.body = body; }


        public override string Stringify()
        {
            string result = "fn " + name + "(";
            if (parameters.Count != 0)
            {
                foreach (var param in parameters)
                    result += param.Stringify() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += ")" + body.Stringify();
            return result + "}";
        }
        public override AstType Type() { return AstType.FUNCTION; }
        public string name;
        public List<IdentifierExpr> parameters;
        public ScopeStmt body;
    }

    public class LambdaExpr : Expr
    {
        public LambdaExpr() { this.parameters = new List<IdentifierExpr>(); this.body = new ScopeStmt(); }
        public LambdaExpr(List<IdentifierExpr> parameters, ScopeStmt body) { this.parameters = new List<IdentifierExpr>(); this.body = new ScopeStmt(); }
        public override string Stringify()
        {
            string result = "lambda(";
            if (parameters.Count != 0)
            {
                foreach (var param in parameters)
                    result += param.Stringify() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += ")" + body.Stringify();
            return result + "}";
        }
        public override AstType Type() { return AstType.LAMBDA; }
        public List<IdentifierExpr> parameters;
        public ScopeStmt body;
    }

    public class WhileStmt : Stmt
    {
        public WhileStmt() {}
        public WhileStmt(Expr condition, Stmt body) { this.condition = condition; this.body = body; }
        public override string Stringify() { return "while(" + condition.Stringify() + ")" + body.Stringify(); }
        public override AstType Type() { return AstType.WHILE; }
        public  Expr condition;
        public Stmt body;
    }

    public class StructStmt : Stmt
    {
        public StructStmt() { name = ""; members = new List<VarStmt>(); }
        public StructStmt(string name, List<VarStmt> members) { this.name = name; this.members = members; }
        public override string Stringify()
        {
            string result = "struct " + name + "{";

            foreach (var varStmt in members)
                result += varStmt.Stringify() + "\n";

            return result + "}";
        }
        public override AstType Type() { return AstType.STRUCT; }

        public string name;
        public List<VarStmt> members;
    }
}