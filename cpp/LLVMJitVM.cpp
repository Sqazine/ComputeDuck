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

void LLVMJitVM::Run(FunctionObject *mainFn)
{
    ResetStatus();

    llvm::FunctionType *fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_Context), false);
    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "main", m_Module.get());
    llvm::BasicBlock *codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
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

    for (auto& g : m_GlobalVariables)
        g = nullptr;

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    m_Jit = OrcJit::Create();

    InitModuleAndPassManager();
}

void LLVMJitVM::CompileToLLVMIR(const CallFrame &callFrame)
{

    enum class State
    {
        CONDITION,
        THEN,
        ELSE,
        END
    };

    static State state = State::CONDITION;

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
                llvm::Value *alloc = llvm::ConstantFP::get(m_DoubleType, TO_NUM_VALUE(value));
                Push(alloc);
            }
            else if (IS_BOOL_VALUE(value))
            {
                llvm::Value *alloc = llvm::ConstantInt::get(m_BoolType, TO_BOOL_VALUE(value));
                Push(alloc);
            }
            else if (IS_NIL_VALUE(value))
            {
                llvm::Value *alloc = llvm::ConstantPointerNull::get(m_BoolPtrType);
                Push(alloc);
            }
            else if (IS_STR_VALUE(value))
            {
                auto str = TO_STR_VALUE(value)->value;
                llvm::Value *chars = m_Builder->CreateGlobalString(str);
                Push(chars);
            }

            break;
        }
        case OP_ADD:
        {
            auto left = Pop();
            auto right = Pop();
            llvm::Value *result = nullptr;

            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFAdd(left, right));
            else if (left->getType()->isPointerTy() && right->getType()->isPointerTy())
            {
                auto leftPtrType = static_cast<llvm::PointerType *>(left->getType());
                auto rightPtrType = static_cast<llvm::PointerType *>(right->getType());

                if (leftPtrType->getElementType()->isArrayTy() && rightPtrType->getElementType()->isArrayTy())
                {
                    auto leftArrayType = static_cast<llvm::ArrayType *>(leftPtrType->getElementType());
                    auto rightArrayType = static_cast<llvm::ArrayType *>(rightPtrType->getElementType());
                    if (leftArrayType->getElementType() == m_Int8Type && rightArrayType->getElementType() == m_Int8Type)
                    {
                        // convert chars[] to i8*
                        auto leftCharsPtr = m_Builder->CreateInBoundsGEP(leftArrayType, left, {m_Builder->getInt64(0), m_Builder->getInt64(0)});
                        auto rightCharsPtr = m_Builder->CreateInBoundsGEP(rightArrayType, right, {m_Builder->getInt64(0), m_Builder->getInt64(0)});

                        auto leftNum = leftArrayType->getNumElements() - 1;
                        auto rightNum = rightArrayType->getNumElements() - 1;
                        auto totalNum = leftNum + rightNum + 1;
                        auto arrayType = llvm::ArrayType::get(m_Int8Type, totalNum);

                        auto alloc = m_Builder->CreateAlloca(arrayType);

                        auto firstPartPtr = m_Builder->CreateInBoundsGEP(arrayType, alloc, {m_Builder->getInt64(0), m_Builder->getInt64(0)});
                        m_Builder->CreateMemCpy(firstPartPtr, llvm::MaybeAlign(8), leftCharsPtr, llvm::MaybeAlign(8), m_Builder->getInt64(leftNum));

                        auto secondPartPtr = m_Builder->CreateInBoundsGEP(arrayType, alloc, {m_Builder->getInt64(0), m_Builder->getInt64(leftNum)});
                        m_Builder->CreateMemCpy(secondPartPtr, llvm::MaybeAlign(8), rightCharsPtr, llvm::MaybeAlign(8), m_Builder->getInt64(rightNum));

                        auto lastPartPtr = m_Builder->CreateInBoundsGEP(arrayType, alloc, {m_Builder->getInt64(0), m_Builder->getInt64(totalNum - 1)});
                        m_Builder->CreateStore(m_Builder->getInt8(0), lastPartPtr);

                        Push(alloc);
                    }
                }
            }
            else
                ASSERT("Invalid binary op:%s + %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

            break;
        }
        case OP_SUB:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFSub(left, right));
            else
                ASSERT("Invalid binary op:%s - %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_MUL:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFMul(left, right));
            else
                ASSERT("Invalid binary op:%s * %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_DIV:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFDiv(left, right));
            else
                ASSERT("Invalid binary op:%s / %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_LESS:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpULT(left, right));
            else
                ASSERT("Invalid binary op:%s < %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_GREATER:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpUGT(left, right));
            else
                ASSERT("Invalid binary op:%s > %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_NOT:
        {
            auto value = Pop();
            if (value->getType() == m_BoolType)
                Push(m_Builder->CreateNot(value));
            else
                ASSERT("Invalid binary op:not %s.", GetTypeName(value->getType()).c_str());
            break;
        }
        case OP_MINUS:
        {
            auto value = Pop();
            if (value->getType() == m_DoubleType)
                Push(m_Builder->CreateNeg(value));
            else
                ASSERT("Invalid binary op:- %s.", GetTypeName(value->getType()).c_str());
            break;
        }
        case OP_EQUAL:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == right->getType()) 
            {
                if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                    Push(m_Builder->CreateFCmpUEQ(left, right));
                else if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                    Push(m_Builder->CreateICmpEQ(left,right));
                else if (left->getType()->isPointerTy() && right->getType()->isPointerTy())
                {
                    auto leftPtrType = static_cast<llvm::PointerType*>(left->getType());
                    auto rightPtrType = static_cast<llvm::PointerType*>(right->getType());

                    if (leftPtrType->getElementType()->isArrayTy() && rightPtrType->getElementType()->isArrayTy())
                    {
                        auto leftArrayType = static_cast<llvm::ArrayType*>(leftPtrType->getElementType());
                        auto rightArrayType = static_cast<llvm::ArrayType*>(rightPtrType->getElementType());
                        if (leftArrayType->getElementType() == m_Int8Type && rightArrayType->getElementType() == m_Int8Type)
                        {
                            // convert chars[] to i8*
                            auto leftCharsPtr = m_Builder->CreateInBoundsGEP(leftArrayType, left, { m_Builder->getInt64(0), m_Builder->getInt64(0) });
                            auto rightCharsPtr = m_Builder->CreateInBoundsGEP(rightArrayType, right, { m_Builder->getInt64(0), m_Builder->getInt64(0) });

                            auto fnType = llvm::FunctionType::get(m_Int32Type, { m_Int8PtrType, m_Int8PtrType }, false);
                            auto fn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(fnType, llvm::Function::ExternalLinkage,"strcmp", m_Module.get()));

                            auto result = m_Builder->CreateCall(fn, {leftCharsPtr,rightCharsPtr});

                            Push(m_Builder->CreateICmpEQ(result, llvm::ConstantInt::get(m_Int32Type,0)));
                        }
                    }
                }
            }
            else
                ASSERT("Invalid binary op:%s == %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_ARRAY:
        {
            auto numElements = *frame->ip++;

            auto elements = std::vector<llvm::Value*>(numElements);

            int32_t i = numElements - 1;
            for (llvm::Value** p = m_StackTop - 1; p >= m_StackTop - numElements && i >= 0; --p, --i)
            {
                if ((*p)->getType() != m_ValuePtrType)
                {
                    llvm::Value* arg = CreateCDValue(*p);
                    elements[i] = arg;
                }
                else
                    elements[i] = *p;
            }

            auto arrayType = llvm::ArrayType::get(m_ValueType, numElements);
            llvm::Value* alloc = m_Builder->CreateAlloca(arrayType);

            llvm::Value* memberAddr = m_Builder->CreateInBoundsGEP(arrayType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(0) });

            for (auto i = 0; i < elements.size(); ++i)
            {
                if (i == 0)
                    m_Builder->CreateMemCpy(memberAddr, llvm::MaybeAlign(8), elements[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
                else
                {
                    llvm::Value* argIMemberAddr = m_Builder->CreateInBoundsGEP(arrayType, alloc, { m_Builder->getInt32(0), m_Builder->getInt32(i) });
                    m_Builder->CreateMemCpy(argIMemberAddr, llvm::MaybeAlign(8), elements[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
                }
            }

            //alloc = CreateCDValue(alloc);

            m_StackTop -= numElements;

            Push(alloc);
            break;
        }
        case OP_AND:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                Push(m_Builder->CreateLogicalAnd(left, right));
            else
                ASSERT("Invalid binary op:%s and %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_OR:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                Push(m_Builder->CreateLogicalAnd(left, right));
            else
                ASSERT("Invalid binary op:%s or %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_BIT_AND:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
            {
                auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
                auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
                auto result = m_Builder->CreateAnd(lCast, rCast);
                Push(result);
            }
            else
                ASSERT("Invalid binary op:%s & %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_BIT_OR:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
            {
                auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
                auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
                auto result = m_Builder->CreateOr(lCast, rCast);
                Push(result);
            }
            else
                ASSERT("Invalid binary op:%s | %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_BIT_NOT:
        {
            auto value = Pop();
            if (value->getType() == m_BoolType)
            {
                value = m_Builder->CreateFPToSI(value, m_Int64Type);
                Push(m_Builder->CreateXor(value, -1));
            }
            else
                ASSERT("Invalid binary op:~ %s.", GetTypeName(value->getType()).c_str());
            break;
        }
        case OP_BIT_XOR:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_BoolType)
            {
                auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
                auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
                auto result = m_Builder->CreateXor(lCast, rCast);
                Push(result);
            }
            else
                ASSERT("Invalid binary op:%s ^ %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_GET_INDEX:
        {
            auto index = Pop();
            auto ds = Pop();

            bool isSatis = false;

            if (ds->getType()->isPointerTy())
            {
                auto vArrayType = static_cast<llvm::PointerType*>(ds->getType())->getElementType();
                if (vArrayType->isArrayTy())
                {
                    if (index->getType() == m_DoubleType)
                    {
                        isSatis = true;

                        auto iIndex = m_Builder->CreateFPToSI(index,m_Int64Type);

                        llvm::Value* memberAddr = m_Builder->CreateInBoundsGEP(vArrayType, ds, { m_Builder->getInt32(0), iIndex });

                        Push(memberAddr);
                    }
                }
            }

            if(!isSatis)
                ASSERT("Invalid index op: %s[%s]", GetTypeName(ds->getType()), GetTypeName(index->getType()));

            break;
        }
        case OP_JUMP_START:
        {
            auto fn = m_Builder->GetInsertBlock()->getParent();

            IfElse br;
            br.conditionAddr = frame->GetAddr();

            br.conditionBranch = llvm::BasicBlock::Create(*m_Context, "if.condition." + std::to_string(br.conditionAddr), fn);
            br.thenBranch = llvm::BasicBlock::Create(*m_Context, "if.then." + std::to_string(br.conditionAddr), fn);
            br.elseBranch = llvm::BasicBlock::Create(*m_Context, "if.else." + std::to_string(br.conditionAddr), fn);
            br.endBranch = llvm::BasicBlock::Create(*m_Context, "if.end." + std::to_string(br.conditionAddr));

            m_IfElseTable.emplace_back(br);

            m_Builder->CreateBr(br.conditionBranch);

            m_Builder->SetInsertPoint(br.conditionBranch);

            state = State::CONDITION;
            break;
        }
        case OP_JUMP_COMPARE:
        {
            auto address = *frame->ip++;
            auto condition = Pop();

            auto conditionValue=m_Builder->CreateICmpEQ(condition, llvm::ConstantInt::get(m_BoolType, true));

            auto& br = m_IfElseTable.back();
            br.thenAddr = frame->GetAddr();
            br.elseAddr = address;

            m_Builder->CreateCondBr(conditionValue, br.thenBranch, br.elseBranch);

            m_Builder->SetInsertPoint(br.thenBranch);
            state = State::THEN;
            break;
        }
        case OP_JUMP:
        {
            auto address = *frame->ip++;
            auto& br = m_IfElseTable.back();
            br.endAddr = address;

            if (br.endBranch->getParent() == nullptr)
            {
                // set endBranch parent
                auto fn = m_Builder->GetInsertBlock()->getParent();
                fn->getBasicBlockList().push_back(br.endBranch);
            }

            if(state==State::THEN)
            m_Builder->CreateBr(br.endBranch);

            m_Builder->SetInsertPoint(br.elseBranch);
            state = State::ELSE;

            break;
        }
        case OP_JUMP_END:
        {
            auto& br = m_IfElseTable.back();
            if (br.endBranch->getParent() == nullptr)
            {
                // set endBranch parent
                auto fn = m_Builder->GetInsertBlock()->getParent();
                fn->getBasicBlockList().push_back(br.endBranch);
            }

            if (state == State::ELSE)
                m_Builder->CreateBr(br.endBranch);// create jump br for else branch;

            m_Builder->SetInsertPoint(br.endBranch);
            state = State::END;

            {
                auto offset = m_IfElseTable.size() - 1;
                if (offset > 0)
                    m_Builder->CreateBr(m_IfElseTable[offset - 1].endBranch);
            }

            m_IfElseTable.pop_back();

            break;
        }
        case OP_LOOP_START:
        {
            auto fn = m_Builder->GetInsertBlock()->getParent();

            auto curAddress = frame->GetAddr();

            WhileLoop br;
            br.conditionBranch = llvm::BasicBlock::Create(*m_Context, "loop.condition." + std::to_string(curAddress), fn);
            br.loopBodyBranch = llvm::BasicBlock::Create(*m_Context, "loop.body." + std::to_string(curAddress), fn);
            br.endBranch = llvm::BasicBlock::Create(*m_Context, "loop.end." + std::to_string(curAddress));

            m_LoopTable.emplace_back(br);

            m_Builder->CreateBr(br.conditionBranch);

            m_Builder->SetInsertPoint(br.conditionBranch);
            break;
        }
        case OP_LOOP_COMPARE:
        {
            auto address = *frame->ip++;
            auto condition = Pop();

            auto conditionValue = m_Builder->CreateICmpEQ(condition, llvm::ConstantInt::get(m_BoolType, true));

            auto br = m_LoopTable.back();

            m_Builder->CreateCondBr(conditionValue, br.loopBodyBranch, br.endBranch);

            m_Builder->SetInsertPoint(br.loopBodyBranch);
            break;
        }
        case OP_LOOP_END:
        {
            auto address = *frame->ip++;

            auto br = m_LoopTable.back();

            // set endBranch parent
            auto fn = m_Builder->GetInsertBlock()->getParent();
            fn->getBasicBlockList().push_back(br.endBranch);

            m_Builder->CreateBr(br.conditionBranch);
            m_Builder->SetInsertPoint(br.endBranch);
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

            auto& ref = m_GlobalVariables[index];

            if (ref == nullptr)
            {
                auto alloc = m_Builder->CreateAlloca(v->getType(), nullptr, "globalVar_" + std::to_string(index));
                m_Builder->CreateStore(v, alloc);

                ref = alloc;
            }
            else
            {
                auto value = m_Builder->CreateStore(v,ref);
            }

            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *frame->ip++;

            auto stored = m_GlobalVariables[index];

            if (stored->getType()->isPointerTy())
            {
                auto ptrType = static_cast<llvm::PointerType*>(stored->getType());
                auto v = m_Builder->CreateLoad(ptrType->getElementType(),stored);
                Push(v);
            }

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
            auto fn = static_cast<llvm::Function *>(*(m_StackTop - argCount - 1));

            // now only builtin fn
            std::vector<llvm::Value *> argsV;
            for (auto slot = m_StackTop - argCount; slot != m_StackTop; ++slot)
            {
                if ((*slot)->getType() != m_ValuePtrType)
                {
                    llvm::Value *arg = CreateCDValue(*slot);
                    argsV.emplace_back(arg);
                }
                else
                    argsV.emplace_back(*slot);

            }

            auto valueArrayType = llvm::ArrayType::get(m_ValueType, argsV.size());

            auto arg0 = m_Builder->CreateAlloca(valueArrayType);
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
            auto idx = *frame->ip++;
            auto value = frame->fn->chunk.constants[idx];
            auto name = TO_STR_VALUE(value)->value;
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
            auto offset = *frame->ip++;
            m_StackTop += offset;
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

        m_Int8PtrPtrType = llvm::PointerType::get(m_Int8PtrType, 0);

        mUnionType = llvm::StructType::create(*m_Context, {m_DoubleType}, "union.anon");

        m_ValueType = llvm::StructType::create(*m_Context, {m_Int8Type, mUnionType}, "struct.Value");
        m_ValuePtrType = llvm::PointerType::get(m_ValueType, 0);

        m_ObjectType = llvm::StructType::create(*m_Context, "struct.Object");
        m_ObjectPtrType = llvm::PointerType::get(m_ObjectType, 0);
        m_ObjectType->setBody({m_Int8Type, m_BoolType, m_ObjectPtrType});
        m_ObjectPtrPtrType = llvm::PointerType::get(m_ObjectPtrType, 0);

        m_StrObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_Int8PtrType, m_Int32Type}, "struct.StrObject");
        m_StrObjectPtrType = llvm::PointerType::get(m_StrObjectType, 0);
       
        m_ArrayObjectType = llvm::StructType::create(*m_Context,{m_ObjectType,m_ValuePtrType,m_Int32Type},"struct.ArrayObject");
        m_ArrayObjectPtrType = llvm::PointerType::get(m_ArrayObjectType, 0);

        m_BuiltinFunctionType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType, m_Int8Type, m_ValuePtrType}, false);
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
    else if (v->getType() == m_BoolPtrType)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NIL));
        storedV = llvm::ConstantFP::get(m_DoubleType, 0.0);
    }
    else if (v->getType() == m_ObjectPtrType)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));
        type = m_ObjectPtrPtrType;
        storedV = v;
    }
    else if (v->getType()->isPointerTy())
    {
        auto vPtrType = static_cast<llvm::PointerType *>(v->getType());
        if (vPtrType->getElementType()->isArrayTy())
        {
            auto vArrayType = static_cast<llvm::ArrayType *>(vPtrType->getElementType());
            if (vArrayType->getElementType() == m_Int8Type)
            {
                vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));
                type = m_ObjectPtrPtrType;

                // convert chars[] to i8*
                auto charsPtr = m_Builder->CreateInBoundsGEP(vArrayType, v, {m_Builder->getInt64(0), m_Builder->getInt64(0)});

                //create str object
                auto strObject = m_Builder->CreateAlloca(m_StrObjectType, nullptr);
                auto base = m_Builder->CreateBitCast(strObject, m_ObjectPtrType);

                {
                    auto objctTypeVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
                    m_Builder->CreateStore(m_Builder->getInt8(std::underlying_type<ObjectType>::type(ObjectType::STR)), objctTypeVar);

                    auto markedVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                    m_Builder->CreateStore(m_Builder->getInt1(0), markedVar);

                    auto nextVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, {m_Builder->getInt32(0), m_Builder->getInt32(2)});
                    m_Builder->CreateStore(llvm::ConstantPointerNull::get(m_ObjectPtrType), nextVar);

                    auto strPtr = m_Builder->CreateInBoundsGEP(m_StrObjectType, strObject, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                    m_Builder->CreateStore(charsPtr, strPtr);

                    auto len = m_Builder->CreateInBoundsGEP(m_StrObjectType, strObject, {m_Builder->getInt32(0), m_Builder->getInt32(2)});
                    m_Builder->CreateStore(m_Builder->getInt32(vArrayType->getArrayNumElements() - 1), len); //-1 for ignoring the end char '\0'
                }

                storedV = base;
            }
            else if (vArrayType->getElementType() == m_ValueType)
            {
                auto numCount = vArrayType->getArrayNumElements();

                vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));
                type = m_ObjectPtrPtrType;

                // convert Value[] to Value*
                auto valuesPtr = m_Builder->CreateInBoundsGEP(vArrayType, v, { m_Builder->getInt64(0), m_Builder->getInt64(0) });

                //create array object
                auto arrayObject = m_Builder->CreateAlloca(m_ArrayObjectType, nullptr);
                auto base = m_Builder->CreateBitCast(arrayObject, m_ObjectPtrType);

                {
                    auto objctTypeVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, { m_Builder->getInt32(0), m_Builder->getInt32(0) });
                    m_Builder->CreateStore(m_Builder->getInt8(std::underlying_type<ObjectType>::type(ObjectType::ARRAY)), objctTypeVar);

                    auto markedVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, { m_Builder->getInt32(0), m_Builder->getInt32(1) });
                    m_Builder->CreateStore(m_Builder->getInt1(0), markedVar);

                    auto nextVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, { m_Builder->getInt32(0), m_Builder->getInt32(2) });
                    m_Builder->CreateStore(llvm::ConstantPointerNull::get(m_ObjectPtrType), nextVar);

                    auto valuePtr = m_Builder->CreateInBoundsGEP(m_ArrayObjectType, arrayObject, { m_Builder->getInt32(0), m_Builder->getInt32(1) });
                    m_Builder->CreateStore(valuesPtr, valuePtr);

                    auto len = m_Builder->CreateInBoundsGEP(m_ArrayObjectType, arrayObject, { m_Builder->getInt32(0), m_Builder->getInt32(2) });
                    m_Builder->CreateStore(m_Builder->getInt32(numCount), len); 
                }

                storedV = base;
            }
        }
    }

    auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

    llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
    m_Builder->CreateStore(vt, memberAddr);
    memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
    memberAddr = m_Builder->CreateBitCast(memberAddr, type);
    m_Builder->CreateStore(storedV, memberAddr);

    return alloc;
}

void LLVMJitVM::Push(llvm::Value *v)
{
    *m_StackTop++ = v;
}

llvm::Value *LLVMJitVM::Pop()
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

std::string LLVMJitVM::GetTypeName(llvm::Type* type)
{
    std::string str = "";
    llvm::raw_string_ostream typeStream(str);
    type->print(typeStream);
    return str;
}
