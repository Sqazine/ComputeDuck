#include "LLVMJitVM.h"
#include "BuiltinManager.h"

LLVMJitVM::OrcJit::OrcJit(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl)
    : m_Es(std::move(es)), m_DataLayout(std::move(dl)), m_Mangle(*m_Es, m_DataLayout),
      m_ObjectLayer(*m_Es,
                    []()
                    { return std::make_unique<llvm::SectionMemoryManager>(); }),
      m_CompileLayer(*m_Es, m_ObjectLayer,
                     std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(jtmb))),
      m_MainJD(m_Es->createBareJITDylib("<main>"))
{
    m_MainJD.addGenerator(cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(m_DataLayout.getGlobalPrefix())));

    if (jtmb.getTargetTriple().isOSBinFormatCOFF())
    {
        m_ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
        m_ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }
}

LLVMJitVM::OrcJit::~OrcJit()
{
    if (auto Err = m_Es->endSession())
        m_Es->reportError(std::move(Err));
}

std::unique_ptr<LLVMJitVM::OrcJit> LLVMJitVM::OrcJit::Create()
{
    auto epc = llvm::orc::SelfExecutorProcessControl::Create();
    if (!epc)
        return nullptr;

    auto es = std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));

    llvm::orc::JITTargetMachineBuilder JTMB(es->getExecutorProcessControl().getTargetTriple());

    auto dataLayout = JTMB.getDefaultDataLayoutForTarget();
    if (!dataLayout)
        return nullptr;

    return std::make_unique<LLVMJitVM::OrcJit>(std::move(es), std::move(JTMB), std::move(*dataLayout));
}

const llvm::DataLayout &LLVMJitVM::OrcJit::GetDataLayout() const
{
    return m_DataLayout;
}

llvm::orc::JITDylib &LLVMJitVM::OrcJit::GetMainJITDylib()
{
    return m_MainJD;
}

llvm::Error LLVMJitVM::OrcJit::AddModule(llvm::orc::ThreadSafeModule tsm, llvm::orc::ResourceTrackerSP rt)
{
    if (!rt)
        rt = m_MainJD.getDefaultResourceTracker();
    return m_CompileLayer.add(rt, std::move(tsm));
}

llvm::Expected<llvm::JITEvaluatedSymbol> LLVMJitVM::OrcJit::LookUp(llvm::StringRef name)
{
    return m_Es->lookup({&m_MainJD}, m_Mangle(name.str()));
}

LLVMJitVM::LLVMJitVM()
{
}

LLVMJitVM::~LLVMJitVM()
{
}

void LLVMJitVM::Run(FunctionObject* mainFn)
{
    ResetStatus();

    llvm::FunctionType* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_Context), false);
    llvm::Function* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "main", m_Module.get());
    llvm::BasicBlock* codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
    m_Builder->SetInsertPoint(codeBlock);

    m_CallFrameTop = m_CallFrameStack;
    m_StackTop = m_ValueStack;
    auto mainCallFrame = CallFrame(mainFn, fn, m_StackTop);

    CompileToLLVMIR(mainCallFrame);

#ifndef NDEBUG
    m_Module->print(llvm::outs(), nullptr);
#endif

    // run function opt pass
    {
        fn = PopCallFrame()->llvmFn;
        m_Builder->CreateRet(llvm::ConstantPointerNull::get(m_ValuePtrType));

        auto b = llvm::verifyFunction(*fn);
        m_FPM->run(*fn);
    }
    // run jit
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
}

void LLVMJitVM::ResetStatus()
{
    m_BuiltinFnCache.clear();

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    m_Jit = OrcJit::Create();

    InitModuleAndPassManager();
}

void LLVMJitVM::CompileToLLVMIR(const CallFrame& callFrame)
{
    PushCallFrame(callFrame);

    while (true)
    {
        auto frame = PeekCallFrame(1);

        if (frame->IsEnd())
            return;

        int32_t instruction = *frame->ip++;
        switch (instruction)
        {
        case OP_CONSTANT:
        {
            auto idx = *frame->ip++;
            auto value = frame->fn->chunk.constants[idx];

            if (IS_NUM_VALUE(value))
            {
                llvm::Value *alloc = llvm::ConstantFP::get(*m_Context, llvm::APFloat(TO_NUM_VALUE(value)));
                Push(alloc);
            }
            else if (IS_BOOL_VALUE(value))
            {
                llvm::Value *alloc = llvm::ConstantInt::get(m_BoolType, TO_BOOL_VALUE(value));
                Push(alloc);
            }
            else if (IS_NIL_VALUE(value))
            {
                llvm::Value *alloc = llvm::ConstantPointerNull::get(m_Int8PtrType);
                Push(alloc);
            }
            else if (IS_STR_VALUE(value))
            {
                Push(value);
            }

            break;
        }
        case OP_ADD:
        {
            auto left = Pop();
            auto right = Pop();
            llvm::Value* result = nullptr;
            if (left.Is<llvm::Value*>() && right.Is<llvm::Value*>())
            {
                auto leftValue = left.Get<llvm::Value*>();
                auto rightValue = right.Get<llvm::Value*>();
                if (leftValue->getType() == m_DoubleType && rightValue->getType() == m_DoubleType)
                {
                    result = m_Builder->CreateFAdd(leftValue, rightValue);
                    Push(result);
                }
                else
                    ASSERT("Invalid binary op:(Type)+(Type),Only (double)+(double) or (string)+(string) is available.");
            }
            else if (left.Is<Value>() && right.Is<Value>())
            {
                auto leftValue = left.Get<Value>();
                auto rightValue = right.Get<Value>();
                if (IS_STR_VALUE(leftValue) && IS_STR_VALUE(rightValue))
                    Push(Value(StrAdd(TO_STR_VALUE(leftValue), TO_STR_VALUE(rightValue))));
                else
                    ASSERT("Invalid binary op:(Type)+(Type),Only (double)+(double) or (string)+(string) is available.");
            }
            else
                ASSERT("Invalid binary op:(Type)+(Type),Only (double)+(double) or (string)+(string) is available.");
            break;
        }
        case OP_SUB:
        {
            auto left = Pop().Get<llvm::Value *>();
            auto right = Pop().Get<llvm::Value *>();
            auto result = m_Builder->CreateFSub(left, right);
            Push(result);
            break;
        }
        case OP_MUL:
        {
            auto left = Pop().Get<llvm::Value *>();
            auto right = Pop().Get<llvm::Value *>();
            auto result = m_Builder->CreateFMul(left, right);
            Push(result);
            break;
        }
        case OP_DIV:
        {
            auto left = Pop().Get<llvm::Value *>();
            auto right = Pop().Get<llvm::Value *>();
            auto result = m_Builder->CreateFDiv(left, right);
            Push(result);
            break;
        }
        case OP_LESS:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto result = m_Builder->CreateFCmpULT(left, right);
            Push(result);
            break;
        }
        case OP_GREATER:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto result = m_Builder->CreateFCmpUGT(left, right);
            Push(result);
            break;
        }
        case OP_NOT:
        {
            auto value = Pop().Get<llvm::Value*>();
            auto result = m_Builder->CreateNot(value);
            Push(result);
            break;
        }
        case OP_MINUS:
        {
            auto value = Pop().Get<llvm::Value*>();
            auto result = m_Builder->CreateNeg(value);
            Push(result);
            break;
        }
        case OP_EQUAL:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto result = m_Builder->CreateFCmpUEQ(left, right);
            Push(result);
            break;
        }
        case OP_ARRAY:
        {
            break;
        }
        case OP_AND:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto result = m_Builder->CreateLogicalAnd(left, right);
            Push(result);
            break;
        }
        case OP_OR:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto result = m_Builder->CreateLogicalAnd(left, right);
            Push(result);
            break;
        }
        case OP_BIT_AND:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
            auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
            auto result = m_Builder->CreateAnd(lCast, rCast);
            Push(result);
            break;
        }
        case OP_BIT_OR:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
            auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
            auto result = m_Builder->CreateOr(lCast, rCast);
            Push(result);
            break;
        }
        case OP_BIT_NOT:
        {
            auto value= Pop().Get<llvm::Value*>();
            value = m_Builder->CreateFPToSI(value, m_Int64Type);
            Push(m_Builder->CreateXor(value, -1));
            break;
        }
        case OP_BIT_XOR:
        {
            auto left = Pop().Get<llvm::Value*>();
            auto right = Pop().Get<llvm::Value*>();
            auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
            auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
            auto result = m_Builder->CreateXor(lCast, rCast);
            Push(result);
            break;
        }
        case OP_INDEX:
        {
            break;
        }
        case OP_JUMP:
        {
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            break;
        }
        case OP_RETURN:
        {
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = *frame->ip++;
            auto v = Pop();

            m_GlobalVariables[index] = v;

            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *frame->ip++;
            Push(m_GlobalVariables[index]);
            break;
        }
        case OP_SET_LOCAL:
        {
            break;
        }
        case OP_GET_LOCAL:
        {
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = (uint8_t)*frame->ip++;
            auto fn = static_cast<llvm::Function *>((m_StackTop - argCount - 1)->Get<llvm::Value *>());

            // now only builtin fn
            std::vector<llvm::Value *> argsV;
            for (auto slot = m_StackTop - argCount; slot != m_StackTop; ++slot)
            {
                llvm::Value* arg=nullptr;
                if (slot->Is<Value>())
                {
                    auto v = slot->Get<Value>();
                    if (IS_STR_VALUE(v))
                    {
                        auto strObject = CreateCDObject(TO_STR_VALUE(v)->value);
                        auto base = m_Builder->CreateBitCast(strObject, m_ObjectPtrType);
                        arg = CreateCDValue(base);
                    }
                }
                else
                    arg = CreateCDValue(slot->Get<llvm::Value *>());
                argsV.emplace_back(arg);
            }

            auto valueArrayType = llvm::ArrayType::get(m_ValueType, argsV.size());

            auto arg0 = m_Builder->CreateAlloca(valueArrayType, nullptr);
            llvm::Value *arg0MemberAddr = m_Builder->CreateInBoundsGEP(valueArrayType, arg0, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

            for (auto i = 0; i < argsV.size(); ++i)
            {
                if (i == 0)
                    m_Builder->CreateMemCpy(arg0MemberAddr, llvm::MaybeAlign(8), argsV[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
                else
                {
                    llvm::Value *argIMemberAddr = m_Builder->CreateInBoundsGEP(valueArrayType, arg0, {m_Builder->getInt32(0), m_Builder->getInt32(i)});
                    m_Builder->CreateMemCpy(argIMemberAddr, llvm::MaybeAlign(8), argsV[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
                }
            }

            llvm::Value *arg1 = m_Builder->getInt8(argsV.size());

            auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);

            auto callInst = AddBuiltinFnOrCallParamAttributes(m_Builder->CreateCall(fn, {arg0MemberAddr, arg1, result}));

            Push(result);

            break;
        }
        case OP_GET_BUILTIN:
        {
            auto name = TO_STR_VALUE(Pop().Get<Value>())->value;
            std::string namrStr = name;
            auto iter = m_BuiltinFnCache.find(name);

            llvm::Function *fn = nullptr;
            if (iter == m_BuiltinFnCache.end())
            {
                fn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, BUILTIN_FN_PREFIX_STR + namrStr, m_Module.get()));
                m_BuiltinFnCache[name] = fn;
            }
            else
                fn = iter->second;

            Push(fn);
            break;
        }
        case OP_STRUCT:
        {
            break;
        }
        case OP_GET_STRUCT:
        {
            break;
        }
        case OP_SET_STRUCT:
        {
            break;
        }
        case OP_REF_GLOBAL:
        {
            break;
        }
        case OP_REF_LOCAL:
        {
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            break;
        }
        case OP_REF_INDEX_LOCAL:
        {
            break;
        }
        case OP_SP_OFFSET:
        {
            break;
        }
        default:
            break;
        }
    }
}

void LLVMJitVM::InitModuleAndPassManager()
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
}

llvm::Value *LLVMJitVM::CreateCDValue(llvm::Value *v)
{
    llvm::Value *vt = nullptr;
    llvm::Value *storedV = nullptr;
    llvm::Type *type = m_DoublePtrType;
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

llvm::Value *LLVMJitVM::CreateCDObject(const std::string &str)
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

void LLVMJitVM::Push(const JitValue &v)
{
    *m_StackTop++ = v;
}

LLVMJitVM::JitValue LLVMJitVM::Pop()
{
    return *(--m_StackTop);
}

void LLVMJitVM::PushCallFrame(const CallFrame &callFrame)
{
    *m_CallFrameTop++ = callFrame;
}

LLVMJitVM::CallFrame *LLVMJitVM::PopCallFrame()
{
    return --m_CallFrameTop;
}

LLVMJitVM::CallFrame *LLVMJitVM::PeekCallFrame(int32_t distance)
{
    return m_CallFrameTop - distance;
}