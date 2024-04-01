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

	auto fn = GetCurFunction();

	PopFunction();

	m_Builder->CreateRet(llvm::ConstantPointerNull::get(m_ValuePtrType));

	auto b = llvm::verifyFunction(*fn);
	m_FPM->run(*fn);

	return fn;
}

void LLVMCompiler::Run(llvm::Function* fn)
{
	auto rt = m_Jit->GetMainJITDylib().createResourceTracker();

	auto tsm = llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context));

	m_Jit->AddModule(std::move(tsm), rt);

	InitModuleAndPassManager();

	auto symbol = m_ExitOnErr(m_Jit->LookUp("main"));

	void (*fp)() = symbol.getAddress().toPtr<void (*)()>();
	fp();
	rt->remove();
}

void LLVMCompiler::ResetStatus()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

	if (m_SymbolTable)
		SAFE_DELETE(m_SymbolTable);
	m_SymbolTable = new SymbolTable();

	m_Jit = LLVMJit::Create();

	InitModuleAndPassManager();
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
	CompileExpr(stmt->expr);
	Pop();
}

void LLVMCompiler::CompileIfStmt(IfStmt* stmt)
{
}
void LLVMCompiler::CompileScopeStmt(ScopeStmt* stmt)
{
	EnterScope();

	llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_Context, "scope" + std::to_string(m_SymbolTable->scopeDepth), m_Builder->GetInsertBlock()->getParent());
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

		if (expr->op == "+")
		{
			auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
			llvm::Value* resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, { m_Builder->getInt32(0), m_Builder->getInt32(0) });

			auto fn = BuiltinManager::GetInstance()->m_LlvmBuiltins["ValueAdd"];

			auto callInst = m_Builder->CreateCall(fn, { lValue, rValue,resultAddr });
			callInst->addParamAttr(0, llvm::Attribute::NoUndef);
			callInst->addParamAttr(1, llvm::Attribute::NoUndef);
			callInst->addParamAttr(2, llvm::Attribute::NoUndef);

			Push(resultAddr);

		}
		else if (expr->op == "-")
		{
			auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
			llvm::Value* resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, { m_Builder->getInt32(0), m_Builder->getInt32(0) });

			auto fn = BuiltinManager::GetInstance()->m_LlvmBuiltins["ValueSub"];

			auto callInst = m_Builder->CreateCall(fn, { lValue, rValue,resultAddr });
			callInst->addParamAttr(0, llvm::Attribute::NoUndef);
			callInst->addParamAttr(1, llvm::Attribute::NoUndef);
			callInst->addParamAttr(2, llvm::Attribute::NoUndef);

			Push(resultAddr);

		}
		else if (expr->op == "*")
		{
			auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
			llvm::Value* resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, { m_Builder->getInt32(0), m_Builder->getInt32(0) });

			auto fn = BuiltinManager::GetInstance()->m_LlvmBuiltins["ValueMul"];

			auto callInst = m_Builder->CreateCall(fn, { lValue, rValue,resultAddr });
			callInst->addParamAttr(0, llvm::Attribute::NoUndef);
			callInst->addParamAttr(1, llvm::Attribute::NoUndef);
			callInst->addParamAttr(2, llvm::Attribute::NoUndef);

			Push(resultAddr);

		}
		else if (expr->op == "/")
		{
			auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
			llvm::Value* resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, { m_Builder->getInt32(0), m_Builder->getInt32(0) });

			auto fn = BuiltinManager::GetInstance()->m_LlvmBuiltins["ValueDiv"];

			auto callInst = m_Builder->CreateCall(fn, { lValue, rValue,resultAddr });
			callInst->addParamAttr(0, llvm::Attribute::NoUndef);
			callInst->addParamAttr(1, llvm::Attribute::NoUndef);
			callInst->addParamAttr(2, llvm::Attribute::NoUndef);

			Push(resultAddr);

		}
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
	auto vT = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NUM));
	auto d = llvm::ConstantFP::get(*m_Context, llvm::APFloat(expr->value));

	auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

	llvm::Value* memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(0) });
	m_Builder->CreateStore(vT, memberAddr);
	llvm::Value* memberAddr1 = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(1) });
	m_Builder->CreateStore(d, memberAddr1);

	Push(memberAddr);
}
void LLVMCompiler::CompileBoolExpr(BoolExpr* expr)
{
	auto vT = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::BOOL));
	auto d = llvm::ConstantFP::get(*m_Context, llvm::APFloat(expr->value ? 1.0 : 0.0));

	auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

	llvm::Value* memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(0) });
	m_Builder->CreateStore(vT, memberAddr);
	llvm::Value* memberAddr1 = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(1) });
	m_Builder->CreateStore(d, memberAddr1);

	Push(memberAddr);
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
	//Push(m_Builder->CreateGlobalString(expr->value));
}
void LLVMCompiler::CompileNilExpr(NilExpr* expr)
{
	//Push(llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getVoidTy(*m_Context), 0)));
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
	Symbol symbol;
	bool isFound = m_SymbolTable->Resolve(expr->literal, symbol);
	if (state == RWState::READ)
	{
		if (!isFound)
			ASSERT("Undefined variable:%s", expr->Stringify().c_str());

		switch (symbol.scope)
		{
		case SymbolScope::GLOBAL:
		{
			Push(symbol.allocationGEP);
			break;
		}
		case SymbolScope::LOCAL:
		{
			Push(symbol.allocationGEP);
			break;
		}
		case SymbolScope::BUILTIN:
		{
			auto v = BuiltinManager::GetInstance()->m_LlvmBuiltins[symbol.name];
			Push(v);
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
		case SymbolScope::GLOBAL:
		{
			auto init = Pop();
			if (!isFound) //already exists like:a=10;{a=20;}
			{
				auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr, symbol.name);
				auto memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(0) }, symbol.name);

				m_Builder->CreateMemCpy(memberAddr, llvm::MaybeAlign(8), init, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));

				m_SymbolTable->Set(symbol.name, memberAddr);

				Push(memberAddr);
			}
			else
			{
				auto memberAddr = symbol.allocationGEP;
				m_Builder->CreateMemCpy(memberAddr, llvm::MaybeAlign(8), init, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
				Push(memberAddr);
			}
			break;
		}
		case SymbolScope::LOCAL:
		{
			auto init = Pop();
			if (!isFound) //already exists like:{a=10;{a=20;}}
			{
				auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr, symbol.name);
				auto memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(0) }, symbol.name);

				m_Builder->CreateMemCpy(memberAddr, llvm::MaybeAlign(8), init, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));

				m_SymbolTable->Set(symbol.name, memberAddr);

				Push(memberAddr);
			}
			else
			{
				auto memberAddr = symbol.allocationGEP;
				m_Builder->CreateMemCpy(memberAddr, llvm::MaybeAlign(8), init, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
				Push(memberAddr);
			}
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

	llvm::BasicBlock* block = llvm::BasicBlock::Create(*m_Context, "", fn);
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

	//if (fn->arg_size() != expr->arguments.size())
	//	ASSERT("InCompatible arg size:%d,%d", fn->arg_size(), expr->arguments.size());

	// builtin function
	{
		std::vector<llvm::Value*> argsV;
		for (unsigned i = 0; i < expr->arguments.size(); ++i)
		{
			CompileExpr(expr->arguments[i]);
			argsV.push_back(Pop());
			if (!argsV.back())
				return;
		}

		auto valueArrayType = llvm::ArrayType::get(m_ValueType, argsV.size());

		auto arg0 = m_Builder->CreateAlloca(valueArrayType, nullptr);
		llvm::Value* arg0MemberAddr = m_Builder->CreateInBoundsGEP(valueArrayType, arg0, { m_Builder->getInt32(0), m_Builder->getInt32(0) });

		for (auto i = 0; i < argsV.size(); ++i)
		{
			if (i == 0)
			{
				m_Builder->CreateMemCpy(arg0MemberAddr, llvm::MaybeAlign(8), argsV[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
			}
			else
			{
				llvm::Value* argIMemberAddr = m_Builder->CreateInBoundsGEP(valueArrayType, arg0, { m_Builder->getInt32(0), m_Builder->getInt32(i) });
				m_Builder->CreateMemCpy(argIMemberAddr, llvm::MaybeAlign(8), argsV[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
			}
		}

		llvm::Value* arg1 = m_Builder->getInt8(argsV.size());

		auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
		auto resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, { m_Builder->getInt32(0), m_Builder->getInt32(0) });

		fn->addParamAttr(0, llvm::Attribute::NoUndef);
		fn->addParamAttr(2, llvm::Attribute::NoUndef);

		auto callInst = m_Builder->CreateCall(fn, { arg0MemberAddr, arg1 ,resultAddr });
		callInst->addParamAttr(0, llvm::Attribute::NoUndef);
		callInst->addParamAttr(2, llvm::Attribute::NoUndef);

		Push(resultAddr);
	}
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

void LLVMCompiler::PushFunction(llvm::Function* fn)
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
	m_SymbolTable = new SymbolTable(m_SymbolTable);
}
void LLVMCompiler::ExitScope()
{
	m_SymbolTable = m_SymbolTable->enclosing;
}

void LLVMCompiler::InitModuleAndPassManager()
{

	m_Context = std::make_unique<llvm::LLVMContext>();

	m_Module = std::make_unique<llvm::Module>(BuiltinManager::GetInstance()->GetExecuteFilePath(), *m_Context);
	m_Module->setDataLayout(m_Jit->GetDataLayout());

	m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);

	m_FPM = std::make_unique<llvm::legacy::FunctionPassManager>(m_Module.get());
	m_FPM->add(llvm::createPromoteMemoryToRegisterPass());
	m_FPM->add(llvm::createInstructionCombiningPass());
	m_FPM->add(llvm::createReassociatePass());
	m_FPM->add(llvm::createGVNPass());
	m_FPM->add(llvm::createCFGSimplificationPass());
	m_FPM->doInitialization();

	{
		m_Int8Type = llvm::Type::getInt8Ty(*m_Context);
		m_DoubleType = llvm::Type::getDoubleTy(*m_Context);
		m_BoolType = llvm::Type::getInt1Ty(*m_Context);
		m_Int64Type = llvm::Type::getInt64Ty(*m_Context);
		m_Int32Type = llvm::Type::getInt32Ty(*m_Context);
		m_VoidType = llvm::Type::getVoidTy(*m_Context);

		m_Int64PtrType = llvm::PointerType::get(m_Int64Type, 0);
		m_Int32PtrType = llvm::PointerType::get(m_Int32Type, 0);
		m_Int8PtrType = llvm::PointerType::get(m_Int8Type, 0);
		m_BoolPtrType = llvm::PointerType::get(m_BoolType, 0);

		m_ObjectType = llvm::StructType::create(*m_Context, { m_Int8Type,m_Int8PtrType,m_Int8Type }, "struct.Object");
		m_ObjectPtrType = llvm::PointerType::get(m_ObjectType, 0);

		mUnionType = llvm::StructType::create(*m_Context, { m_DoubleType }, "union.anon");

		m_ValueType = llvm::StructType::create(*m_Context, { m_Int8Type, mUnionType }, "struct.Value");
		m_ValuePtrType = llvm::PointerType::get(m_ValueType, 0);

		m_BuiiltinFunctionType = llvm::FunctionType::get(m_VoidType, { m_ValuePtrType,m_Int8Type ,m_ValuePtrType }, false);

		m_ValueFunctionType = llvm::FunctionType::get(m_VoidType, { m_ValuePtrType,m_ValuePtrType,m_ValuePtrType }, false);
	}

	auto valueAddFn = llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "gValueAdd", m_Module.get());
	valueAddFn->addParamAttr(0, llvm::Attribute::NoUndef);
	valueAddFn->addParamAttr(1, llvm::Attribute::NoUndef);
	valueAddFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto valueSubFn = llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "gValueSub", m_Module.get());
	valueSubFn->addParamAttr(0, llvm::Attribute::NoUndef);
	valueSubFn->addParamAttr(1, llvm::Attribute::NoUndef);
	valueSubFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto valueMulFn = llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "gValueMul", m_Module.get());
	valueMulFn->addParamAttr(0, llvm::Attribute::NoUndef);
	valueMulFn->addParamAttr(1, llvm::Attribute::NoUndef);
	valueMulFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto valueDivFn = llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "gValueDiv", m_Module.get());
	valueDivFn->addParamAttr(0, llvm::Attribute::NoUndef);
	valueDivFn->addParamAttr(1, llvm::Attribute::NoUndef);
	valueDivFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto printFn = llvm::Function::Create(m_BuiiltinFunctionType, llvm::Function::ExternalLinkage, "gPrint", m_Module.get());
	printFn->addParamAttr(0, llvm::Attribute::NoUndef);
	printFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto printlnFn = llvm::Function::Create(m_BuiiltinFunctionType, llvm::Function::ExternalLinkage, "gPrintln", m_Module.get());
	printlnFn->addParamAttr(0, llvm::Attribute::NoUndef);
	printlnFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto sizeofFn = llvm::Function::Create(m_BuiiltinFunctionType, llvm::Function::ExternalLinkage, "gSizeof", m_Module.get());
	sizeofFn->addParamAttr(0, llvm::Attribute::NoUndef);
	sizeofFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto insertFn = llvm::Function::Create(m_BuiiltinFunctionType, llvm::Function::ExternalLinkage, "gInsert", m_Module.get());
	insertFn->addParamAttr(0, llvm::Attribute::NoUndef);
	insertFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto eraseFn = llvm::Function::Create(m_BuiiltinFunctionType, llvm::Function::ExternalLinkage, "gErase", m_Module.get());
	eraseFn->addParamAttr(0, llvm::Attribute::NoUndef);
	eraseFn->addParamAttr(2, llvm::Attribute::NoUndef);

	auto clockFn = llvm::Function::Create(m_BuiiltinFunctionType, llvm::Function::ExternalLinkage, "gClock", m_Module.get());
	clockFn->addParamAttr(0, llvm::Attribute::NoUndef);
	clockFn->addParamAttr(2, llvm::Attribute::NoUndef);

	BuiltinManager::GetInstance()->RegisterLlvmFn("ValueAdd", valueAddFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("ValueSub", valueSubFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("ValueMul", valueMulFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("ValueDiv", valueDivFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("print", printFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("println", printlnFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("sizeof", sizeofFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("insert", insertFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("erase", eraseFn);
	BuiltinManager::GetInstance()->RegisterLlvmFn("clock", clockFn);

	m_SymbolTable->DefineBuiltin("ValueAdd");
	m_SymbolTable->DefineBuiltin("ValueSub");
	m_SymbolTable->DefineBuiltin("ValueMul");
	m_SymbolTable->DefineBuiltin("ValueDiv");
	m_SymbolTable->DefineBuiltin("print");
	m_SymbolTable->DefineBuiltin("println");
	m_SymbolTable->DefineBuiltin("sizeof");
	m_SymbolTable->DefineBuiltin("insert");
	m_SymbolTable->DefineBuiltin("erase");
	m_SymbolTable->DefineBuiltin("clock");

	llvm::FunctionType* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_Context), false);
	llvm::Function* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "main", m_Module.get());
	llvm::BasicBlock* codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
	m_Builder->SetInsertPoint(codeBlock);

	PushFunction(fn);
}
