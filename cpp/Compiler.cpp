#include "Compiler.h"
#include "Utils.h"
Compiler::Compiler()
{
}
Compiler::~Compiler()
{
}

Frame Compiler::Compile(std::vector<Stmt *> stmts)
{
	for (const auto &s : stmts)
		CompileStmt(s, m_RootFrame);
	return m_RootFrame;
}

void Compiler::ResetStatus()
{
	m_RootFrame.Clear();
}

void Compiler::CompileStmt(Stmt *stmt, Frame &frame)
{
	switch (stmt->Type())
	{
	case AstType::RETURN:
		CompileReturnStmt((ReturnStmt *)stmt, frame);
		break;
	case AstType::EXPR:
		CompileExprStmt((ExprStmt *)stmt, frame);
		break;
	case AstType::VAR:
		CompileVarStmt((VarStmt *)stmt, frame);
		break;
	case AstType::SCOPE:
		CompileScopeStmt((ScopeStmt *)stmt, frame);
		break;
	case AstType::IF:
		CompileIfStmt((IfStmt *)stmt, frame);
		break;
	case AstType::WHILE:
		CompileWhileStmt((WhileStmt *)stmt, frame);
		break;
	case AstType::FUNCTION:
		CompileFunctionStmt((FunctionStmt *)stmt, frame);
		break;
	case AstType::STRUCT:
		CompileStructStmt((StructStmt *)stmt, frame);
		break;
	default:
		break;
	}
}
void Compiler::CompileReturnStmt(ReturnStmt *stmt, Frame &frame)
{
	if (stmt->expr)
		CompileExpr(stmt->expr, frame);

	frame.AddOpCode(OP_RETURN);
}

void Compiler::CompileExprStmt(ExprStmt *stmt, Frame &frame)
{
	CompileExpr(stmt->expr, frame);
}

void Compiler::CompileVarStmt(VarStmt *stmt, Frame &frame)
{
	CompileExpr(stmt->value, frame);
	CompileExpr(stmt->name, frame, INIT);
}

void Compiler::CompileScopeStmt(ScopeStmt *stmt, Frame &frame)
{
	frame.AddOpCode(OP_ENTER_SCOPE);

	for (const auto &s : stmt->stmts)
		CompileStmt(s, frame);

	frame.AddOpCode(OP_EXIT_SCOPE);
}

void Compiler::CompileIfStmt(IfStmt *stmt, Frame &frame)
{
	CompileExpr(stmt->condition, frame);

	frame.AddOpCode(OP_JUMP_IF_FALSE);
	uint64_t jmpIfFalseOffset = frame.AddNum(0);
	frame.AddOpCode(jmpIfFalseOffset);

	CompileStmt(stmt->thenBranch, frame);

	frame.AddOpCode(OP_JUMP);
	uint64_t jmpOffset = frame.AddNum(0);
	frame.AddOpCode(jmpOffset);

	frame.GetNums()[jmpIfFalseOffset] = (double)frame.GetOpCodeSize() - 1.0;

	if (stmt->elseBranch)
		CompileStmt(stmt->elseBranch, frame);

	frame.GetNums()[jmpOffset] = (double)frame.GetOpCodeSize() - 1.0;
}
void Compiler::CompileWhileStmt(WhileStmt *stmt, Frame &frame)
{
	uint64_t jmpAddress = frame.GetOpCodeSize() - 1;
	CompileExpr(stmt->condition, frame);

	frame.AddOpCode(OP_JUMP_IF_FALSE);
	uint64_t jmpIfFalseOffset = frame.AddNum(0);
	frame.AddOpCode(jmpIfFalseOffset);

	CompileStmt(stmt->body, frame);

	frame.AddOpCode(OP_JUMP);
	uint64_t offset = frame.AddNum(jmpAddress);
	frame.AddOpCode(offset);

	frame.GetNums()[jmpIfFalseOffset] = (double)frame.GetOpCodeSize() - 1.0;
}

void Compiler::CompileFunctionStmt(FunctionStmt *stmt, Frame &frame)
{
	Frame functionFrame = Frame(&frame);

	functionFrame.AddOpCode(OP_ENTER_SCOPE);

	for (int64_t i = stmt->parameters.size() - 1; i >= 0; --i)
		CompileIdentifierExpr(stmt->parameters[i], functionFrame, INIT);

	for (const auto &s : stmt->body->stmts)
		CompileStmt(s, functionFrame);

	functionFrame.AddOpCode(OP_EXIT_SCOPE);

	frame.AddFunctionFrame(stmt->name, functionFrame);
}

void Compiler::CompileStructStmt(StructStmt *stmt, Frame &frame)
{
	Frame structFrame = Frame(&frame);

	structFrame.AddOpCode(OP_ENTER_SCOPE);

	for (const auto &m : stmt->members)
		CompileVarStmt(m, structFrame);

	structFrame.AddOpCode(OP_NEW_STRUCT);
	uint64_t offset = structFrame.AddString(stmt->name);
	structFrame.AddOpCode(offset);

	structFrame.AddOpCode(OP_RETURN);

	frame.AddStructFrame(stmt->name, structFrame);
}

void Compiler::CompileExpr(Expr *expr, Frame &frame, ObjectState state)
{
	switch (expr->Type())
	{
	case AstType::NUM:
		CompileNumExpr((NumExpr *)expr, frame);
		break;
	case AstType::STR:
		CompileStrExpr((StrExpr *)expr, frame);
		break;
	case AstType::BOOL:
		CompileBoolExpr((BoolExpr *)expr, frame);
		break;
	case AstType::NIL:
		CompileNilExpr((NilExpr *)expr, frame);
		break;
	case AstType::IDENTIFIER:
		CompileIdentifierExpr((IdentifierExpr *)expr, frame, state);
		break;
	case AstType::GROUP:
		CompileGroupExpr((GroupExpr *)expr, frame);
		break;
	case AstType::ARRAY:
		CompileArrayExpr((ArrayExpr *)expr, frame);
		break;
	case AstType::INDEX:
		CompileIndexExpr((IndexExpr *)expr, frame, state);
		break;
	case AstType::PREFIX:
		CompilePrefixExpr((PrefixExpr *)expr, frame);
		break;
	case AstType::INFIX:
		CompileInfixExpr((InfixExpr *)expr, frame);
		break;
	case AstType::FUNCTION_CALL:
		CompileFunctionCallExpr((FunctionCallExpr *)expr, frame);
		break;
	case AstType::STRUCT_CALL:
		CompileStructCallExpr((StructCallExpr *)expr, frame, state);
		break;
	case AstType::REF:
		CompileRefExpr((RefExpr *)expr, frame);
		break;
	case AstType::LAMBDA:
		CompileLambdaExpr((LambdaExpr *)expr, frame);
		break;
	default:
		break;
	}
}

void Compiler::CompileNumExpr(NumExpr *expr, Frame &frame)
{
	frame.AddOpCode(OP_NEW_NUM);
	size_t offset = frame.AddNum(expr->value);
	frame.AddOpCode(offset);
}

void Compiler::CompileStrExpr(StrExpr *expr, Frame &frame)
{
	frame.AddOpCode(OP_NEW_STR);
	size_t offset = frame.AddString(expr->value);
	frame.AddOpCode(offset);
}

void Compiler::CompileBoolExpr(BoolExpr *expr, Frame &frame)
{
	if (expr->value)
		frame.AddOpCode(OP_NEW_TRUE);
	else
		frame.AddOpCode(OP_NEW_FALSE);
}

void Compiler::CompileNilExpr(NilExpr *expr, Frame &frame)
{
	frame.AddOpCode(OP_NEW_NIL);
}

void Compiler::CompileIdentifierExpr(IdentifierExpr *expr, Frame &frame, ObjectState state)
{
	if (state == READ)
		frame.AddOpCode(OP_GET_VAR);
	else if (state == WRITE)
		frame.AddOpCode(OP_SET_VAR);
	else if (state == INIT)
		frame.AddOpCode(OP_DEFINE_VAR);
	else if (state == STRUCT_READ)
		frame.AddOpCode(OP_GET_STRUCT_VAR);
	else if (state == STRUCT_WRITE)
		frame.AddOpCode(OP_SET_STRUCT_VAR);

	uint64_t offset = frame.AddString(expr->literal);
	frame.AddOpCode(offset);
}

void Compiler::CompileGroupExpr(GroupExpr *expr, Frame &frame)
{
	CompileExpr(expr->expr, frame);
}

void Compiler::CompileArrayExpr(ArrayExpr *expr, Frame &frame)
{
	for (const auto &e : expr->elements)
		CompileExpr(e, frame);

	frame.AddOpCode(OP_NEW_ARRAY);
	size_t offset = frame.AddNum(expr->elements.size());
	frame.AddOpCode(offset);
}

void Compiler::CompileIndexExpr(IndexExpr *expr, Frame &frame, ObjectState state)
{
	CompileExpr(expr->ds, frame);
	CompileExpr(expr->index, frame);
	if (state == READ)
		frame.AddOpCode(OP_GET_INDEX_VAR);
	else if (state == WRITE)
		frame.AddOpCode(OP_SET_INDEX_VAR);
}

void Compiler::CompileRefExpr(RefExpr *expr, Frame &frame)
{
	frame.AddOpCode(OP_REF);
	size_t offset = frame.AddString(expr->refExpr->literal);
	frame.AddOpCode(offset);
}

void Compiler::CompileLambdaExpr(LambdaExpr *expr, Frame &frame)
{
	Frame lambdaFrame = Frame(frame);

	lambdaFrame.AddOpCode(OP_ENTER_SCOPE);

	for (int64_t i = expr->parameters.size() - 1; i >= 0; --i)
		CompileIdentifierExpr(expr->parameters[i], lambdaFrame, INIT);

	for (const auto &s : expr->body->stmts)
		CompileStmt(s, lambdaFrame);

	lambdaFrame.AddOpCode(OP_EXIT_SCOPE);

	frame.AddOpCode(OP_NEW_LAMBDA);
	size_t offset = frame.AddNum(frame.AddLambdaFrame(lambdaFrame));
	frame.AddOpCode(offset);
}

void Compiler::CompilePrefixExpr(PrefixExpr *expr, Frame &frame)
{
	CompileExpr(expr->right, frame);
	if (expr->op == "-")
		frame.AddOpCode(OP_NEG);
	else if (expr->op == "not")
		frame.AddOpCode(OP_NOT);
}

void Compiler::CompileInfixExpr(InfixExpr *expr, Frame &frame)
{
	if (expr->op == "=")
	{
		CompileExpr(expr->right, frame);
		CompileExpr(expr->left, frame, WRITE);
	}
	else
	{
		CompileExpr(expr->right, frame);
		CompileExpr(expr->left, frame);

		if (expr->op == "+")
			frame.AddOpCode(OP_ADD);
		else if (expr->op == "-")
			frame.AddOpCode(OP_SUB);
		else if (expr->op == "*")
			frame.AddOpCode(OP_MUL);
		else if (expr->op == "/")
			frame.AddOpCode(OP_DIV);
		else if (expr->op == "and")
			frame.AddOpCode(OP_AND);
		else if (expr->op == "or")
			frame.AddOpCode(OP_OR);
		else if (expr->op == ">")
			frame.AddOpCode(OP_GREATER);
		else if (expr->op == "<")
			frame.AddOpCode(OP_LESS);
		else if (expr->op == ">=")
			frame.AddOpCode(OP_GREATER_EQUAL);
		else if (expr->op == "<=")
			frame.AddOpCode(OP_LESS_EQUAL);
		else if (expr->op == "==")
			frame.AddOpCode(OP_EQUAL);
		else if (expr->op == "!=")
			frame.AddOpCode(OP_NOT_EQUAL);
		else
			Assert("Unknown binary op:" + expr->op);
	}
}

void Compiler::CompileFunctionCallExpr(FunctionCallExpr *expr, Frame &frame)
{

	for (const auto &arg : expr->arguments)
		CompileExpr(arg, frame);

	//argument count
	frame.AddOpCode(OP_NEW_NUM);
	uint64_t offset = frame.AddNum(expr->arguments.size());
	frame.AddOpCode(offset);

	frame.AddOpCode(OP_FUNCTION_CALL);
	offset = frame.AddString(expr->name);
	frame.AddOpCode(offset);
}

void Compiler::CompileStructCallExpr(StructCallExpr *expr, Frame &frame, ObjectState state)
{
	CompileExpr(expr->callee, frame);

	if (expr->callMember->Type() == AstType::STRUCT_CALL) //continuous struct call such as a.b.c;
		CompileExpr(((StructCallExpr *)expr->callMember)->callee, frame, STRUCT_READ);

	if (state == READ)
		CompileExpr(expr->callMember, frame, STRUCT_READ);
	else if (state == WRITE)
		CompileExpr(expr->callMember, frame, STRUCT_WRITE);
}