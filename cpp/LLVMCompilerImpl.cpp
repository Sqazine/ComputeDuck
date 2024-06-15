#include "LLVMCompilerImpl.h"
#include "BuiltinManager.h"

LLVMCompilerImpl::LLVMCompilerImpl()
{
}

LLVMCompilerImpl::~LLVMCompilerImpl()
{
	SAFE_DELETE(m_SymbolTable);
}

void LLVMCompilerImpl::Compile(const std::vector<Stmt *> &stmts)
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
}

void LLVMCompilerImpl::Run()
{
	auto rt = m_Jit->GetMainJITDylib().createResourceTracker();

	m_Module->setDataLayout(m_Jit->GetDataLayout());

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
		else if (expr->op == "&")
		{
			auto lCast = m_Builder->CreateFPToSI(lValue, m_Int64Type);
			auto rCast = m_Builder->CreateFPToSI(rValue, m_Int64Type);
			auto result = m_Builder->CreateAnd(lCast, rCast);
			Push(result);
		}
		else if (expr->op == "|")
		{
			auto lCast = m_Builder->CreateFPToSI(lValue, m_Int64Type);
			auto rCast = m_Builder->CreateFPToSI(rValue, m_Int64Type);
			auto result = m_Builder->CreateOr(lCast, rCast);
			Push(result);
		}
		else if (expr->op == "^")
		{
			auto lCast = m_Builder->CreateFPToSI(lValue, m_Int64Type);
			auto rCast = m_Builder->CreateFPToSI(rValue, m_Int64Type);
			auto result = m_Builder->CreateXor(lCast, rCast);
			Push(result);
		}
	}
}
void LLVMCompilerImpl::CompileNumExpr(NumExpr *expr)
{
	auto alloc = llvm::ConstantFP::get(*m_Context, llvm::APFloat(expr->value));
	Push(alloc);
}
void LLVMCompilerImpl::CompileBoolExpr(BoolExpr *expr)
{
    auto alloc = llvm::ConstantInt::get(m_BoolType,expr->value);
    Push(alloc);
}
void LLVMCompilerImpl::CompilePrefixExpr(PrefixExpr *expr)
{
	CompileExpr(expr->right);
	auto rValue = Pop();
	if (expr->op == "-")
		Push(m_Builder->CreateNeg(rValue));
	else if (expr->op == "not")
		Push(m_Builder->CreateNot(rValue));
	else if (expr->op == "~")
	{
		rValue = m_Builder->CreateFPToSI(rValue, m_Int64Type);
		Push(m_Builder->CreateXor (rValue,-1));
	}
	else
		ASSERT("Unrecognized prefix op");
}
void LLVMCompilerImpl::CompileStrExpr(StrExpr *expr)
{
	// create str object
	auto strObject = CreateCDObject(expr->value);
	auto base = m_Builder->CreateBitCast(strObject, m_ObjectPtrType);

	Push(base);
}
void LLVMCompilerImpl::CompileNilExpr(NilExpr *expr)
{
	Push(llvm::ConstantPointerNull::get(m_Int8PtrType));
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
			Push(symbol.allocation);
			break;
		}
		case SymbolScope::LOCAL:
		{
			Push(symbol.allocation);
			break;
		}
		case SymbolScope::BUILTIN:
		{
			auto v = FindLlvmFn(symbol.name);
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

				m_SymbolTable->Set(symbol.name, alloc);

				Push(alloc);
			}
			else
			{
				auto memberAddr = symbol.allocation;
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

				m_SymbolTable->Set(symbol.name, alloc);

				Push(alloc);
			}
			else
			{
				auto alloc = symbol.allocation;
				m_Builder->CreateMemCpy(alloc, llvm::MaybeAlign(8), init, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
				Push(alloc);
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
			auto cdValue = CreateCDValue(Pop());
			argsV.push_back(cdValue);
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

		auto callInst = AddBuiltinFnOrCallParamAttributes(m_Builder->CreateCall(fn, {arg0MemberAddr, arg1, result}));

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

		m_ObjectType = llvm::StructType::create(*m_Context, "struct.Object");
		m_ObjectPtrType = llvm::PointerType::get(m_ObjectType, 0);
		m_ObjectType->setBody({m_Int8Type, m_BoolType, m_ObjectPtrType});
		m_ObjectPtrPtrType = llvm::PointerType::get(m_ObjectPtrType, 0);

		m_StrObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_Int8PtrType}, "struct.StrObject");
		m_StrObjectPtrType = llvm::PointerType::get(m_StrObjectType, 0);

		mUnionType = llvm::StructType::create(*m_Context, {m_DoubleType}, "union.anon");

		m_ValueType = llvm::StructType::create(*m_Context, {m_Int8Type, mUnionType}, "struct.Value");
		m_ValuePtrType = llvm::PointerType::get(m_ValueType, 0);

		m_BuiltinFunctionType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType, m_Int8Type, m_ValuePtrType}, false);

		m_ValueFunctionType = llvm::FunctionType::get(m_VoidType, {m_ValuePtrType, m_ValuePtrType, m_ValuePtrType}, false);
	}

	auto printFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Print", m_Module.get()));
	auto printlnFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Println", m_Module.get()));
	auto sizeofFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Sizeof", m_Module.get()));
	auto insertFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Insert", m_Module.get()));
	auto eraseFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Erase", m_Module.get()));
	auto clockFn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, "Clock", m_Module.get()));

	RegisterLlvmFn("print", printFn);
	RegisterLlvmFn("println", printlnFn);
	RegisterLlvmFn("sizeof", sizeofFn);
	RegisterLlvmFn("insert", insertFn);
	RegisterLlvmFn("erase", eraseFn);
	RegisterLlvmFn("clock", clockFn);

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
	m_LlvmBuiltins[name] = fn;
}

llvm::Function *LLVMCompilerImpl::FindLlvmFn(std::string_view name)
{
	auto iter = m_LlvmBuiltins.find(name);
	if (iter == m_LlvmBuiltins.end())
		ASSERT("No llvm fn:%s", name.data());
	return iter->second;
}

llvm::Value *LLVMCompilerImpl::CreateCDValue(llvm::Value *v)
{
	llvm::Value *vt = nullptr;
	llvm::Value *storedV = nullptr;
	llvm::Type* type = m_DoublePtrType;
	if (v->getType() == m_DoubleType)
	{
		vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NUM));
		storedV = v;
	}
	else if (v->getType() == m_Int64Type)
	{
		vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NUM));
		storedV = m_Builder->CreateSIToFP(v, m_DoubleType);
	}
	else if (v->getType() == m_BoolType)
	{
		vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::BOOL));
		storedV = m_Builder->CreateUIToFP(v, m_DoubleType);
	}
	else if (v->getType() == m_Int8PtrType)
	{
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NIL));
        storedV = llvm::ConstantFP::get(*m_Context, llvm::APFloat(0.0));
	}
	else if (v->getType() == m_ObjectPtrType)
	{
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));
		type = m_ObjectPtrPtrType;
		storedV = v;
	}

	auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

	llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
	m_Builder->CreateStore(vt, memberAddr);
	memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
	memberAddr = m_Builder->CreateBitCast(memberAddr, type);
	m_Builder->CreateStore(storedV, memberAddr);

	return alloc;
}

llvm::Value* LLVMCompilerImpl::CreateCDObject(const std::string& str)
{
    auto arrayType = llvm::ArrayType::get(m_Int8Type, str.size() + 1);
    auto globalChars = m_Builder->CreateGlobalString(str);

    auto globalCharsPtr = m_Builder->CreateInBoundsGEP(arrayType, globalChars, { m_Builder->getInt64(0), m_Builder->getInt64(0) });

    //create str object
    auto strObject = m_Builder->CreateAlloca(m_StrObjectType, nullptr);
    auto base = m_Builder->CreateBitCast(strObject, m_ObjectPtrType);

    auto objctTypeVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, { m_Builder->getInt32(0), m_Builder->getInt32(0) });
    m_Builder->CreateStore(m_Builder->getInt8(std::underlying_type<ObjectType>::type(ObjectType::STR)), objctTypeVar);

    auto markedVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, { m_Builder->getInt32(0), m_Builder->getInt32(1) });
    m_Builder->CreateStore(m_Builder->getInt1(0), markedVar);

    auto nextVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, { m_Builder->getInt32(0), m_Builder->getInt32(2) });
    m_Builder->CreateStore(llvm::ConstantPointerNull::get(m_ObjectPtrType), nextVar);

    auto strPtr = m_Builder->CreateInBoundsGEP(m_StrObjectType, strObject, { m_Builder->getInt32(0), m_Builder->getInt32(1) });
    m_Builder->CreateStore(globalCharsPtr, strPtr);
	return strObject;
}
