#include "LLVMCompilerImpl.h"
#include "BuiltinManager.h"

LLVMCompilerImpl::LLVMCompilerImpl()
{
}

LLVMCompilerImpl::~LLVMCompilerImpl()
{
    SAFE_DELETE(m_SymbolTable);
}

llvm::Function *LLVMCompilerImpl::Compile(const std::vector<Stmt *> &stmts)
{
	ResetStatus();

	for (const auto &stmt : stmts)
		CompileStmt(stmt);

#ifndef NDEBUG
	m_Module->print(llvm::outs(), nullptr);
#endif

	auto fn = GetCurFunction();

	PopFunction();

	m_Builder->CreateRet(llvm::ConstantPointerNull::get(m_ValuePtrType));

	auto b = llvm::verifyFunction(*fn);
	m_FPM->run(*fn);

	return fn;
}

void LLVMCompilerImpl::Run(llvm::Function *fn)
{
	auto rt = m_Jit->GetMainJITDylib().createResourceTracker();

	auto tsm = llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context));

	m_ExitOnErr(m_Jit->AddModule(std::move(tsm), rt));

	auto symbol = m_ExitOnErr(m_Jit->LookUp("main"));

	using JitFuncType = void (*)();

	JitFuncType fp = reinterpret_cast<JitFuncType>(symbol.getAddress());
	fp();
	m_ExitOnErr(rt->remove());
}

void LLVMCompilerImpl::ResetStatus()
{
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();
	llvm::InitializeNativeTargetAsmParser();

	SAFE_DELETE(m_SymbolTable);
	
	m_SymbolTable = new SymbolTable();

	m_Jit = LLVMJit::Create();

	InitModuleAndPassManager();
}

void LLVMCompilerImpl::CompileStmt(Stmt *stmt)
{
	switch (stmt->type)
	{
	case AstType::RETURN:
		CompileReturnStmt((ReturnStmt *)stmt);
		break;
	case AstType::EXPR:
		CompileExprStmt((ExprStmt *)stmt);
		break;
	case AstType::SCOPE:
		CompileScopeStmt((ScopeStmt *)stmt);
		break;
	case AstType::IF:
		CompileIfStmt((IfStmt *)stmt);
		break;
	case AstType::WHILE:
		CompileWhileStmt((WhileStmt *)stmt);
		break;
	case AstType::STRUCT:
		CompileStructStmt((StructStmt *)stmt);
		break;
	default:
		break;
	}
}

void LLVMCompilerImpl::CompileExprStmt(ExprStmt *stmt)
{
	CompileExpr(stmt->expr);
	Pop();
}

void LLVMCompilerImpl::CompileIfStmt(IfStmt *stmt)
{
}
void LLVMCompilerImpl::CompileScopeStmt(ScopeStmt *stmt)
{
	EnterScope();

	llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_Context, "scope" + std::to_string(m_SymbolTable->scopeDepth), m_Builder->GetInsertBlock()->getParent());
	m_Builder->CreateBr(block);
	m_Builder->SetInsertPoint(block);

	for (const auto &s : stmt->stmts)
		CompileStmt(s);

	ExitScope();
}
void LLVMCompilerImpl::CompileWhileStmt(WhileStmt *stmt)
{
}
void LLVMCompilerImpl::CompileReturnStmt(ReturnStmt *stmt)
{
	auto function = m_Builder->GetInsertBlock()->getParent();
	if (!stmt->expr)
		m_Builder->CreateRetVoid();
	CompileExpr(stmt->expr);
	m_Builder->CreateRet(Pop());
}
void LLVMCompilerImpl::CompileStructStmt(StructStmt *stmt)
{
}

void LLVMCompilerImpl::CompileExpr(Expr *expr, const RWState &state)
{
	switch (expr->type)
	{
	case AstType::NUM:
		CompileNumExpr((NumExpr *)expr);
		break;
	case AstType::STR:
		CompileStrExpr((StrExpr *)expr);
		break;
	case AstType::BOOL:
		CompileBoolExpr((BoolExpr *)expr);
		break;
	case AstType::NIL:
		CompileNilExpr((NilExpr *)expr);
		break;
	case AstType::IDENTIFIER:
		CompileIdentifierExpr((IdentifierExpr *)expr, state);
		break;
	case AstType::GROUP:
		CompileGroupExpr((GroupExpr *)expr);
		break;
	case AstType::ARRAY:
		CompileArrayExpr((ArrayExpr *)expr);
		break;
	case AstType::INDEX:
		CompileIndexExpr((IndexExpr *)expr);
		break;
	case AstType::PREFIX:
		CompilePrefixExpr((PrefixExpr *)expr);
		break;
	case AstType::INFIX:
		CompileInfixExpr((InfixExpr *)expr);
		break;
	case AstType::FUNCTION_CALL:
		CompileFunctionCallExpr((FunctionCallExpr *)expr);
		break;
	case AstType::STRUCT_CALL:
		CompileStructCallExpr((StructCallExpr *)expr, state);
		break;
	case AstType::REF:
		CompileRefExpr((RefExpr *)expr);
		break;
	case AstType::FUNCTION:
		CompileFunctionExpr((FunctionExpr *)expr);
		break;
	case AstType::STRUCT:
		CompileStructExpr((StructExpr *)expr);
		break;
	case AstType::DLL_IMPORT:
		CompileDllImportExpr((DllImportExpr *)expr);
		break;
	default:
		break;
	}
}
void LLVMCompilerImpl::CompileInfixExpr(InfixExpr *expr)
{
	if (expr->op == "=")
	{
		if (expr->left->type == AstType::IDENTIFIER && expr->right->type == AstType::FUNCTION)
			m_SymbolTable->Define(((IdentifierExpr *)expr->left)->literal);

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
			llvm::Value *resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

			auto fn = m_LlvmBuiltins["ValueAdd"];

			auto callInst = AddValueFnParamAttributes(m_Builder->CreateCall(fn, {lValue, rValue, resultAddr}));

			Push(resultAddr);
		}
		else if (expr->op == "-")
		{
			auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
			llvm::Value *resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

			auto fn = m_LlvmBuiltins["ValueSub"];

			auto callInst = m_Builder->CreateCall(fn, {lValue, rValue, resultAddr});
			callInst->addParamAttr(0, llvm::Attribute::NoUndef);
			callInst->addParamAttr(1, llvm::Attribute::NoUndef);
			callInst->addParamAttr(2, llvm::Attribute::NoUndef);

			Push(resultAddr);
		}
		else if (expr->op == "*")
		{
			auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
			llvm::Value *resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

			auto fn = m_LlvmBuiltins["ValueMul"];

			auto callInst = m_Builder->CreateCall(fn, {lValue, rValue, resultAddr});
			callInst->addParamAttr(0, llvm::Attribute::NoUndef);
			callInst->addParamAttr(1, llvm::Attribute::NoUndef);
			callInst->addParamAttr(2, llvm::Attribute::NoUndef);

			Push(resultAddr);
		}
		else if (expr->op == "/")
		{
			auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);
			llvm::Value *resultAddr = m_Builder->CreateInBoundsGEP(m_ValueType, result, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

			auto fn = m_LlvmBuiltins["ValueDiv"];

			auto callInst = AddValueFnParamAttributes(m_Builder->CreateCall(fn, {lValue, rValue, resultAddr}));

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
void LLVMCompilerImpl::CompileNumExpr(NumExpr *expr)
{
	auto vT = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NUM));
	auto d = llvm::ConstantFP::get(*m_Context, llvm::APFloat(expr->value));

	auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

	llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
	m_Builder->CreateStore(vT, memberAddr);
	llvm::Value* memberAddr1 = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(1) });
	memberAddr1 = m_Builder->CreateBitCast(memberAddr1, m_DoublePtrType);
	m_Builder->CreateStore(d, memberAddr1);

	Push(memberAddr);
}
void LLVMCompilerImpl::CompileBoolExpr(BoolExpr *expr)
{
	auto vT = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::BOOL));
	auto d = llvm::ConstantFP::get(*m_Context, llvm::APFloat(expr->value ? 1.0 : 0.0));

	auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

	llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
	m_Builder->CreateStore(vT, memberAddr);
	llvm::Value *memberAddr1 = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
	m_Builder->CreateStore(d, memberAddr1);

	Push(memberAddr);
}
void LLVMCompilerImpl::CompilePrefixExpr(PrefixExpr *expr)
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
void LLVMCompilerImpl::CompileStrExpr(StrExpr *expr)
{
	auto str = m_Builder->CreateGlobalString(expr->value);

	auto objAlloc = m_Builder->CreateAlloca(m_StrObjectType, nullptr);
	m_Builder->CreateMemSet(objAlloc, llvm::ConstantInt::get(m_Int8Type, llvm::APInt(8, 0)), 32, llvm::MaybeAlign(8));

	llvm::Value *objAddr0 = m_Builder->CreateInBoundsGEP(m_StrObjectType, objAlloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

	llvm::Value *objAddr1 = m_Builder->CreateInBoundsGEP(m_StrObjectType, objAlloc, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
	m_Builder->CreateStore(str, objAddr1);

	auto vT = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));

	auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);
	llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
	m_Builder->CreateStore(vT, memberAddr);
	llvm::Value *memberAddr1 = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
	m_Builder->CreateStore(objAddr0, memberAddr1);

	Push(memberAddr);
}
void LLVMCompilerImpl::CompileNilExpr(NilExpr *expr)
{
	auto vT = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NIL));
	auto d = llvm::ConstantFP::get(*m_Context, llvm::APFloat(0.0));

	auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

	llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
	m_Builder->CreateStore(vT, memberAddr);
	llvm::Value *memberAddr1 = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
	m_Builder->CreateStore(d, memberAddr1);

	Push(memberAddr);
}
void LLVMCompilerImpl::CompileGroupExpr(GroupExpr *expr)
{
	CompileExpr(expr->expr);
}
void LLVMCompilerImpl::CompileArrayExpr(ArrayExpr *expr)
{
}
void LLVMCompilerImpl::CompileIndexExpr(IndexExpr *expr)
{
}
void LLVMCompilerImpl::CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state)
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
			auto v = m_LlvmBuiltins[symbol.name];
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
			if (!isFound) // already exists like:a=10;{a=20;}
			{
				auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr, symbol.name.data());
				auto memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)}, symbol.name.data());

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
			if (!isFound) // already exists like:{a=10;{a=20;}}
			{
				auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr, symbol.name.data());
				auto memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)}, symbol.name.data());

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
void LLVMCompilerImpl::CompileFunctionExpr(FunctionExpr *expr)
{
	auto beforeblock = m_Builder->GetInsertBlock();

	EnterScope();

	for (const auto &param : expr->parameters)
		m_SymbolTable->Define(param->literal);

	std::vector<llvm::Type *> argTypes(expr->parameters.size(), llvm::Type::getDoubleTy(*m_Context));
	llvm::FunctionType *fnType = llvm::FunctionType::get(llvm::Type::getDoubleTy(*m_Context), argTypes, false);
	llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "", m_Module.get());

	unsigned idx = 0;
	for (auto &arg : fn->args())
		arg.setName(expr->parameters[idx++]->literal);

	llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_Context, "", fn);
	m_Builder->SetInsertPoint(block);

	CompileStmt(expr->body);

	ExitScope();

	m_Builder->SetInsertPoint(beforeblock);

	Push(fn);
}
void LLVMCompilerImpl::CompileFunctionCallExpr(FunctionCallExpr *expr)
{
	CompileExpr(expr->name);
	auto fn = static_cast<llvm::Function *>(Pop());
	if (!fn)
		ASSERT("Unknown function:%s", expr->name->Stringify().c_str());

	// if (fn->arg_size() != expr->arguments.size())
	//	ASSERT("InCompatible arg size:%d,%d", fn->arg_size(), expr->arguments.size());

	// builtin function
	{
		std::vector<llvm::Value *> argsV;
		for (unsigned i = 0; i < expr->arguments.size(); ++i)
		{
			CompileExpr(expr->arguments[i]);
			argsV.push_back(Pop());
			if (!argsV.back())
				return;
		}

		auto valueArrayType = llvm::ArrayType::get(m_ValueType, argsV.size());

		auto arg0 = m_Builder->CreateAlloca(valueArrayType, nullptr);
		llvm::Value *arg0MemberAddr = m_Builder->CreateInBoundsGEP(valueArrayType, arg0, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

		for (auto i = 0; i < argsV.size(); ++i)
		{
			if (i == 0)
			{
				m_Builder->CreateMemCpy(arg0MemberAddr, llvm::MaybeAlign(8), argsV[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
			}
			else
			{
				llvm::Value *argIMemberAddr = m_Builder->CreateInBoundsGEP(valueArrayType, arg0, {m_Builder->getInt32(0), m_Builder->getInt32(i)});
				m_Builder->CreateMemCpy(argIMemberAddr, llvm::MaybeAlign(8), argsV[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
			}
		}

		llvm::Value *arg1 = m_Builder->getInt8(argsV.size());

		auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);

		auto callInst = AddBuiltinFnOrCallParamAttributes( m_Builder->CreateCall(fn, { arg0MemberAddr, arg1, result}));

		Push(callInst);
	}
}
void LLVMCompilerImpl::CompileStructCallExpr(StructCallExpr *expr, const RWState &state)
{
}
void LLVMCompilerImpl::CompileRefExpr(RefExpr *expr)
{
}
void LLVMCompilerImpl::CompileStructExpr(StructExpr *expr)
{
}
void LLVMCompilerImpl::CompileDllImportExpr(DllImportExpr *expr)
{
}

void LLVMCompilerImpl::Push(llvm::Value *v)
{
	m_ValueStack.push_back(v);
}

llvm::Value *LLVMCompilerImpl::Peek(int32_t distance)
{
	return m_ValueStack[m_ValueStack.size() - 1 - distance];
}

llvm::Value *LLVMCompilerImpl::Pop()
{
	auto v = Peek(0);
	m_ValueStack.pop_back();
	return v;
}

llvm::Function *LLVMCompilerImpl::GetCurFunction()
{
	return PeekFunction(0);
}

void LLVMCompilerImpl::PushFunction(llvm::Function *fn)
{
	m_FunctionStack.push_back(fn);
}

llvm::Function *LLVMCompilerImpl::PeekFunction(int32_t distance)
{
	return m_FunctionStack[m_FunctionStack.size() - 1 - distance];
}

llvm::Function *LLVMCompilerImpl::PopFunction()
{
	auto f = PeekFunction(0);
	m_FunctionStack.pop_back();
	return f;
}

void LLVMCompilerImpl::EnterScope()
{
	m_SymbolTable = new SymbolTable(m_SymbolTable);
}
void LLVMCompilerImpl::ExitScope()
{
	m_SymbolTable = m_SymbolTable->enclosing;
}

void LLVMCompilerImpl::InitModuleAndPassManager()
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
		m_DoublePtrType = llvm::PointerType::get(m_DoubleType, 0);

		m_ObjectType = llvm::StructType::create(*m_Context, {m_Int8Type, m_Int8PtrType, m_Int8Type}, "struct.Object");
		m_ObjectPtrType = llvm::PointerType::get(m_ObjectType, 0);

		m_StrObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_Int8PtrType}, "struct.StrObject");
		m_StrObjectPtrType = llvm::PointerType::get(m_StrObjectType, 0);

		mUnionType = llvm::StructType::create(*m_Context, {m_DoubleType}, "union.anon");

		m_ValueType = llvm::StructType::create(*m_Context, {m_Int8Type, mUnionType}, "struct.Value");
		m_ValuePtrType = llvm::PointerType::get(m_ValueType, 0);

		m_BuiltinFunctionType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType, m_Int8Type, m_ValuePtrType}, false);

		m_ValueFunctionType = llvm::FunctionType::get(m_VoidType, {m_ValuePtrType, m_ValuePtrType, m_ValuePtrType}, false);
	}

	auto valueAddFn = AddValueFnParamAttributes(llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "ValueAdd", m_Module.get()));
	auto valueSubFn = AddValueFnParamAttributes(llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "ValueSub", m_Module.get()));
	auto valueMulFn = AddValueFnParamAttributes(llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "ValueMul", m_Module.get()));
	auto valueDivFn = AddValueFnParamAttributes(llvm::Function::Create(m_ValueFunctionType, llvm::Function::ExternalLinkage, "ValueDiv", m_Module.get()));

	auto printFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Print", m_Module.get()));
	auto printlnFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Println", m_Module.get()));
	auto sizeofFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Sizeof", m_Module.get()));
	auto insertFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Insert", m_Module.get()));
	auto eraseFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Erase", m_Module.get()));
	auto clockFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Clock", m_Module.get()));

	RegisterLlvmFn("ValueAdd", valueAddFn);
	RegisterLlvmFn("ValueSub", valueSubFn);
	RegisterLlvmFn("ValueMul", valueMulFn);
	RegisterLlvmFn("ValueDiv", valueDivFn);
	RegisterLlvmFn("print", printFn);
	RegisterLlvmFn("println", printlnFn);
	RegisterLlvmFn("sizeof", sizeofFn);
	RegisterLlvmFn("insert", insertFn);
	RegisterLlvmFn("erase", eraseFn);
	RegisterLlvmFn("clock", clockFn);

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

	llvm::FunctionType *fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_Context), false);
	llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "main", m_Module.get());
	llvm::BasicBlock *codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
	m_Builder->SetInsertPoint(codeBlock);

	PushFunction(fn);
}

void LLVMCompilerImpl::RegisterLlvmFn(std::string_view name, llvm::Function *fn)
{
	m_LlvmBuiltins[name.data()] = fn;
}
