#include "LLVMJit.h"
#include "BuiltinManager.h"
#include "VM.h"

LLVMJit::OrcJit::OrcJit(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl)
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

LLVMJit::OrcJit::~OrcJit()
{
    if (auto Err = m_Es->endSession())
        m_Es->reportError(std::move(Err));
}

std::unique_ptr<LLVMJit::OrcJit> LLVMJit::OrcJit::Create()
{
    auto epc = llvm::orc::SelfExecutorProcessControl::Create();
    if (!epc)
        return nullptr;

    auto es = std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));

    llvm::orc::JITTargetMachineBuilder JTMB(es->getExecutorProcessControl().getTargetTriple());

    auto dataLayout = JTMB.getDefaultDataLayoutForTarget();
    if (!dataLayout)
        return nullptr;

    return std::make_unique<LLVMJit::OrcJit>(std::move(es), std::move(JTMB), std::move(*dataLayout));
}

const llvm::DataLayout &LLVMJit::OrcJit::GetDataLayout() const
{
    return m_DataLayout;
}

llvm::orc::JITDylib &LLVMJit::OrcJit::GetMainJITDylib()
{
    return m_MainJD;
}

llvm::Error LLVMJit::OrcJit::AddModule(llvm::orc::ThreadSafeModule tsm, llvm::orc::ResourceTrackerSP rt)
{
    if (!rt)
        rt = m_MainJD.getDefaultResourceTracker();
    return m_CompileLayer.add(rt, std::move(tsm));
}

llvm::Expected<llvm::JITEvaluatedSymbol> LLVMJit::OrcJit::LookUp(llvm::StringRef name)
{
    return m_Es->lookup({&m_MainJD}, m_Mangle(name.str()));
}

LLVMJit::LLVMJit(VM *vm)
    : m_VM(vm)
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    m_Jit = OrcJit::Create();

    InitModuleAndPassManager();
}

LLVMJit::~LLVMJit()
{
    m_VM = nullptr;
}

void LLVMJit::ResetStatus()
{
    m_BuiltinFnCache.clear();

    for (auto &g : m_GlobalVariables)
        g = nullptr;

    m_StackTop = m_ValueStack;
}

bool LLVMJit::Compile(FunctionObject *fnObj, const std::string &fnName)
{
    ResetStatus();

    enum class State
    {
        IF_CONDITION,
        IF_BODY,
        IF_ELSE,
        IF_END,
        WHILE_CONDITION,
        WHILE_BODY,
        WHILE_END
    };

    State state = State::IF_CONDITION;

    std::vector<llvm::Type *> paramTypes;
    for (Value *slot = m_VM->m_StackTop - fnObj->parameterCount; slot < m_VM->m_StackTop; ++slot)
    {
        if (IS_NUM_VALUE(*slot))
            paramTypes.push_back(m_DoubleType);
        else if (IS_STR_VALUE(*slot))
            paramTypes.push_back(m_Int8PtrType);
    }

    llvm::FunctionType* fnType = nullptr;
    if (fnObj->probableReturnTypes.size() == 0)
        fnType=llvm::FunctionType::get(m_VoidType, paramTypes, false);
    else if(fnObj->probableReturnTypes.contains(ValueType::NUM))
        fnType=llvm::FunctionType::get(m_DoubleType, paramTypes, false);
    else if (fnObj->probableReturnTypes.contains(ValueType::NIL))
        fnType = llvm::FunctionType::get(m_BoolPtrType, paramTypes, false);
    else if (fnObj->probableReturnTypes.contains(ValueType::BOOL))
        fnType = llvm::FunctionType::get(m_BoolType, paramTypes, false);
    else if (fnObj->probableReturnTypes.contains(ValueType::STR))
        fnType = llvm::FunctionType::get(m_Int8PtrType, paramTypes, false);
    else
        fnType=llvm::FunctionType::get(m_DoubleType, paramTypes, false);

    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName.c_str(), m_Module.get());
    llvm::BasicBlock *codeBlock = llvm::BasicBlock::Create(*m_Context, fnName + ".entry", fn);
    m_Builder->SetInsertPoint(codeBlock);

    auto ip = fnObj->chunk.opCodes.data();

    while ((ip - fnObj->chunk.opCodes.data()) < fnObj->chunk.opCodes.size())
    {
        int32_t instruction = *ip++;
        switch (instruction)
        {
        case OP_CONSTANT:
        {
            auto idx = *ip++;
            auto value = fnObj->chunk.constants[idx];

            if (IS_NUM_VALUE(value))
            {
                llvm::Value* alloc = llvm::ConstantFP::get(m_DoubleType, TO_NUM_VALUE(value));
                Push(alloc);
            }
            else if (IS_BOOL_VALUE(value))
            {
                llvm::Value* alloc = llvm::ConstantInt::get(m_BoolType, TO_BOOL_VALUE(value));
                Push(alloc);
            }
            else if (IS_NIL_VALUE(value))
            {
                llvm::Value* alloc = llvm::ConstantPointerNull::get(m_BoolPtrType);
                Push(alloc);
            }
            else if (IS_STR_VALUE(value))
            {
                auto str = TO_STR_VALUE(value)->value;
                llvm::Value* chars = m_Builder->CreateGlobalString(str);
                Push(chars);
            }
            else
            {
                ERROR("Unsupported value type:%d", value.type);
                return false;
            }
            /*else if (IS_FUNCTION_VALUE(value))
            {
                auto parenetFn = m_Builder->GetInsertBlock()->getParent();

                auto newFnObj = TO_FUNCTION_VALUE(value);

                auto newFnName = "function_" + newFnObj->uuid + "_" + std::to_string(paramTypeHash);

                auto newFn = Compile(newFnObj, "fn." + std::to_string(idx));

                auto block = &parenetFn->getBasicBlockList().back();
                m_Builder->SetInsertPoint(block);
            }*/

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
            {
                ERROR("Invalid binary op:%s + %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }

            break;
        }
        case OP_SUB:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFSub(left, right));
            else
            {
                ERROR("Invalid binary op:%s - %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_MUL:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFMul(left, right));
            else
            {
                ERROR("Invalid binary op:%s * %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_DIV:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFDiv(left, right));
            else
            {
                ERROR("Invalid binary op:%s / %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_LESS:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpULT(left, right));
            else
            {
                ERROR("Invalid binary op:%s < %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_GREATER:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpUGT(left, right));
            else
            {
                ERROR("Invalid binary op:%s > %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_NOT:
        {
            auto value = Pop();
            if (value->getType() == m_BoolType)
                Push(m_Builder->CreateNot(value));
            else
            {
                ERROR("Invalid binary op:not %s.", GetTypeName(value->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_MINUS:
        {
            auto value = Pop();
            if (value->getType() == m_DoubleType)
                Push(m_Builder->CreateNeg(value));
            else
            {
                ERROR("Invalid binary op:- %s.", GetTypeName(value->getType()).c_str());
                return false;
            }
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
                    Push(m_Builder->CreateICmpEQ(left, right));
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

                            auto fnType = llvm::FunctionType::get(m_Int32Type, {m_Int8PtrType, m_Int8PtrType}, false);
                            auto fn = AddBuiltinFnOrCallParamAttributes(llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "strcmp", m_Module.get()));

                            auto result = m_Builder->CreateCall(fn, {leftCharsPtr, rightCharsPtr});

                            Push(m_Builder->CreateICmpEQ(result, llvm::ConstantInt::get(m_Int32Type, 0)));
                        }
                    }
                }
            }
            else
            {
                ERROR("Invalid binary op:%s == %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_ARRAY:
        {
            auto numElements = *ip++;

            auto elements = std::vector<llvm::Value *>(numElements);

            int32_t i = numElements - 1;
            for (llvm::Value **p = m_StackTop - 1; p >= m_StackTop - numElements && i >= 0; --p, --i)
            {
                if ((*p)->getType() != m_ValuePtrType)
                {
                    llvm::Value *arg = CreateCDValue(*p);
                    elements[i] = arg;
                }
                else
                    elements[i] = *p;
            }

            auto arrayType = llvm::ArrayType::get(m_ValueType, numElements);
            llvm::Value *alloc = m_Builder->CreateAlloca(arrayType);

            llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(arrayType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});

            for (auto i = 0; i < elements.size(); ++i)
            {
                if (i == 0)
                    m_Builder->CreateMemCpy(memberAddr, llvm::MaybeAlign(8), elements[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
                else
                {
                    llvm::Value *argIMemberAddr = m_Builder->CreateInBoundsGEP(arrayType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(i)});
                    m_Builder->CreateMemCpy(argIMemberAddr, llvm::MaybeAlign(8), elements[i], llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
                }
            }

            // alloc = CreateCDValue(alloc);

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
            {
                ERROR("Invalid binary op:%s and %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_OR:
        {
            auto left = Pop();
            auto right = Pop();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                Push(m_Builder->CreateLogicalAnd(left, right));
            else
            {
                ERROR("Invalid binary op:%s or %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
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
            {
                ERROR("Invalid binary op:%s & %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
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
            {
                ERROR("Invalid binary op:%s | %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
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
            {
                ERROR("Invalid binary op:~ %s.", GetTypeName(value->getType()).c_str());
                return false;
            }
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
            {
                ERROR("Invalid binary op:%s ^ %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
                return false;
            }
            break;
        }
        case OP_GET_INDEX:
        {
            auto index = Pop();
            auto ds = Pop();

            bool isSatis = false;

            if (ds->getType()->isPointerTy())
            {
                auto vArrayType = static_cast<llvm::PointerType *>(ds->getType())->getElementType();
                if (vArrayType->isArrayTy())
                {
                    if (index->getType() == m_DoubleType)
                    {
                        isSatis = true;

                        auto iIndex = m_Builder->CreateFPToSI(index, m_Int64Type);

                        llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(vArrayType, ds, {m_Builder->getInt32(0), iIndex});

                        Push(memberAddr);
                    }
                }
            }

            if (!isSatis)
            {
                ERROR("Invalid index op: %s[%s]", GetTypeName(ds->getType()), GetTypeName(index->getType()));
                return false;
            }

            break;
        }
        case OP_SET_INDEX:
        {
            break;
        }
        case OP_JUMP_START:
        {
            auto mode = *ip++;

            auto fn = m_Builder->GetInsertBlock()->getParent();
            auto curAddress = ip - fnObj->chunk.opCodes.data();
            JumpInstrSet instrSet;

            if (mode == JumpMode::IF)
            {
                instrSet.conditionBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.condition." + std::to_string(curAddress), fn);
                instrSet.bodyBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.body." + std::to_string(curAddress), fn);
                instrSet.elseBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.else." + std::to_string(curAddress), fn);
                instrSet.endBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.end." + std::to_string(curAddress));

                m_JumpInstrSetTable.emplace_back(instrSet);

                m_Builder->CreateBr(instrSet.conditionBranch);

                m_Builder->SetInsertPoint(instrSet.conditionBranch);
                state = State::IF_CONDITION;
            }
            else
            {
                instrSet.conditionBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.condition." + std::to_string(curAddress), fn);
                instrSet.bodyBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.body." + std::to_string(curAddress), fn);
                instrSet.endBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.end." + std::to_string(curAddress));

                m_JumpInstrSetTable.emplace_back(instrSet);

                m_Builder->CreateBr(instrSet.conditionBranch);

                m_Builder->SetInsertPoint(instrSet.conditionBranch);
                state = State::WHILE_CONDITION;
            }
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = *ip++;
            auto mode = *ip++;

            auto condition = Pop();
            auto conditionValue = m_Builder->CreateICmpEQ(condition, llvm::ConstantInt::get(m_BoolType, true));
            auto &instrSet = m_JumpInstrSetTable.back();
            if (mode == JumpMode::IF)
            {
                m_Builder->CreateCondBr(conditionValue, instrSet.bodyBranch, instrSet.elseBranch);

                m_Builder->SetInsertPoint(instrSet.bodyBranch);
                state = State::IF_BODY;
            }
            else
            {
                m_Builder->CreateCondBr(conditionValue, instrSet.bodyBranch, instrSet.endBranch);

                m_Builder->SetInsertPoint(instrSet.bodyBranch);
                state = State::WHILE_BODY;
            }
            break;
        }
        case OP_JUMP:
        {
            auto address = *ip++;
            auto mode = *ip++;

            auto &instrSet = m_JumpInstrSetTable.back();
            if (instrSet.endBranch->getParent() == nullptr)
            {
                // set endBranch parent
                auto fn = m_Builder->GetInsertBlock()->getParent();
                fn->getBasicBlockList().push_back(instrSet.endBranch);
            }

            if (mode == JumpMode::IF)
            {
                if (state == State::IF_BODY)
                    m_Builder->CreateBr(instrSet.endBranch);

                m_Builder->SetInsertPoint(instrSet.elseBranch);
                state = State::IF_ELSE;
            }
            else
            {
                m_JumpInstrSetTable.pop_back();

                m_Builder->CreateBr(instrSet.conditionBranch);
                m_Builder->SetInsertPoint(instrSet.endBranch);
                state = State::WHILE_END;
            }

            break;
        }
        case OP_JUMP_END:
        {
            auto &br = m_JumpInstrSetTable.back();
            // set endBranch parent
            if (br.endBranch->getParent() == nullptr)
            {
                auto fn = m_Builder->GetInsertBlock()->getParent();
                fn->getBasicBlockList().push_back(br.endBranch);
            }

            if (state == State::IF_ELSE)
                m_Builder->CreateBr(br.endBranch); // create jump br for else branch;

            m_Builder->SetInsertPoint(br.endBranch);
            state = State::IF_END;

            {
                auto offset = m_JumpInstrSetTable.size() - 1;
                if (offset > 0)
                    m_Builder->CreateBr(m_JumpInstrSetTable[offset - 1].endBranch);
            }

            m_JumpInstrSetTable.pop_back();

            break;
        }
        case OP_RETURN:
        {
            auto returnCount = *ip++;
            if (returnCount == 1)
            {
                auto value = Pop();
                m_Builder->CreateRet(value);
            }
            else
                m_Builder->CreateRetVoid();
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = *ip++;
            auto v = Pop();

            auto &ref = m_GlobalVariables[index];

            if (ref == nullptr || ref->getType() != v->getType())
            {
                auto alloc = m_Builder->CreateAlloca(v->getType(), nullptr, "globalVar_" + std::to_string(index));
                m_Builder->CreateStore(v, alloc);

                ref = alloc;
            }
            else
                m_Builder->CreateStore(v, ref);

            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *ip++;

            auto stored = m_GlobalVariables[index];

            if (stored->getType()->isPointerTy())
            {
                auto ptrType = static_cast<llvm::PointerType *>(stored->getType());
                auto v = m_Builder->CreateLoad(ptrType->getElementType(), stored);
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
            auto argCount = (uint8_t)*ip++;
            auto fn = static_cast<llvm::Function *>(*(m_StackTop - argCount - 1));

            auto ptrType = fn->getType();
            auto elemType = ptrType->getElementType();

            // builtin function type
            if (elemType == m_BuiltinFunctionType)
            {
                std::vector<llvm::Value *> argsV;
                for (auto slot = m_StackTop - argCount; slot != m_StackTop; ++slot)
                {
                    if ((*slot)->getType() != m_ValuePtrType)
                        argsV.emplace_back(CreateCDValue(*slot));
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
            }

            break;
        }
        case OP_GET_BUILTIN:
        {
            auto idx = *ip++;
            auto value = fnObj->chunk.constants[idx];
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
            auto offset = *ip++;
            m_StackTop += offset;
            break;
        }
        default:
            break;
        }
    }

    fn->print(llvm::errs());

    llvm::verifyFunction(*fn);
    m_FPM->run(*fn);

    m_Jit->AddModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context)));
    InitModuleAndPassManager();

    return true;
}

void LLVMJit::InitModuleAndPassManager()
{
    m_Context = std::make_unique<llvm::LLVMContext>();

    m_Module = std::make_unique<llvm::Module>("ComputeDuck LLVM JIT", *m_Context);
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

        m_Int8PtrPtrType = llvm::PointerType::get(m_Int8PtrType, 0);

        m_UnionType = llvm::StructType::create(*m_Context, {m_DoubleType}, "union.anon");

        m_ValueType = llvm::StructType::create(*m_Context, {m_Int8Type, m_UnionType}, "struct.Value");
        m_ValuePtrType = llvm::PointerType::get(m_ValueType, 0);

        m_ObjectType = llvm::StructType::create(*m_Context, "struct.Object");
        m_ObjectPtrType = llvm::PointerType::get(m_ObjectType, 0);
        m_ObjectType->setBody({m_BoolType, m_ObjectPtrType});
        m_ObjectPtrPtrType = llvm::PointerType::get(m_ObjectPtrType, 0);

        m_StrObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_Int8PtrType, m_Int32Type}, "struct.StrObject");
        m_StrObjectPtrType = llvm::PointerType::get(m_StrObjectType, 0);

        m_ArrayObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_ValuePtrType, m_Int32Type}, "struct.ArrayObject");
        m_ArrayObjectPtrType = llvm::PointerType::get(m_ArrayObjectType, 0);

        m_BuiltinFunctionType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType, m_Int8Type, m_ValuePtrType}, false);
    }
}

llvm::Value *LLVMJit::CreateCDValue(llvm::Value *v)
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
    /*else if (v->getType() == m_ObjectPtrType)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));
        type = m_ObjectPtrPtrType;
        storedV = v;
    }*/
    else if (v->getType()->isPointerTy())
    {
        auto vPtrType = static_cast<llvm::PointerType *>(v->getType());
        if (vPtrType->getElementType()->isArrayTy())
        {
            auto vArrayType = static_cast<llvm::ArrayType *>(vPtrType->getElementType());
            if (vArrayType->getElementType() == m_Int8Type)
            {
                vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::STR));
                type = m_ObjectPtrPtrType;

                // convert chars[] to i8*
                auto charsPtr = m_Builder->CreateInBoundsGEP(vArrayType, v, {m_Builder->getInt64(0), m_Builder->getInt64(0)});

                // create str object
                auto strObject = m_Builder->CreateAlloca(m_StrObjectType, nullptr);
                auto base = m_Builder->CreateBitCast(strObject, m_ObjectPtrType);

                {
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

                vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::ARRAY));
                type = m_ObjectPtrPtrType;

                // convert Value[] to Value*
                auto valuesPtr = m_Builder->CreateInBoundsGEP(vArrayType, v, {m_Builder->getInt64(0), m_Builder->getInt64(0)});

                // create array object
                auto arrayObject = m_Builder->CreateAlloca(m_ArrayObjectType, nullptr);
                auto base = m_Builder->CreateBitCast(arrayObject, m_ObjectPtrType);

                {
                    auto markedVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                    m_Builder->CreateStore(m_Builder->getInt1(0), markedVar);

                    auto nextVar = m_Builder->CreateInBoundsGEP(m_ObjectType, base, {m_Builder->getInt32(0), m_Builder->getInt32(2)});
                    m_Builder->CreateStore(llvm::ConstantPointerNull::get(m_ObjectPtrType), nextVar);

                    auto valuePtr = m_Builder->CreateInBoundsGEP(m_ArrayObjectType, arrayObject, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                    m_Builder->CreateStore(valuesPtr, valuePtr);

                    auto len = m_Builder->CreateInBoundsGEP(m_ArrayObjectType, arrayObject, {m_Builder->getInt32(0), m_Builder->getInt32(2)});
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

void LLVMJit::Push(llvm::Value *v)
{
    *m_StackTop++ = v;
}

llvm::Value *LLVMJit::Pop()
{
    return *(--m_StackTop);
}

std::string LLVMJit::GetTypeName(llvm::Type *type)
{
    std::string str = "";
    llvm::raw_string_ostream typeStream(str);
    type->print(typeStream);
    return str;
}