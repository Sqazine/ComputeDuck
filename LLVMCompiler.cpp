#include "LLVMCompiler.h"
#include "BuiltinManager.h"
LLVMCompiler::LLVMCompiler()
	: m_SymbolTable(nullptr)
{
	ResetStatus();
}

LLVMCompiler::~LLVMCompiler()
{
	if (m_SymbolTable)
		SAFE_DELETE(m_SymbolTable);
}

llvm::Function* LLVMCompiler::Compile(const std::vector<Stmt*>& stmts)
{
	for (const auto& stmt : stmts)
		CompileStmt(stmt);

#ifdef _DEBUG
	m_Module->print(llvm::outs(), nullptr);
#endif

	return GetCurFunction();
}

void LLVMCompiler::ResetStatus()
{
	m_Context = std::make_unique<llvm::LLVMContext>();
	m_Module = std::make_unique<llvm::Module>(BuiltinManager::GetInstance()->GetExecuteFilePath(), *m_Context);
	m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);

	if (m_SymbolTable)
		SAFE_DELETE(m_SymbolTable);
	m_SymbolTable = new LLVMSymbolTable();

	std::vector<llvm::Type*> Doubles(1, llvm::Type::getDoubleTy(*m_Context));
	llvm::FunctionType* FT = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_Context), Doubles, false);
	llvm::Function* F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "println", m_Module.get());
	BuiltinManager::GetInstance()->RegisterLlvmFn("println", F);

	m_SymbolTable->DefineBuiltin("println");

	m_FunctionStack.clear();

	llvm::FunctionType* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_Context), {}, false);
	llvm::Function* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "main", m_Module.get());
	llvm::BasicBlock* codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
	m_Builder->SetInsertPoint(codeBlock);

	m_FunctionStack.emplace_back(fn);
}

void LLVMCompiler::CompileStmt(Stmt* stmt)
{
	switch (stmt->type)
	{
	case AstType::RETURN:
		CompileReturnStmt((ReturnStmt*)stmt);
		break;
	case AstType::EXPR:
		CompileExprStmt((ExprStmt*)stmt);
		break;
	case AstType::SCOPE:
		CompileScopeStmt((ScopeStmt*)stmt);
		break;
	case AstType::IF:
		CompileIfStmt((IfStmt*)stmt);
		break;
	case AstType::WHILE:
		CompileWhileStmt((WhileStmt*)stmt);
		break;
	case AstType::STRUCT:
		CompileStructStmt((StructStmt*)stmt);
		break;
	default:
		break;
	}
}

void LLVMCompiler::CompileExprStmt(ExprStmt* stmt)
{
	return CompileExpr(stmt->expr);
}

void LLVMCompiler::CompileIfStmt(IfStmt* stmt)
{
}
void LLVMCompiler::CompileScopeStmt(ScopeStmt* stmt)
{
	EnterScope();

	llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_Context, "scope"+ std::to_string(m_SymbolTable->scopeDepth), m_Builder->GetInsertBlock()->getParent());
	m_Builder->CreateBr(block);
	m_Builder->SetInsertPoint(block);

	for (const auto& s : stmt->stmts)	
		CompileStmt(s);

	ExitScope();
}
void LLVMCompiler::CompileWhileStmt(WhileStmt* stmt)
{
}
void LLVMCompiler::CompileReturnStmt(ReturnStmt* stmt)
{
	auto function = m_Builder->GetInsertBlock()->getParent();
	if (!stmt->expr)
		m_Builder->CreateRetVoid();
	CompileExpr(stmt->expr);
	m_Builder->CreateRet(Pop());
}
void LLVMCompiler::CompileStructStmt(StructStmt* stmt)
{
}

void LLVMCompiler::CompileExpr(Expr* expr, const RWState& state)
{
	switch (expr->type)
	{
	case AstType::NUM:
		CompileNumExpr((NumExpr*)expr);
		break;
	case AstType::STR:
		CompileStrExpr((StrExpr*)expr);
		break;
	case AstType::BOOL:
		CompileBoolExpr((BoolExpr*)expr);
		break;
	case AstType::NIL:
		CompileNilExpr((NilExpr*)expr);
		break;
	case AstType::IDENTIFIER:
		CompileIdentifierExpr((IdentifierExpr*)expr, state);
		break;
	case AstType::GROUP:
		CompileGroupExpr((GroupExpr*)expr);
		break;
	case AstType::ARRAY:
		CompileArrayExpr((ArrayExpr*)expr);
		break;
	case AstType::INDEX:
		CompileIndexExpr((IndexExpr*)expr);
		break;
	case AstType::PREFIX:
		CompilePrefixExpr((PrefixExpr*)expr);
		break;
	case AstType::INFIX:
		CompileInfixExpr((InfixExpr*)expr);
		break;
	case AstType::FUNCTION_CALL:
		CompileFunctionCallExpr((FunctionCallExpr*)expr);
		break;
	case AstType::STRUCT_CALL:
		CompileStructCallExpr((StructCallExpr*)expr, state);
		break;
	case AstType::REF:
		CompileRefExpr((RefExpr*)expr);
		break;
	case AstType::FUNCTION:
		CompileFunctionExpr((FunctionExpr*)expr);
		break;
	case AstType::ANONY_STRUCT:
		CompileAnonyStructExpr((AnonyStructExpr*)expr);
		break;
	case AstType::DLL_IMPORT:
		CompileDllImportExpr((DllImportExpr*)expr);
		break;
	default:
		break;
	}
}
void LLVMCompiler::CompileInfixExpr(InfixExpr* expr)
{
	if (expr->op == "=")
	{
		if (expr->left->type == AstType::IDENTIFIER && expr->right->type == AstType::FUNCTION)
			m_SymbolTable->Define(((IdentifierExpr*)expr->left)->literal);

		CompileExpr(expr->right);
		CompileExpr(expr->left, RWState::WRITE);
	}
	else
	{
		CompileExpr(expr->right);
		CompileExpr(expr->left);

		auto lValue = Pop();
		auto rValue = Pop();

		if (!lValue || !rValue)
			return;

		if (expr->op == "+")
			Push(m_Builder->CreateFAdd(lValue, rValue));
		else if (expr->op == "-")
			Push(m_Builder->CreateFSub(lValue, rValue));
		else if (expr->op == "*")
			Push(m_Builder->CreateFMul(lValue, rValue));
		else if (expr->op == "/")
			Push(m_Builder->CreateFDiv(lValue, rValue));
		else if (expr->op == ">")
			Push(m_Builder->CreateFCmpUGT(lValue, rValue));
		else if (expr->op == "<")
			Push(m_Builder->CreateFCmpULT(lValue, rValue));
		else if (expr->op == "&")
			Push(m_Builder->CreateBinOp(llvm::Instruction::And, lValue, rValue));
		else if (expr->op == "|")
			Push(m_Builder->CreateBinOp(llvm::Instruction::Or, lValue, rValue));
		else if (expr->op == "^")
			Push(m_Builder->CreateBinOp(llvm::Instruction::Xor, lValue, rValue));
		else if (expr->op == ">=")
			Push(m_Builder->CreateFCmpUGE(lValue, rValue));
		else if (expr->op == "<=")
			Push(m_Builder->CreateFCmpULE(lValue, rValue));
		else if (expr->op == "==")
			Push(m_Builder->CreateFCmpUEQ(lValue, rValue));
		else if (expr->op == "!=")
			Push(m_Builder->CreateFCmpUNE(lValue, rValue));
		else if (expr->op == "and")
			Push(m_Builder->CreateLogicalAnd(lValue, rValue));
		else if (expr->op == "or")
			Push(m_Builder->CreateLogicalOr(lValue, rValue));
	}
}
void LLVMCompiler::CompileNumExpr(NumExpr* expr)
{
	Push(llvm::ConstantFP::get(*m_Context, llvm::APFloat(expr->value)));
}
void LLVMCompiler::CompileBoolExpr(BoolExpr* expr)
{
	Push(llvm::ConstantInt::get(*m_Context, llvm::APInt(1, expr->value ? 1 : 0)));
}
void LLVMCompiler::CompilePrefixExpr(PrefixExpr* expr)
{
	CompileExpr(expr->right);
	auto rValue = Pop();
	if (expr->op == "-")
		Push(m_Builder->CreateUnOp(llvm::Instruction::UnaryOps::FNeg, rValue));
	else if (expr->op == "not")
		Push(m_Builder->CreateNot(rValue));
	else if (expr->op == "~")
	{
		rValue = m_Builder->CreateFPToUI(rValue, llvm::Type::getInt64Ty(*m_Context));
		Push(m_Builder->CreateUnOp(llvm::Instruction::UnaryOps::FNeg, rValue));
	}
	else
		ASSERT("Unrecognized prefix op");
}
void LLVMCompiler::CompileStrExpr(StrExpr* expr)
{
	Push(m_Builder->CreateGlobalString(expr->value));
}
void LLVMCompiler::CompileNilExpr(NilExpr* expr)
{
	Push(llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getVoidTy(*m_Context), 0)));
}
void LLVMCompiler::CompileGroupExpr(GroupExpr* expr)
{
	CompileExpr(expr->expr);
}
void LLVMCompiler::CompileArrayExpr(ArrayExpr* expr)
{

}
void LLVMCompiler::CompileIndexExpr(IndexExpr* expr)
{

}
void LLVMCompiler::CompileIdentifierExpr(IdentifierExpr* expr, const RWState& state)
{
	LLVMSymbol symbol;
	bool isFound = m_SymbolTable->Resolve(expr->literal, symbol);
	if (state == RWState::READ)
	{
		if (!isFound)
			ASSERT("Undefined variable:%s", expr->Stringify().c_str());

		switch (symbol.scope)
		{
		case LLVMSymbolScope::GLOBAL:
		{
			Push(m_Builder->CreateLoad(symbol.alloc->getAllocatedType(), symbol.alloc, expr->literal));
			break;
		}
		case LLVMSymbolScope::LOCAL:
		{
			for (auto& arg : m_Builder->GetInsertBlock()->getParent()->args())
			{
				if (arg.getName() == symbol.name)
				{
					Push(&arg);	
					break;
				}
			}
			break;
		}
		case LLVMSymbolScope::BUILTIN:
		{
			Push(BuiltinManager::GetInstance()->m_LlvmBuiltins[symbol.name]);
			break;
		}
		default:
			break;
		}
	}
	else
	{
		if (!isFound)
			symbol = m_SymbolTable->Define(expr->literal);
		switch (symbol.scope)
		{
		case LLVMSymbolScope::GLOBAL:
		{
			auto init = Pop();
			auto alloc = CreateEntryBlockAlloca(m_Module->getFunction("main"), symbol.name, init->getType());
			m_SymbolTable->Set(symbol.name, alloc);
			m_Builder->CreateStore(init, alloc);
			break;
		}
		case LLVMSymbolScope::LOCAL:
		{
			auto init = Pop();
			auto alloc = CreateEntryBlockAlloca(m_Builder->GetInsertBlock()->getParent(), symbol.name, init->getType());
			m_SymbolTable->Set(symbol.name, alloc);
			m_Builder->CreateStore(init, alloc);
			break;
		}
		default:
			break;
		}
	}
}
void LLVMCompiler::CompileFunctionExpr(FunctionExpr* expr)
{
	auto beforeblock = m_Builder->GetInsertBlock();

	EnterScope();

	for (const auto& param : expr->parameters)
		m_SymbolTable->Define(param->literal);

	std::vector<llvm::Type*> argTypes(expr->parameters.size(), llvm::Type::getDoubleTy(*m_Context));
	llvm::FunctionType* fnType = llvm::FunctionType::get(llvm::Type::getDoubleTy(*m_Context), argTypes, false);
	llvm::Function* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "", m_Module.get());

	unsigned idx = 0;
	for (auto& arg : fn->args())
		arg.setName(expr->parameters[idx++]->literal);

	llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_Context,"",fn);
	m_Builder->SetInsertPoint(block);

	CompileStmt(expr->body);

	ExitScope();

	m_Builder->SetInsertPoint(beforeblock);

	Push(fn);
}
void LLVMCompiler::CompileFunctionCallExpr(FunctionCallExpr* expr)
{
	CompileExpr(expr->name);
	auto fn = static_cast<llvm::Function*>(Pop());
	if (!fn)
		ASSERT("Unknown function:%s", expr->name->Stringify());

	if (fn->arg_size() != expr->arguments.size())
		ASSERT("InCompatible arg size:%d,%d",fn->arg_size(),expr->arguments.size());

	std::vector<llvm::Value*> argsV;
	for (unsigned i = 0;i<expr->arguments.size();++i) 
	{
		CompileExpr(expr->arguments[i]);
		argsV.push_back(Pop());
		if (!argsV.back())
			return;
	}

	m_Builder->CreateCall(fn, argsV, "call");
}
void LLVMCompiler::CompileStructCallExpr(StructCallExpr* expr, const RWState& state)
{

}
void LLVMCompiler::CompileRefExpr(RefExpr* expr)
{

}
void LLVMCompiler::CompileAnonyStructExpr(AnonyStructExpr* expr)
{

}
void LLVMCompiler::CompileDllImportExpr(DllImportExpr* expr)
{

}

llvm::AllocaInst* LLVMCompiler::CreateEntryBlockAlloca(llvm::Function* fn, llvm::StringRef name, llvm::Type* type)
{
	return m_Builder->CreateAlloca(type, nullptr, name);
}

void LLVMCompiler::Push(llvm::Value* v)
{
	m_ValueStack.push_back(v);
}
llvm::Value* LLVMCompiler::Peek(int32_t distance)
{
	return m_ValueStack[m_ValueStack.size() - 1 - distance];
}
llvm::Value* LLVMCompiler::Pop()
{
	auto v = Peek(0);
	m_ValueStack.pop_back();
	return v;
}

llvm::Function* LLVMCompiler::GetCurFunction()
{
	return PeekFunction(0);
}

void LLVMCompiler::AddFunction(llvm::Function* fn)
{
	m_FunctionStack.push_back(fn);
}

llvm::Function* LLVMCompiler::PeekFunction(int32_t distance)
{
	return m_FunctionStack[m_FunctionStack.size() - 1 - distance];
}

llvm::Function* LLVMCompiler::PopFunction()
{
	auto f = PeekFunction(0);
	m_FunctionStack.pop_back();
	return f;
}

void LLVMCompiler::EnterScope()
{
	m_SymbolTable = new LLVMSymbolTable(m_SymbolTable);
}
void LLVMCompiler::ExitScope()
{
	m_SymbolTable = m_SymbolTable->enclosing;
}