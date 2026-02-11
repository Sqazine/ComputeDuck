#ifdef COMPUTEDUCK_BUILD_WITH_LLVM

#include "Jit.h"
#include "Allocator.h"
#include "BuiltinManager.h"

Jit::OrcExecutor::OrcExecutor(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl)
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

Jit::OrcExecutor::~OrcExecutor()
{
    if (auto Err = m_Es->endSession())
        m_Es->reportError(std::move(Err));
}

std::unique_ptr<Jit::OrcExecutor> Jit::OrcExecutor::Create()
{
    auto epc = llvm::orc::SelfExecutorProcessControl::Create();
    if (!epc)
        return nullptr;

    auto es = std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));

    llvm::orc::JITTargetMachineBuilder JTMB(es->getExecutorProcessControl().getTargetTriple());

    auto dataLayout = JTMB.getDefaultDataLayoutForTarget();
    if (!dataLayout)
        return nullptr;

    return std::make_unique<Jit::OrcExecutor>(std::move(es), std::move(JTMB), std::move(*dataLayout));
}

const llvm::DataLayout &Jit::OrcExecutor::GetDataLayout() const
{
    return m_DataLayout;
}

llvm::orc::JITDylib &Jit::OrcExecutor::GetMainJITDylib()
{
    return m_MainJD;
}

llvm::Error Jit::OrcExecutor::AddModule(llvm::orc::ThreadSafeModule tsm, llvm::orc::ResourceTrackerSP rt)
{
    if (!rt)
        rt = m_MainJD.getDefaultResourceTracker();
    return m_CompileLayer.add(rt, std::move(tsm));
}

llvm::Expected<llvm::JITEvaluatedSymbol> Jit::OrcExecutor::LookUp(llvm::StringRef name)
{
    return m_Es->lookup({&m_MainJD}, m_Mangle(name.str()));
}

Jit::Jit()
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    m_Executor = OrcExecutor::Create();

    InitModuleAndPassManager();

    CreateSetGlobalVariablesFunction();
    CreateSetStackFunction();
}

Jit::~Jit()
{
}

void Jit::CreateSetGlobalVariablesFunction()
{
    {
        m_Module->setModuleIdentifier(SET_GLOBAL_VARIABLE_FN_STR);
        m_Module->setSourceFileName(SET_GLOBAL_VARIABLE_FN_STR);
    }

    m_Module->getOrInsertGlobal(GLOBAL_VARIABLE_STR, m_ValuePtrType);
    auto globalVariable = m_Module->getNamedGlobal(GLOBAL_VARIABLE_STR);
    globalVariable->setInitializer(llvm::ConstantPointerNull::get(m_ValuePtrType));
    globalVariable->setAlignment(llvm::MaybeAlign(8));
    globalVariable->setDSOLocal(true);

    llvm::FunctionType *fnType = llvm::FunctionType::get(m_VoidType, {m_ValuePtrType}, false);
    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, SET_GLOBAL_VARIABLE_FN_STR, m_Module.get());
    llvm::BasicBlock *codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
    m_Builder->SetInsertPoint(codeBlock);
    auto tmpAlloc = m_Builder->CreateAlloca(m_ValuePtrType);
    m_Builder->CreateStore(fn->getArg(0), tmpAlloc);
    auto load = m_Builder->CreateLoad(tmpAlloc->getAllocatedType(), tmpAlloc);
    m_Builder->CreateStore(load, globalVariable);
    m_Builder->CreateRetVoid();
#ifndef NDEBUG
    m_Module->print(llvm::errs(), nullptr);
#endif

    m_Executor->AddModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context)));
    InitModuleAndPassManager();
}

void Jit::CreateGlobalVariablesDecl()
{
    m_Module->getOrInsertGlobal(GLOBAL_VARIABLE_STR, m_ValuePtrType);
    auto globalVariable = m_Module->getNamedGlobal(GLOBAL_VARIABLE_STR);
    globalVariable->setLinkage(llvm::GlobalVariable::ExternalLinkage);
    globalVariable->setAlignment(llvm::MaybeAlign(8));
}

void Jit::CreateSetStackFunction()
{
    {
        m_Module->setModuleIdentifier(SET_STACK_FN_STR);
        m_Module->setSourceFileName(SET_STACK_FN_STR);
    }

    m_Module->getOrInsertGlobal(STACK_STR, m_ValuePtrType);
    auto stackVariable = m_Module->getNamedGlobal(STACK_STR);
    stackVariable->setInitializer(llvm::ConstantPointerNull::get(m_ValuePtrType));
    stackVariable->setAlignment(llvm::MaybeAlign(8));
    stackVariable->setDSOLocal(true);

    llvm::FunctionType *fnType = llvm::FunctionType::get(m_VoidType, {m_ValuePtrType}, false);
    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, SET_STACK_FN_STR, m_Module.get());
    llvm::BasicBlock *codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
    m_Builder->SetInsertPoint(codeBlock);
    auto tmpAlloc = m_Builder->CreateAlloca(m_ValuePtrType);
    m_Builder->CreateStore(fn->getArg(0), tmpAlloc);
    auto load = m_Builder->CreateLoad(tmpAlloc->getAllocatedType(), tmpAlloc);
    m_Builder->CreateStore(load, stackVariable);
    m_Builder->CreateRetVoid();
#ifndef NDEBUG
    m_Module->print(llvm::errs(), nullptr);
#endif

    m_Executor->AddModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context)));
    InitModuleAndPassManager();
}

void Jit::CreateStackDecl()
{
    m_Module->getOrInsertGlobal(STACK_STR, m_ValuePtrType);
    auto stackTop = m_Module->getNamedGlobal(STACK_STR);
    stackTop->setLinkage(llvm::GlobalVariable::ExternalLinkage);
    stackTop->setAlignment(llvm::MaybeAlign(8));
}

void Jit::ResetStatus()
{
    m_BuiltinFnCache.clear();

    m_StackTop = m_ValueStack;
}

JitFnDecl Jit::Compile(const CallFrame &frame, const std::string &fnName)
{
    {
        m_Module->setSourceFileName(fnName);
        m_Module->setModuleIdentifier(fnName);
    }

    std::map<std::string, llvm::AllocaInst *> localVariables;
    BranchState branchState = BranchState::IF_CONDITION;
    JitFnDecl jitFnDecl;

    std::vector<llvm::Type *> paramTypes;
    for (Value *slot = frame.slot; slot < frame.slot + frame.fn->parameterCount; ++slot)
    {
        auto typePair = GetTypeFromValue(*slot);
        paramTypes.emplace_back(typePair.first);
        jitFnDecl.paramTypes.emplace_back(typePair.second);
    }

    llvm::FunctionType *fnType = nullptr;
    if (frame.fn->probableReturnTypeSet->IsNone())
        fnType = llvm::FunctionType::get(m_VoidType, paramTypes, false);
    else if (!frame.fn->probableReturnTypeSet->IsMultiplyType())
    {
        auto type = frame.fn->probableReturnTypeSet->GetOnly();
        auto llvmType = GetLlvmTypeFromValueType(type);
        fnType = llvm::FunctionType::get(llvmType, paramTypes, false);
    }
    else
        ERROR(JitCompileState::FAIL, "Multiply return type of function,skip jit compile");

    llvm::Function *currentCompileFunction = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName.c_str(), m_Module.get());
    currentCompileFunction->setDSOLocal(true);
    for (auto i = 0; i < paramTypes.size(); ++i)
        currentCompileFunction->addParamAttr(i, llvm::Attribute::NoUndef);

    llvm::BasicBlock *codeBlock = llvm::BasicBlock::Create(*m_Context, "", currentCompileFunction);
    m_Builder->SetInsertPoint(codeBlock);

    auto ip = frame.fn->chunk.opCodes.data();
    while ((ip - frame.fn->chunk.opCodes.data()) < frame.fn->chunk.opCodes.size())
    {
        int32_t instruction = *ip++;
        switch (instruction)
        {
        case OP_CONSTANT:
        {
            auto idx = *ip++;
            auto value = frame.fn->chunk.constants[idx];

            if (IS_FUNCTION_VALUE(value))
                ERROR(JitCompileState::FAIL, "Not support jit compile for:%s", fnName.c_str())
            else
            {
                auto llvmValue = CreateLlvmValue(value);
                if (!llvmValue)
                    ERROR(JitCompileState::FAIL, "Unsupported value type:%d", value.type);

                Push(llvmValue);
            }
            break;
        }
        case OP_ADD:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();

            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFAdd(left, right));
            else if (left->getType() == m_StrObjectPtrType && right->getType() == m_StrObjectPtrType)
                Push(m_Builder->CreateCall(m_Module->getFunction(STR(StrAdd)), {left, right}));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto result = m_Builder->CreateAlloca(m_ValueType);

                m_Builder->CreateCall(m_Module->getFunction(STR(ValueAdd)), {left, right, result});

                Push(result);
            }

            break;
        }
        case OP_SUB:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFSub(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueSub)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_MUL:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFMul(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueMul)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_DIV:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFDiv(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueDiv)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_LESS:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpULT(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueLess)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_GREATER:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpUGT(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueGreater)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_NOT:
        {
            auto value = Pop().GetLlvmValue();
            if (value->getType() == m_BoolType)
                Push(m_Builder->CreateNot(value));
            else
            {
                if (value->getType() != m_ValuePtrType)
                    value = CreateLlvmValue(value);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueLogicNot)), {value});
                Push(call);
            }
            break;
        }
        case OP_MINUS:
        {
            auto value = Pop().GetLlvmValue();
            if (value->getType() == m_DoubleType)
                Push(m_Builder->CreateNeg(value));
            else
            {
                if (value->getType() != m_ValuePtrType)
                    value = CreateLlvmValue(value);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueMinus)), {value});
                Push(call);
            }
            break;
        }
        case OP_EQUAL:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();

            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpUEQ(left, right));
            else if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                Push(m_Builder->CreateICmpEQ(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueGreater)), {left, right});
                Push(call);
            }

            break;
        }
        case OP_ARRAY:
        {
            auto numElements = *ip++;

            auto elements = std::vector<llvm::Value *>(numElements);

            int32_t i = numElements - 1;
            for (auto p = m_StackTop - 1; p >= m_StackTop - numElements && i >= 0; --p, --i)
            {
                auto v = p->GetLlvmValue();
                if (v->getType() != m_ValuePtrType)
                {
                    llvm::Value *arg = CreateLlvmValue(v);
                    elements[i] = arg;
                }
                else
                    elements[i] = v;
            }

            m_StackTop -= numElements;

            llvm::Value *alloc = m_Builder->CreateAlloca(m_ValuePtrType);

            auto mallocation = m_Builder->CreateCall(m_Module->getFunction(STR(malloc)), {llvm::ConstantInt::get(m_Int64Type, numElements * sizeof(Value))});
            auto arrayObjectBitCast = m_Builder->CreateBitCast(mallocation, m_ValuePtrType);
            m_Builder->CreateStore(arrayObjectBitCast, alloc);

            auto load = m_Builder->CreateLoad(m_ValuePtrType, alloc);

            llvm::Value *memberAddr = nullptr;
            for (auto i = 0; i < elements.size(); ++i)
            {
                memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, load, {m_Builder->getInt64(i)});
                auto addrBitCast = m_Builder->CreateBitCast(memberAddr, m_Int8PtrType);
                auto eleBitCast = m_Builder->CreateBitCast(elements[i], m_Int8PtrType);
                m_Builder->CreateMemCpy(addrBitCast, llvm::MaybeAlign(sizeof(Value)), eleBitCast, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
            }

            auto arrayObject = m_Builder->CreateCall(m_Module->getFunction(STR(CreateArrayObject)), {load, llvm::ConstantInt::get(m_Int32Type, numElements)});
            arrayObjectBitCast = m_Builder->CreateBitCast(arrayObject, m_ObjectPtrType);

            Push(arrayObjectBitCast);
            break;
        }
        case OP_AND:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                Push(m_Builder->CreateLogicalAnd(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueLogicAnd)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_OR:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                Push(m_Builder->CreateLogicalAnd(left, right));
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueLogicOr)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_BIT_AND:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
            {
                auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
                auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
                auto result = m_Builder->CreateAnd(lCast, rCast);
                Push(result);
            }
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueBitAnd)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_BIT_OR:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
            {
                auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
                auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
                auto result = m_Builder->CreateOr(lCast, rCast);
                Push(result);
            }
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueBitOr)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_BIT_NOT:
        {
            auto value = Pop().GetLlvmValue();
            if (value->getType() == m_DoubleType)
            {
                value = m_Builder->CreateFPToSI(value, m_Int64Type);
                Push(m_Builder->CreateXor(value, -1));
            }
            else
            {
                if (value->getType() != m_ValuePtrType)
                    value = CreateLlvmValue(value);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueBitNot)), {value});
                Push(call);
            }
            break;
        }
        case OP_BIT_XOR:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_BoolType)
            {
                auto lCast = m_Builder->CreateFPToSI(left, m_Int64Type);
                auto rCast = m_Builder->CreateFPToSI(right, m_Int64Type);
                auto result = m_Builder->CreateXor(lCast, rCast);
                Push(result);
            }
            else
            {
                if (left->getType() != m_ValuePtrType)
                    left = CreateLlvmValue(left);
                if (right->getType() != m_ValuePtrType)
                    right = CreateLlvmValue(right);

                auto call = m_Builder->CreateCall(m_Module->getFunction(STR(ValueBitXor)), {left, right});
                Push(call);
            }
            break;
        }
        case OP_GET_INDEX:
        {
            auto index = Pop().GetLlvmValue();
            auto ds = Pop().GetLlvmValue();

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
                else if (ds->getType() == m_ArrayObjectPtrType && index->getType() == m_DoubleType)
                {
                    ds = CreateLlvmValue(ds);
                    index = CreateLlvmValue(index);

                    auto result = m_Builder->CreateAlloca(m_ValueType, nullptr);

                    m_Builder->CreateCall(m_Module->getFunction(STR(GetArrayObjectElement)), {ds, index, result});

                    Push(result);
                    isSatis = true;
                }
            }

            if (!isSatis)
                ERROR(JitCompileState::FAIL, "Invalid index op: %s[%s]", GetTypeName(ds->getType()).c_str(), GetTypeName(index->getType()).c_str());
            break;
        }
        case OP_SET_INDEX:
        {
            auto index = Pop().GetLlvmValue();
            auto ds = Pop().GetLlvmValue();
            auto v = Pop().GetLlvmValue();

            bool isSatis = false;

            if (ds->getType()->isPointerTy())
            {
                auto elementType = static_cast<llvm::PointerType *>(ds->getType())->getElementType();
                if (elementType->isArrayTy())
                {
                    if (index->getType() == m_DoubleType)
                    {
                        isSatis = true;
                        auto iIndex = m_Builder->CreateFPToSI(index, m_Int64Type);
                        llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(elementType, ds, {m_Builder->getInt32(0), iIndex});
                        m_Builder->CreateStore(v, memberAddr);
                    }
                }
                else if (elementType == m_ValueType)
                {
                    auto objMemberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, ds, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                    auto arrayObjMemberAddr = m_Builder->CreateBitCast(objMemberAddr, m_ObjectPtrPtrType);
                    arrayObjMemberAddr = m_Builder->CreateLoad(m_ObjectPtrType, arrayObjMemberAddr);
                    arrayObjMemberAddr = m_Builder->CreateBitCast(arrayObjMemberAddr, m_ArrayObjectPtrType);

                    auto array = m_Builder->CreateInBoundsGEP(m_ArrayObjectType, arrayObjMemberAddr, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                    auto arrayElements = m_Builder->CreateLoad(m_ValuePtrType, array);

                    if (index->getType() == m_DoubleType)
                    {
                        isSatis = true;
                        auto iIndex = m_Builder->CreateFPToSI(index, m_Int64Type);

                        llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, arrayElements, iIndex);

                        if (v->getType() != m_ValueType || v->getType() != m_ValuePtrType)
                            v = CreateLlvmValue(v);

                        auto castV = m_Builder->CreateBitCast(v, m_Int8PtrType);
                        auto castMemberAddr = m_Builder->CreateBitCast(memberAddr, m_Int8PtrType);

                        m_Builder->CreateMemCpy(castMemberAddr, llvm::MaybeAlign(sizeof(Value)), castV, llvm::MaybeAlign(sizeof(Value)), m_Builder->getInt64(sizeof(Value)));
                    }
                }
            }

            if (!isSatis)
                ERROR(JitCompileState::FAIL, "Invalid index op: %s[%s]", GetTypeName(ds->getType()).c_str(), GetTypeName(index->getType()).c_str());
            break;
        }
        case OP_JUMP_START:
        {
            auto mode = *ip++;

            auto fn = m_Builder->GetInsertBlock()->getParent();
            auto curAddress = ip - frame.fn->chunk.opCodes.data();
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
                branchState = BranchState::IF_CONDITION;
            }
            else
            {
                instrSet.conditionBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.condition." + std::to_string(curAddress), fn);
                instrSet.bodyBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.body." + std::to_string(curAddress), fn);
                instrSet.endBranch = llvm::BasicBlock::Create(*m_Context, "jumpInstr.end." + std::to_string(curAddress));

                m_JumpInstrSetTable.emplace_back(instrSet);

                m_Builder->CreateBr(instrSet.conditionBranch);

                m_Builder->SetInsertPoint(instrSet.conditionBranch);
                branchState = BranchState::WHILE_CONDITION;
            }
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = *ip++;
            auto mode = *ip++;

            auto condition = Pop().GetLlvmValue();
            auto conditionValue = m_Builder->CreateICmpEQ(condition, llvm::ConstantInt::get(m_BoolType, true));
            auto &instrSet = m_JumpInstrSetTable.back();
            if (mode == JumpMode::IF)
            {
                m_Builder->CreateCondBr(conditionValue, instrSet.bodyBranch, instrSet.elseBranch);

                m_Builder->SetInsertPoint(instrSet.bodyBranch);
                branchState = BranchState::IF_BODY;
            }
            else
            {
                m_Builder->CreateCondBr(conditionValue, instrSet.bodyBranch, instrSet.endBranch);

                m_Builder->SetInsertPoint(instrSet.bodyBranch);
                branchState = BranchState::WHILE_BODY;
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
                if (branchState == BranchState::IF_BODY)
                    m_Builder->CreateBr(instrSet.endBranch);

                m_Builder->SetInsertPoint(instrSet.elseBranch);
                branchState = BranchState::IF_ELSE;
            }
            else
            {
                m_JumpInstrSetTable.pop_back();

                m_Builder->CreateBr(instrSet.conditionBranch);
                m_Builder->SetInsertPoint(instrSet.endBranch);
                branchState = BranchState::WHILE_END;
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

            if (branchState == BranchState::IF_ELSE)
                m_Builder->CreateBr(br.endBranch); // create jump br for else branch;

            m_Builder->SetInsertPoint(br.endBranch);
            branchState = BranchState::IF_END;

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
                auto value = Pop().GetLlvmValue();
                if (value->getType() == m_ValuePtrType)
                {
                    if (currentCompileFunction->getReturnType() == m_DoubleType)
                    {
                        value = m_Builder->CreateInBoundsGEP(m_ValueType, value, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                        value = m_Builder->CreateBitCast(value, m_DoublePtrType);
                        value = m_Builder->CreateLoad(m_DoubleType, value);
                    }
                    else if (currentCompileFunction->getReturnType() == m_StrObjectPtrType)
                    {
                        value = m_Builder->CreateInBoundsGEP(m_ValueType, value, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                        value = m_Builder->CreateBitCast(value, m_ObjectPtrPtrType);
                        value = m_Builder->CreateLoad(m_ObjectPtrType, value);
                        value = m_Builder->CreateBitCast(value, m_StrObjectPtrType);
                    }
                    else if (currentCompileFunction->getReturnType() == m_ArrayObjectPtrType)
                    {
                        value = m_Builder->CreateInBoundsGEP(m_ValueType, value, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                        value = m_Builder->CreateBitCast(value, m_ObjectPtrPtrType);
                        value = m_Builder->CreateLoad(m_ObjectPtrType, value);
                        value = m_Builder->CreateBitCast(value, m_ArrayObjectPtrType);
                    }
                }

                m_Builder->CreateRet(value);
            }
            else
                m_Builder->CreateRetVoid();
            break;
        }
        case OP_DEF_GLOBAL:
        {
            auto index = *ip++;
            auto v = Pop().GetLlvmValue();

            auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(GLOBAL_VARIABLE_STR));
            auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));

            if (v->getType() != m_ValueType)
                v = CreateLlvmValue(v);

            v = m_Builder->CreateBitCast(v, m_Int8PtrType);
            globalVar = m_Builder->CreateBitCast(v, m_Int8PtrType);

            m_Builder->CreateMemCpy(globalVar, llvm::MaybeAlign(8), v, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = *ip++;
            auto v = Pop().GetLlvmValue();

            auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(GLOBAL_VARIABLE_STR));
            auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));

            if (v->getType() != m_ValueType)
                v = CreateLlvmValue(v);

            globalVar = m_Builder->CreateCall(m_Module->getFunction(STR(GetEndOfRefValuePtr)), {globalVar});

            v = m_Builder->CreateBitCast(v, m_Int8PtrType);
            globalVar = m_Builder->CreateBitCast(v, m_Int8PtrType);

            m_Builder->CreateMemCpy(globalVar, llvm::MaybeAlign(8), v, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *ip++;

            auto vmGlobal = *GET_GLOBAL_VARIABLE_REF(index);
            if (IS_FUNCTION_VALUE(vmGlobal))
                Push(vmGlobal);
            else
            {
                auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(GLOBAL_VARIABLE_STR));
                auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));
                Push(globalVar);
            }
            break;
        }
        case OP_DEF_LOCAL:
        {
            auto index = *ip++;
            auto value = Pop().GetLlvmValue();

            auto name = GenerateLocalVarName(index);

            auto alloc = m_Builder->CreateAlloca(value->getType());
            m_Builder->CreateStore(value, alloc);

            localVariables[name] = alloc;
            break;
        }
        case OP_SET_LOCAL:
        {
            auto index = *ip++;
            auto value = Pop().GetLlvmValue();

            auto name = GenerateLocalVarName(index);

            auto iter = localVariables.find(name);
            if (iter == localVariables.end())
                ERROR(JitCompileState::FAIL, "Cannot find local variable:%s,maybe it is a upvalue.", name.c_str());
            if (iter->second->getAllocatedType() == m_RefObjectPtrType)
            {
                if (value->getType() != m_ValuePtrType)
                    value = CreateLlvmValue(value);

                llvm::Value *refValue = m_Builder->CreateLoad(m_RefObjectPtrType, iter->second);
                refValue = CreateLlvmValue(refValue);

                refValue = m_Builder->CreateCall(m_Module->getFunction(STR(GetEndOfRefValuePtr)), {refValue});

                value = m_Builder->CreateBitCast(value, m_Int8PtrType);
                refValue = m_Builder->CreateBitCast(refValue, m_Int8PtrType);

                m_Builder->CreateMemCpy(refValue, llvm::MaybeAlign(8), value, llvm::MaybeAlign(8), llvm::ConstantInt::get(m_Int64Type, sizeof(Value)));
            }
            else
                m_Builder->CreateStore(value, iter->second);

            break;
        }
        case OP_GET_LOCAL:
        {
            auto index = *ip++;

            auto name = GenerateLocalVarName(index);

            auto iter = localVariables.find(name);
            if (iter == localVariables.end()) // create from function argumenet
            {
                auto parentFn = m_Builder->GetInsertBlock()->getParent();
                if (index < parentFn->arg_size()) // load from function arguments
                {
                    auto arg = parentFn->getArg(index);

                    llvm::AllocaInst *alloc = m_Builder->CreateAlloca(arg->getType());

                    m_Builder->CreateStore(arg, alloc);

                    localVariables[name] = alloc;

                    auto load = m_Builder->CreateLoad(alloc->getAllocatedType(), alloc);

                    Push(load);
                }
                else // load from interpreter vm
                {
                    auto slot = m_Builder->CreateCall(m_Module->getFunction(STR(GetLocalVariableSlot)), {
                                                                                                            llvm::ConstantInt::get(m_Int16Type, index),
                                                                                                        });

                    auto alloc = m_Builder->CreateAlloca(slot->getType());
                    m_Builder->CreateStore(slot, alloc);

                    localVariables[name] = alloc;
                    Push(slot);
                }
            }
            else
            {
                auto load = m_Builder->CreateLoad(iter->second->getAllocatedType(), iter->second);
                Push(load);
            }
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = (uint8_t)*ip++;

            auto fnSlot = (m_StackTop - argCount - 1);

            if (fnSlot->IsLlvmValue())
            {
                auto fn = static_cast<llvm::Function *>(fnSlot->GetLlvmValue());

                auto ptrType = fn->getType();
                auto elemType = ptrType->getElementType();

                // builtin function type
                if (elemType == m_BuiltinFunctionType)
                {
                    std::vector<llvm::Value *> argsV;
                    for (auto slot = m_StackTop - argCount; slot != m_StackTop; ++slot)
                    {
                        auto v = slot->GetLlvmValue();
                        if (v->getType() != m_ValuePtrType)
                            argsV.emplace_back(CreateLlvmValue(v));
                        else
                            argsV.emplace_back(v);
                    }

                    m_StackTop = m_StackTop - (argCount + 1);

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

                    auto callInst = m_Builder->CreateCall(fn, {arg0MemberAddr, arg1, result});

                    Push(result);
                }
            }
            else
            {
                auto vmFn = TO_FUNCTION_VALUE(fnSlot->GetVmValue());

                size_t paramTypeHash = 0;
                for (auto slot = m_StackTop - vmFn->parameterCount; slot < m_StackTop; ++slot)
                {
                    auto vType = GetValueTypeFromLlvmType(slot->GetLlvmValue()->getType());
                    paramTypeHash ^= std::hash<uint8_t>()(vType);
                }

                auto fnName = GenerateFunctionName(vmFn->uuid, vmFn->probableReturnTypeSet->Hash(), paramTypeHash);

                auto iter = vmFn->jitCache.find(paramTypeHash);
                if (iter != vmFn->jitCache.end() && !vmFn->probableReturnTypeSet->IsMultiplyType())
                {
                    std::vector<llvm::Value *> args;
                    for (auto slot = m_StackTop - argCount; slot < m_StackTop; ++slot)
                        args.emplace_back(slot->GetLlvmValue());

                    m_StackTop = m_StackTop - argCount - 1;

                    auto internalFunction = m_Module->getFunction(fnName);
                    if (internalFunction == nullptr)
                    {
                        JitFnDecl jitDecl = iter->second;

                        llvm::Type *fnReturnType = GetLlvmTypeFromValueType(jitDecl.returnType);
                        std::vector<llvm::Type *> fnParameterTypes{};

                        for (auto &paramType : jitDecl.paramTypes)
                        {
                            auto llvmType = GetLlvmTypeFromValueType(jitDecl.returnType);
                            fnParameterTypes.emplace_back(llvmType);
                        }

                        llvm::FunctionType *newFnDeclType = llvm::FunctionType::get(fnReturnType, fnParameterTypes, false);
                        m_Module->getOrInsertFunction(fnName, newFnDeclType);
                    }
                    llvm::Value *ret = m_Builder->CreateCall(m_Module->getFunction(fnName), args);
                    Push(ret);
                }
                else
                    ERROR(JitCompileState::DEPEND, "%s depends on another function %s", currentCompileFunction->getName().str().c_str(), vmFn->uuid.c_str());
            }
            break;
        }
        case OP_GET_BUILTIN:
        {
            auto idx = *ip++;
            auto value = frame.fn->chunk.constants[idx];
            auto name = TO_STR_VALUE(value)->value;
            std::string nameStr = name;
            auto iter = m_BuiltinFnCache.find(name);

            llvm::Function *fn = nullptr;
            if (iter == m_BuiltinFnCache.end())
            {
                fn = llvm::Function::Create(m_BuiltinFunctionType, llvm::Function::ExternalLinkage, BUILTIN_FN_PREFIX_STR + nameStr, m_Module.get());
                m_BuiltinFnCache[name] = fn;
            }
            else
                fn = iter->second;

            Push(fn);
            break;
        }
        case OP_STRUCT:
        {
            auto memberCount = *ip++;

            auto tableInstancePtr = m_Builder->CreateCall(m_Module->getFunction(STR(CreateTable)));

            for (size_t i = 0; i < memberCount; ++i)
            {
                auto name = Pop().GetLlvmValue();
                if (name->getType() == m_Int8PtrType)
                    name = m_Builder->CreateCall(m_Module->getFunction(STR(CreateStrObject)), {name});

                auto value = Pop().GetLlvmValue();
                if (value->getType() != m_ValueType)
                    value = CreateLlvmValue(value);

                m_Builder->CreateCall(m_Module->getFunction(STR(TableSet)), {tableInstancePtr, name, value});
            }

            auto structInstancePtr = m_Builder->CreateCall(m_Module->getFunction(STR(CreateStructObject)), {tableInstancePtr});
            Push(structInstancePtr);
            break;
        }
        case OP_GET_STRUCT:
        {
            auto memberName = Pop().GetLlvmValue();
            auto instance = Pop().GetLlvmValue();

            if (memberName->getType() == m_Int8PtrType)
                memberName = m_Builder->CreateCall(m_Module->getFunction(STR(CreateStrObject)), {memberName});

            if (memberName->getType() != m_StrObjectPtrType)
                ERROR(JitCompileState::FAIL, "Invalid member name of struct instance");

            if (instance->getType() == m_ValuePtrType)
            {
                instance = m_Builder->CreateInBoundsGEP(m_ValueType, instance, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                instance = m_Builder->CreateBitCast(instance, m_ObjectPtrPtrType);
                instance = m_Builder->CreateLoad(m_ObjectPtrType, instance);
                instance = m_Builder->CreateBitCast(instance, m_StructObjectPtrType);
            }

            auto tablePtr = m_Builder->CreateInBoundsGEP(m_StructObjectType, instance, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
            tablePtr = m_Builder->CreateLoad(m_TablePtrType, tablePtr);

            auto resultValuePtr = m_Builder->CreateAlloca(m_ValueType);

            m_Builder->CreateCall(m_Module->getFunction(STR(TableGet)), {tablePtr, memberName, resultValuePtr});

            Push(resultValuePtr);
            break;
        }
        case OP_SET_STRUCT:
        {
            auto memberName = Pop().GetLlvmValue();
            auto instance = Pop().GetLlvmValue();

            auto value = Pop().GetLlvmValue();

            if (memberName->getType() == m_Int8PtrType)
                memberName = m_Builder->CreateCall(m_Module->getFunction(STR(CreateStrObject)), {memberName});

            if (memberName->getType() != m_StrObjectPtrType)
                ERROR(JitCompileState::FAIL, "Invalid member name of struct instance");

            if (value->getType() != m_ValuePtrType)
                value = CreateLlvmValue(value);

            if (instance->getType() == m_ValuePtrType)
            {
                instance = m_Builder->CreateInBoundsGEP(m_ValueType, instance, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
                instance = m_Builder->CreateBitCast(instance, m_ObjectPtrPtrType);
                instance = m_Builder->CreateLoad(m_ObjectPtrType, instance);
                instance = m_Builder->CreateBitCast(instance, m_StructObjectPtrType);
            }

            auto tablePtr = m_Builder->CreateInBoundsGEP(m_StructObjectType, instance, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
            tablePtr = m_Builder->CreateLoad(m_TablePtrType, tablePtr);

            m_Builder->CreateCall(m_Module->getFunction(STR(TableSetIfFound)), {tablePtr, memberName, value});

            break;
        }
        case OP_REF_GLOBAL:
        {
            auto index = *ip++;
            auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(GLOBAL_VARIABLE_STR));
            auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));
            auto ref = m_Builder->CreateCall(m_Module->getFunction(STR(CreateRefObject)), {globalVar});
            Push(ref);
            break;
        }
        case OP_REF_LOCAL:
        {
            auto index = *ip++;

            auto name = GenerateLocalVarName(index);

            llvm::Value *value = nullptr;

            auto iter = localVariables.find(name);
            if (iter == localVariables.end()) // create from function argumenet
            {
                value = m_Builder->CreateCall(m_Module->getFunction(STR(GetLocalVariableSlot)), {
                                                                                                    llvm::ConstantInt::get(m_Int16Type, index),
                                                                                                });
            }
            else
            {
                if (IsObjectType(iter->second->getAllocatedType()))
                {
                    value = m_Builder->CreateLoad(iter->second->getAllocatedType(), iter->second);
                    value = CreateLlvmValue(value);
                }
                else
                    ERROR(JitCompileState::FAIL, "Cannot refer jit internal variable");
            }
            value = m_Builder->CreateCall(m_Module->getFunction(STR(GetEndOfRefValuePtr)), {value});
            value = m_Builder->CreateCall(m_Module->getFunction(STR(CreateRefObject)), {value});
            Push(value);
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            auto index = *ip++;
            auto idxValue = Pop().GetLlvmValue();

            if (idxValue->getType() != m_ValuePtrType)
                idxValue = CreateLlvmValue(idxValue);

            auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(GLOBAL_VARIABLE_STR));
            auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));

            auto refObject = m_Builder->CreateCall(m_Module->getFunction(STR(GetEndOfRefValuePtr)), {globalVar});
            refObject = m_Builder->CreateCall(m_Module->getFunction(STR(CreateIndexRefObject)), {globalVar, idxValue});
            Push(refObject);
            break;
        }
        case OP_REF_INDEX_LOCAL:
        {
            auto index = *ip++;

            auto idxValue = Pop().GetLlvmValue();
            if (idxValue->getType() != m_ValuePtrType)
                idxValue = CreateLlvmValue(idxValue);

            auto name = GenerateLocalVarName(index);

            llvm::Value *value = nullptr;

            auto iter = localVariables.find(name);
            if (iter == localVariables.end()) // create from function argumenet
            {
                value = m_Builder->CreateCall(m_Module->getFunction(STR(GetLocalVariableSlot)), {
                                                                                                    llvm::ConstantInt::get(m_Int16Type, index),
                                                                                                });
            }
            else
            {
                if (IsObjectType(iter->second->getAllocatedType()))
                {
                    value = m_Builder->CreateLoad(iter->second->getAllocatedType(), iter->second);
                    value = CreateLlvmValue(value);
                }
                else
                    ERROR(JitCompileState::FAIL, "Cannot refer jit internal variable");
            }

            value = m_Builder->CreateCall(m_Module->getFunction(STR(GetEndOfRefValuePtr)), {value});
            value = m_Builder->CreateCall(m_Module->getFunction(STR(CreateIndexRefObject)), {value, idxValue});
            Push(value);
            break;
        }
        default:
            break;
        }
    }

#ifndef NDEBUG
    m_Module->print(llvm::errs(), nullptr);
#endif

    llvm::verifyFunction(*currentCompileFunction);
    m_FPM->run(*currentCompileFunction);

    return jitFnDecl;
}

void Jit::InitModuleAndPassManager()
{
    m_FPM.reset(nullptr);
    m_Module.reset(nullptr);
    m_Context.reset(nullptr);

    m_Context = std::make_unique<llvm::LLVMContext>();

    m_Module = std::make_unique<llvm::Module>("ComputeDuck LLVM JIT", *m_Context);
    m_Module->setDataLayout(m_Executor->GetDataLayout());

    m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);

    m_FPM = std::make_unique<llvm::legacy::FunctionPassManager>(m_Module.get());
    m_FPM->add(llvm::createPromoteMemoryToRegisterPass());
    m_FPM->add(llvm::createInstructionCombiningPass());
    m_FPM->add(llvm::createReassociatePass());
    m_FPM->add(llvm::createGVNPass());
    m_FPM->add(llvm::createCFGSimplificationPass());
    m_FPM->doInitialization();

    InitTypes();
    InitInternalFunctions();

    CreateGlobalVariablesDecl();
    CreateStackDecl();
}

void Jit::InitTypes()
{
    m_Int8Type = llvm::Type::getInt8Ty(*m_Context);
    m_DoubleType = llvm::Type::getDoubleTy(*m_Context);
    m_BoolType = llvm::Type::getInt1Ty(*m_Context);
    m_Int64Type = llvm::Type::getInt64Ty(*m_Context);
    m_Int32Type = llvm::Type::getInt32Ty(*m_Context);
    m_Int16Type = llvm::Type::getInt16Ty(*m_Context);
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
    m_ObjectType->setBody({m_Int8Type, m_BoolType, m_ObjectPtrType});
    m_ObjectPtrPtrType = llvm::PointerType::get(m_ObjectPtrType, 0);

    m_StrObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_Int8PtrType, m_Int32Type, m_Int32Type}, "struct.StrObject");
    m_StrObjectPtrType = llvm::PointerType::get(m_StrObjectType, 0);

    m_ArrayObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_ValuePtrType, m_Int32Type}, "struct.ArrayObject");
    m_ArrayObjectPtrType = llvm::PointerType::get(m_ArrayObjectType, 0);

    m_RefObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_ValuePtrType}, "struct.RefObject");
    m_RefObjectPtrType = llvm::PointerType::get(m_RefObjectType, 0);

    m_BuiltinFunctionType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType, m_Int8Type, m_ValuePtrType}, false);

    m_EntryType = llvm::StructType::create(*m_Context, {m_StrObjectPtrType, m_ValueType}, "struct.Entry");
    m_EntryPtrType = llvm::PointerType::get(m_EntryType, 0);

    m_TableType = llvm::StructType::create(*m_Context, {m_Int32Type, m_Int32Type, m_EntryPtrType}, "struct.Table");
    m_TablePtrType = llvm::PointerType::get(m_TableType, 0);

    m_StructObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_TablePtrType}, "struct.StructObject");
    m_StructObjectPtrType = llvm::PointerType::get(m_StructObjectType, 0);
}

void Jit::InitInternalFunctions()
{
    llvm::FunctionType *fnType = llvm::FunctionType::get(m_Int8PtrType, {m_Int64Type}, false);
    m_Module->getOrInsertFunction(STR(malloc), fnType);

    fnType = llvm::FunctionType::get(m_Int32Type, {m_Int8PtrType, m_Int8PtrType}, false);
    m_Module->getOrInsertFunction(STR(strcmp), fnType);

    fnType = llvm::FunctionType::get(m_BoolType, {m_ObjectPtrType, m_ObjectPtrType}, false);
    m_Module->getOrInsertFunction(STR(IsObjectEqual), fnType);

    fnType = llvm::FunctionType::get(m_StrObjectPtrType, {m_StrObjectPtrType, m_StrObjectPtrType}, false);
    m_Module->getOrInsertFunction(STR(StrAdd), fnType);

    fnType = llvm::FunctionType::get(m_StrObjectPtrType, {m_Int8PtrType}, false);
    m_Module->getOrInsertFunction(STR(CreateStrObject), fnType);

    fnType = llvm::FunctionType::get(m_ArrayObjectPtrType, {m_ValuePtrType, m_Int32Type}, false);
    m_Module->getOrInsertFunction(STR(CreateArrayObject), fnType);

    fnType = llvm::FunctionType::get(m_RefObjectPtrType, {m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(CreateRefObject), fnType);

    fnType = llvm::FunctionType::get(m_StructObjectPtrType, {m_TablePtrType}, false);
    m_Module->getOrInsertFunction(STR(CreateStructObject), fnType);

    fnType = llvm::FunctionType::get(m_ValuePtrType, {m_Int16Type}, false);
    m_Module->getOrInsertFunction(STR(GetLocalVariableSlot), fnType);

    fnType = llvm::FunctionType::get(m_TablePtrType, {}, false);
    m_Module->getOrInsertFunction(STR(CreateTable), fnType);

    fnType = llvm::FunctionType::get(m_RefObjectPtrType, {m_ValuePtrType, m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(CreateIndexRefObject), fnType);

    fnType = llvm::FunctionType::get(m_BoolType, {m_TablePtrType, m_StrObjectPtrType, m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(TableSet), fnType);

    fnType = llvm::FunctionType::get(m_BoolType, {m_TablePtrType, m_StrObjectPtrType, m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(TableSetIfFound), fnType);

    fnType = llvm::FunctionType::get(m_BoolType, {m_TablePtrType, m_StrObjectPtrType, m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(TableGet), fnType);

    fnType = llvm::FunctionType::get(m_ValuePtrType, {m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(GetEndOfRefValuePtr), fnType);

    fnType = llvm::FunctionType::get(m_VoidType, {m_ValuePtrType, m_ValuePtrType, m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(ValueAdd), fnType);
    m_Module->getOrInsertFunction(STR(GetArrayObjectElement), fnType);

    fnType = llvm::FunctionType::get(m_DoubleType, {m_ValuePtrType, m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(ValueSub), fnType);
    m_Module->getOrInsertFunction(STR(ValueMul), fnType);
    m_Module->getOrInsertFunction(STR(ValueDiv), fnType);
    m_Module->getOrInsertFunction(STR(ValueBitAnd), fnType);
    m_Module->getOrInsertFunction(STR(ValueBitOr), fnType);
    m_Module->getOrInsertFunction(STR(ValueBitXor), fnType);

    fnType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType, m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(ValueGreater), fnType);
    m_Module->getOrInsertFunction(STR(ValueLess), fnType);
    m_Module->getOrInsertFunction(STR(ValueEqual), fnType);
    m_Module->getOrInsertFunction(STR(ValueLogicAnd), fnType);
    m_Module->getOrInsertFunction(STR(ValueLogicOr), fnType);

    fnType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(ValueLogicNot), fnType);

    fnType = llvm::FunctionType::get(m_DoubleType, {m_ValuePtrType}, false);
    m_Module->getOrInsertFunction(STR(ValueBitNot), fnType);
    m_Module->getOrInsertFunction(STR(ValueMinus), fnType);
}

llvm::Value *Jit::CreateLlvmValue(llvm::Value *v)
{
    auto valueType = v->getType();

    llvm::Value *vt = nullptr;
    llvm::Value *storedV = nullptr;
    llvm::Type *type = m_DoublePtrType;
    if (valueType == m_DoubleType)
    {
        vt = m_Builder->getInt8(ValueType::NUM);
        storedV = v;
    }
    else if (valueType == m_Int64Type)
    {
        vt = m_Builder->getInt8(ValueType::NUM);
        storedV = m_Builder->CreateSIToFP(v, m_DoubleType);
    }
    else if (valueType == m_BoolType)
    {
        vt = m_Builder->getInt8(ValueType::BOOL);
        storedV = m_Builder->CreateUIToFP(v, m_DoubleType);
    }
    else if (valueType == m_BoolPtrType)
    {
        vt = m_Builder->getInt8(ValueType::NIL);
        storedV = llvm::ConstantFP::get(m_DoubleType, 0.0);
    }
    else if (valueType == m_ObjectPtrType ||
             valueType == m_StrObjectPtrType ||
             valueType == m_ArrayObjectPtrType ||
             valueType == m_RefObjectPtrType)
    {
        vt = m_Builder->getInt8(ValueType::OBJECT);
        type = m_ObjectPtrPtrType;
        auto castV = m_Builder->CreateBitCast(v, m_ObjectPtrType);
        storedV = castV;
    }
    else if (valueType == m_Int8PtrType)
    {
        vt = m_Builder->getInt8(ValueType::OBJECT);
        type = m_ObjectPtrPtrType;
        // create str object
        auto strObject = m_Builder->CreateCall(m_Module->getFunction(STR(CreateStrObject)), {v});
        //to base ptr
        storedV = m_Builder->CreateBitCast(strObject, m_ObjectPtrType);
    }

    auto alloc = m_Builder->CreateAlloca(m_ValueType, nullptr);

    llvm::Value *memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(0)});
    m_Builder->CreateStore(vt, memberAddr);
    memberAddr = m_Builder->CreateInBoundsGEP(m_ValueType, alloc, {m_Builder->getInt32(0), m_Builder->getInt32(1)});
    memberAddr = m_Builder->CreateBitCast(memberAddr, type);
    m_Builder->CreateStore(storedV, memberAddr);

    return alloc;
}

llvm::Value *Jit::CreateLlvmValue(const Value &value)
{
    if (IS_NUM_VALUE(value))
    {
        llvm::Value *alloc = llvm::ConstantFP::get(m_DoubleType, TO_NUM_VALUE(value));
        return alloc;
    }
    else if (IS_BOOL_VALUE(value))
    {
        llvm::Value *alloc = llvm::ConstantInt::get(m_BoolType, TO_BOOL_VALUE(value));
        return alloc;
    }
    else if (IS_NIL_VALUE(value))
    {
        llvm::Value *alloc = llvm::ConstantPointerNull::get(m_BoolPtrType);
        return alloc;
    }
    else if (IS_STR_VALUE(value))
    {
        auto str = TO_STR_VALUE(value)->value;
        llvm::Value *strObject = m_Builder->CreateGlobalString(str);
        strObject = m_Builder->CreateBitCast(strObject, m_Int8PtrType);
        strObject = m_Builder->CreateCall(m_Module->getFunction(STR(CreateStrObject)), {strObject});
        return strObject;
    }
    else
        return nullptr;
}

void Jit::Push(llvm::Value *v)
{
    *m_StackTop++ = v;
}

void Jit::Push(const Value &v)
{
    *m_StackTop++ = v;
}

Jit::StackValue Jit::Pop()
{
    return *(--m_StackTop);
}

std::string Jit::GetTypeName(llvm::Type *type)
{
    std::string str;
    llvm::raw_string_ostream typeStream(str);
    type->print(typeStream);
    return str;
}

std::pair<llvm::Type *, uint8_t> Jit::GetTypeFromValue(const Value &v)
{
    if (IS_NUM_VALUE(v))
        return {m_DoubleType, ValueType::NUM};
    else if (IS_BOOL_VALUE(v))
        return {m_BoolType, ValueType::BOOL};
    else if (IS_NIL_VALUE(v))
        return {m_BoolPtrType, ValueType::NIL};
    else if (IS_STR_VALUE(v))
        return {m_StrObjectPtrType, ObjectType::STR};
    else if (IS_ARRAY_VALUE(v))
        return {m_ArrayObjectPtrType, ObjectType::ARRAY};
    else if (IS_REF_VALUE(v))
        return {m_RefObjectPtrType, ObjectType::REF};
    else if (IS_STRUCT_VALUE(v))
        return {m_StructObjectPtrType, ObjectType::STRUCT};
}

llvm::Type *Jit::GetLlvmTypeFromValueType(uint8_t v)
{
    switch (v)
    {
    case ValueType::NIL:
        return m_BoolPtrType;
    case ValueType::NUM:
        return m_DoubleType;
    case ValueType::BOOL:
        return m_BoolType;
    case ValueType::OBJECT:
        return m_ObjectPtrType;
    case ObjectType::STR:
        return m_StrObjectPtrType;
    case ObjectType::ARRAY:
        return m_ArrayObjectPtrType;
    case ObjectType::STRUCT:
        return m_StructObjectPtrType;
    case ObjectType::REF:
        return m_RefObjectPtrType;
    }

    return nullptr;
}

uint8_t Jit::GetValueTypeFromLlvmType(llvm::Type *v)
{
    if (v == m_BoolPtrType)
        return ValueType::NIL;
    else if (v == m_DoubleType)
        return ValueType::NUM;
    else if (v == m_BoolType)
        return ValueType::BOOL;
    else if (v == m_StrObjectPtrType || v == m_StrObjectType)
        return ObjectType::STR;
    else if (v == m_ArrayObjectPtrType || v == m_ArrayObjectType)
        return ObjectType::ARRAY;
    else if (v == m_RefObjectPtrType || v == m_RefObjectType)
        return ObjectType::REF;
    else if (v == m_StructObjectPtrType || v == m_StructObjectType)
        return ObjectType::REF;
    return ValueType::NIL;
}

bool Jit::IsObjectType(llvm::Type *type)
{
    return type == m_ObjectPtrType ||
           type == m_StrObjectPtrType ||
           type == m_ArrayObjectPtrType ||
           type == m_RefObjectPtrType ||
           type == m_StructObjectPtrType;
}

#endif