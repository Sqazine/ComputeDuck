#include "Jit.h"
#include "Allocator.h"
#include "BuiltinManager.h"
#include "VM.h"

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
        m_Module->setModuleIdentifier(g_SetGlobalVariablesFnStr);
        m_Module->setSourceFileName(g_SetGlobalVariablesFnStr);
    }

    m_Module->getOrInsertGlobal(g_GlobalVariablesStr, m_ValuePtrType);
    auto globalVariable = m_Module->getNamedGlobal(g_GlobalVariablesStr);
    globalVariable->setInitializer(llvm::ConstantPointerNull::get(m_ValuePtrType));
    globalVariable->setAlignment(llvm::MaybeAlign(8));
    globalVariable->setDSOLocal(true);

    llvm::FunctionType *fnType = llvm::FunctionType::get(m_VoidType, {m_ValuePtrType}, false);
    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, g_SetGlobalVariablesFnStr, m_Module.get());
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
    m_Module->getOrInsertGlobal(g_GlobalVariablesStr, m_ValuePtrType);
    auto globalVariable = m_Module->getNamedGlobal(g_GlobalVariablesStr);
    globalVariable->setLinkage(llvm::GlobalVariable::ExternalLinkage);
    globalVariable->setAlignment(llvm::MaybeAlign(8));
}

void Jit::CreateSetStackFunction()
{
    {
        m_Module->setModuleIdentifier(g_SetStackFnStr);
        m_Module->setSourceFileName(g_SetStackFnStr);
    }

    m_Module->getOrInsertGlobal(g_StackStr, m_ValuePtrType);
    auto stackVariable = m_Module->getNamedGlobal(g_StackStr);
    stackVariable->setInitializer(llvm::ConstantPointerNull::get(m_ValuePtrType));
    stackVariable->setAlignment(llvm::MaybeAlign(8));
    stackVariable->setDSOLocal(true);

    llvm::FunctionType *fnType = llvm::FunctionType::get(m_VoidType, {m_ValuePtrType}, false);
    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, g_SetStackFnStr, m_Module.get());
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
    m_Module->getOrInsertGlobal(g_StackStr, m_ValuePtrType);
    auto stackTop = m_Module->getNamedGlobal(g_StackStr);
    stackTop->setLinkage(llvm::GlobalVariable::ExternalLinkage);
    stackTop->setAlignment(llvm::MaybeAlign(8));
}

void Jit::ResetStatus()
{
    m_BuiltinFnCache.clear();

    m_StackTop = m_ValueStack;
}

bool Jit::Compile(const CallFrame &frame, const std::string &fnName)
{
    {
        m_Module->setSourceFileName(fnName);
        m_Module->setModuleIdentifier(fnName);

        CreateGlobalVariablesDecl();
        CreateStackDecl();
    }

    std::map<std::string, llvm::AllocaInst *> localVariables;

    BranchState branchState = BranchState::IF_CONDITION;

    std::vector<llvm::Type *> paramTypes;
    for (Value *slot = frame.slot; slot < frame.slot + frame.fn->parameterCount; ++slot)
    {
        if (IS_NUM_VALUE(*slot))
            paramTypes.push_back(m_DoubleType);
        else if (IS_STR_VALUE(*slot))
            paramTypes.push_back(m_Int8PtrType);
        else if (IS_ARRAY_VALUE(*slot))
            paramTypes.push_back(m_ArrayObjectPtrType);
        else if (IS_REF_VALUE(*slot))
            paramTypes.push_back(m_RefObjectPtrType);
    }

    llvm::FunctionType *fnType = nullptr;
    if (frame.fn->probableReturnTypeSet->IsNone())
        fnType = llvm::FunctionType::get(m_VoidType, paramTypes, false);
    else if (frame.fn->probableReturnTypeSet->IsOnly(ValueType::NUM))
        fnType = llvm::FunctionType::get(m_DoubleType, paramTypes, false);
    else if (frame.fn->probableReturnTypeSet->IsOnly(ValueType::NIL))
        fnType = llvm::FunctionType::get(m_BoolPtrType, paramTypes, false);
    else if (frame.fn->probableReturnTypeSet->IsOnly(ValueType::BOOL))
        fnType = llvm::FunctionType::get(m_BoolType, paramTypes, false);
    else if (frame.fn->probableReturnTypeSet->IsOnly(ObjectType::STR))
        fnType = llvm::FunctionType::get(m_ValuePtrType, paramTypes, false);
    else if (frame.fn->probableReturnTypeSet->IsOnly(ObjectType::ARRAY))
        fnType = llvm::FunctionType::get(m_ArrayObjectPtrType, paramTypes, false);
    else if (frame.fn->probableReturnTypeSet->IsOnly(ObjectType::REF))
        fnType = llvm::FunctionType::get(m_RefObjectPtrType, paramTypes, false);
    else
        ERROR("Unknown return type");

    llvm::Function *fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName.c_str(), m_Module.get());
    fn->setDSOLocal(true);
    for (auto i = 0; i < paramTypes.size(); ++i)
        fn->addParamAttr(i, llvm::Attribute::NoUndef);

    llvm::BasicBlock *codeBlock = llvm::BasicBlock::Create(*m_Context, "", fn);
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
            {
                m_Module->getFunctionList().pop_back(); // pop current invalid function
                ERROR("Not support jit compile for:%s", fnName.c_str());
            }
            else
            {
                auto llvmValue = CreateLlvmValue(value);
                if (!llvmValue)
                    ERROR("Unsupported value type:%d", value.type);

                Push(llvmValue);
            }
            break;
        }
        case OP_ADD:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
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
                ERROR("Invalid binary op:%s + %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

            break;
        }
        case OP_SUB:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFSub(left, right));
            else
                ERROR("Invalid binary op:%s - %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_MUL:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFMul(left, right));
            else
                ERROR("Invalid binary op:%s * %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_DIV:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFDiv(left, right));
            else
                ERROR("Invalid binary op:%s / %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_LESS:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpULT(left, right));
            else
                ERROR("Invalid binary op:%s < %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_GREATER:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_DoubleType && right->getType() == m_DoubleType)
                Push(m_Builder->CreateFCmpUGT(left, right));
            else
                ERROR("Invalid binary op:%s > %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());
            break;
        }
        case OP_NOT:
        {
            auto value = Pop().GetLlvmValue();
            if (value->getType() == m_BoolType)
                Push(m_Builder->CreateNot(value));
            else
                ERROR("Invalid binary op:not %s.", GetTypeName(value->getType()).c_str());

            break;
        }
        case OP_MINUS:
        {
            auto value = Pop().GetLlvmValue();
            if (value->getType() == m_DoubleType)
                Push(m_Builder->CreateNeg(value));
            else
                ERROR("Invalid binary op:- %s.", GetTypeName(value->getType()).c_str());

            break;
        }
        case OP_EQUAL:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
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

                            auto result = m_Builder->CreateCall(m_Module->getFunction("strcmp"), {leftCharsPtr, rightCharsPtr});

                            Push(m_Builder->CreateICmpEQ(result, llvm::ConstantInt::get(m_Int32Type, 0)));
                        }
                    }
                }
            }
            else
                ERROR("Invalid binary op:%s == %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

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

            llvm::Value *alloc = m_Builder->CreateAlloca(m_ValuePtrType);

            auto mallocation = m_Builder->CreateCall(m_Module->getFunction("malloc"), {llvm::ConstantInt::get(m_Int64Type, numElements * sizeof(Value))});
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

            m_StackTop -= numElements;

            auto arrayObject = m_Builder->CreateCall(m_Module->getFunction("CreateArrayObject"), {load, llvm::ConstantInt::get(m_Int32Type, numElements)});
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
                ERROR("Invalid binary op:%s and %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

            break;
        }
        case OP_OR:
        {
            auto left = Pop().GetLlvmValue();
            auto right = Pop().GetLlvmValue();
            if (left->getType() == m_BoolType && right->getType() == m_BoolType)
                Push(m_Builder->CreateLogicalAnd(left, right));
            else
                ERROR("Invalid binary op:%s or %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

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
                ERROR("Invalid binary op:%s & %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

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
                ERROR("Invalid binary op:%s | %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

            break;
        }
        case OP_BIT_NOT:
        {
            auto value = Pop().GetLlvmValue();
            if (value->getType() == m_BoolType)
            {
                value = m_Builder->CreateFPToSI(value, m_Int64Type);
                Push(m_Builder->CreateXor(value, -1));
            }
            else
                ERROR("Invalid binary op:~ %s.", GetTypeName(value->getType()).c_str());

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
                ERROR("Invalid binary op:%s ^ %s.", GetTypeName(left->getType()).c_str(), GetTypeName(right->getType()).c_str());

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
            }

            if (!isSatis)
                ERROR("Invalid index op: %s[%s]", GetTypeName(ds->getType()).c_str(), GetTypeName(index->getType()).c_str());

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
                ERROR("Invalid index op: %s[%s]", GetTypeName(ds->getType()).c_str(), GetTypeName(index->getType()).c_str());
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
                m_Builder->CreateRet(value);
            }
            else
                m_Builder->CreateRetVoid();
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = *ip++;
            auto v = Pop().GetLlvmValue();

            auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(g_GlobalVariablesStr));
            auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));

            if (v->getType() != m_ValueType)
            {
                auto cdValue = CreateLlvmValue(v);
                m_Builder->CreateMemCpy(globalVar, llvm::MaybeAlign(8), cdValue, llvm::MaybeAlign(8), m_Builder->getInt64(sizeof(Value)));
            }
            else
                m_Builder->CreateStore(v, globalVar);

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
                auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(g_GlobalVariablesStr));
                auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));

                if (globalVar && globalVar->getType()->isPointerTy())
                {
                    auto ptrType = static_cast<llvm::PointerType *>(globalVar->getType());
                    if (ptrType->getElementType() == m_ValueType)
                        Push(globalVar);
                    else
                    {
                        auto v = m_Builder->CreateLoad(ptrType->getElementType(), globalVar);
                        Push(v);
                    }
                }
                else
                    ERROR("Not finished yet,tag it as error");
            }
            break;
        }
        case OP_SET_LOCAL:
        {
            auto scopeDepth = *ip++;
            auto index = *ip++;
            auto isUpValue = *ip++;
            auto value = Pop().GetLlvmValue();

            auto name = "localVar_" + std::to_string(scopeDepth) + "_" + std::to_string(index) + "_" + std::to_string(isUpValue);

            auto iter = localVariables.find(name);
            if (iter == localVariables.end())
            {           
                auto alloc = m_Builder->CreateAlloca(value->getType());
                m_Builder->CreateStore(value, alloc);

                localVariables[name] = alloc;
            }
            else
                m_Builder->CreateStore(value, iter->second);

            break;
        }
        case OP_GET_LOCAL:
        {
            auto scopeDepth = *ip++;
            auto index = *ip++;
            auto isUpValue = *ip++;

            auto name = "localVar_" + std::to_string(scopeDepth) + "_" + std::to_string(index) + "_" + std::to_string(isUpValue);

            auto iter = localVariables.find(name);
            if (iter == localVariables.end()) // create from function argumenet
            {
                auto parentFn = m_Builder->GetInsertBlock()->getParent();
                if (isUpValue == 0 && index < parentFn->arg_size()) // load from function arguments
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
                    auto slot = m_Builder->CreateCall(m_Module->getFunction("GetLocalVariableSlot"), {llvm::ConstantInt::get(m_Int16Type, scopeDepth),
                                                                                                      llvm::ConstantInt::get(m_Int16Type, index),
                                                                                                      llvm::ConstantInt::get(m_BoolType, isUpValue)});
                    
                    auto alloc=m_Builder->CreateAlloca(slot->getType());
                    m_Builder->CreateStore(slot,alloc);

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
                auto fn = TO_FUNCTION_VALUE(fnSlot->GetVmValue());

                size_t paramTypeHash = 0;
                for (auto slot = m_StackTop - fn->parameterCount; slot < m_StackTop; ++slot)
                {
                    if (slot->GetLlvmValue()->getType() == m_DoubleType)
                        paramTypeHash ^= std::hash<ValueType>()(ValueType::NUM);
                }

                auto fnName = "function_" + fn->uuid + "_" + std::to_string(paramTypeHash);

                auto iter = fn->jitCache.find(paramTypeHash);
                if (iter != fn->jitCache.end() && !fn->probableReturnTypeSet->IsMultiplyType())
                {
                    std::vector<llvm::Value *> args;
                    for (auto slot = m_StackTop - argCount; slot < m_StackTop; ++slot)
                        args.emplace_back(slot->GetLlvmValue());

                    m_StackTop = m_StackTop - argCount - 1;

                    llvm::Value *ret = m_Builder->CreateCall(m_Module->getFunction(fnName), args);
                    Push(ret);
                }
                else
                    ERROR("Not jitted function");
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
            ERROR("Not finished yet,tag it as error");
            break;
        }
        case OP_GET_STRUCT:
        {
            ERROR("Not finished yet,tag it as error");
            break;
        }
        case OP_SET_STRUCT:
        {
            ERROR("Not finished yet,tag it as error");
            break;
        }
        case OP_REF_GLOBAL:
        {
            auto index = *ip++;

            auto globArray = m_Builder->CreateLoad(m_ValuePtrType, m_Module->getNamedGlobal(g_GlobalVariablesStr));
            auto globalVar = m_Builder->CreateInBoundsGEP(m_ValueType, globArray, llvm::ConstantInt::get(m_Int16Type, index));

            auto alloc = m_Builder->CreateCall(m_Module->getFunction("CreateRefObject"), {globalVar});
            Push(alloc);
            break;
        }
        case OP_REF_LOCAL:
        {
            auto scopeDepth = *ip++;
            auto index = *ip++;
            auto isUpValue = *ip++;

            auto name = "localVar_" + std::to_string(scopeDepth) + "_" + std::to_string(index) + "_" + std::to_string(isUpValue);

            auto iter = localVariables.find(name);
            if (iter == localVariables.end()) // create from function argumenet
            {
                auto slot = m_Builder->CreateCall(m_Module->getFunction("GetLocalVariableSlot"), {llvm::ConstantInt::get(m_Int16Type, scopeDepth),
                                                                                                  llvm::ConstantInt::get(m_Int16Type, index),
                                                                                                  llvm::ConstantInt::get(m_BoolType, isUpValue)});
                
                auto alloc = m_Builder->CreateCall(m_Module->getFunction("CreateRefObject"), {slot});
                Push(alloc);
            }
            else
            {
                if (iter->second->getType() != m_ValuePtrType)
                {
                   ERROR("Cannot refer jit internal variable");
                }
                else
                {
                    auto alloc = m_Builder->CreateCall(m_Module->getFunction("CreateRefObject"), {iter->second});
                    Push(alloc);
                }
            }
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            ERROR("Not finished yet,tag it as error");
            break;
        }
        case OP_REF_INDEX_LOCAL:
        {
            ERROR("Not finished yet,tag it as error");
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

#ifndef NDEBUG
    m_Module->print(llvm::errs(), nullptr);
#endif

    llvm::verifyFunction(*fn);
    m_FPM->run(*fn);

    m_Executor->AddModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context)));
    InitModuleAndPassManager();

    return true;
}

void Jit::InitModuleAndPassManager()
{
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

        m_StrObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_Int8PtrType, m_Int32Type,m_Int32Type}, "struct.StrObject");
        m_StrObjectPtrType = llvm::PointerType::get(m_StrObjectType, 0);

        m_ArrayObjectType = llvm::StructType::create(*m_Context, {m_ObjectType, m_ValuePtrType, m_Int32Type}, "struct.ArrayObject");
        m_ArrayObjectPtrType = llvm::PointerType::get(m_ArrayObjectType, 0);

        m_RefObjectType = llvm::StructType::create(*m_Context, {m_ValuePtrType}, "struct.RefObject");
        m_RefObjectPtrType = llvm::PointerType::get(m_RefObjectType, 0);

        m_BuiltinFunctionType = llvm::FunctionType::get(m_BoolType, {m_ValuePtrType, m_Int8Type, m_ValuePtrType}, false);
    }

    llvm::FunctionType *mallocFnType = llvm::FunctionType::get(m_Int8PtrType, {m_Int64Type}, false);
    m_Module->getOrInsertFunction("malloc", mallocFnType);

    llvm::FunctionType *strcmpFnType = llvm::FunctionType::get(m_Int32Type, {m_Int8PtrType, m_Int8PtrType}, false);
    m_Module->getOrInsertFunction("strcmp", strcmpFnType);

    llvm::FunctionType *createStrObjectFnType = llvm::FunctionType::get(m_StrObjectPtrType, {m_Int8PtrType}, false);
    m_Module->getOrInsertFunction("CreateStrObject", createStrObjectFnType);

    llvm::FunctionType *createArrayObjectFnType = llvm::FunctionType::get(m_ArrayObjectPtrType, {m_ValuePtrType, m_Int32Type}, false);
    m_Module->getOrInsertFunction("CreateArrayObject", createArrayObjectFnType);

    llvm::FunctionType *createRefObjectFnType = llvm::FunctionType::get(m_RefObjectPtrType, {m_ValuePtrType}, false);
    m_Module->getOrInsertFunction("CreateRefObject", createRefObjectFnType);

    llvm::FunctionType *getLocalVariableSlotFnType = llvm::FunctionType::get(m_ValuePtrType, {m_Int16Type, m_Int16Type, m_BoolType}, false);
    m_Module->getOrInsertFunction("GetLocalVariableSlot", getLocalVariableSlotFnType);
}

llvm::Value *Jit::CreateLlvmValue(llvm::Value *v)
{
    auto valueType = v->getType();

    llvm::Value *vt = nullptr;
    llvm::Value *storedV = nullptr;
    llvm::Type *type = m_DoublePtrType;
    if (valueType == m_DoubleType)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NUM));
        storedV = v;
    }
    else if (valueType == m_Int64Type)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NUM));
        storedV = m_Builder->CreateSIToFP(v, m_DoubleType);
    }
    else if (valueType == m_BoolType)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::BOOL));
        storedV = m_Builder->CreateUIToFP(v, m_DoubleType);
    }
    else if (valueType == m_BoolPtrType)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::NIL));
        storedV = llvm::ConstantFP::get(m_DoubleType, 0.0);
    }
    else if (valueType == m_ObjectPtrType || valueType == m_ArrayObjectPtrType)
    {
        vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));
        type = m_ObjectPtrPtrType;
        auto castV = m_Builder->CreateBitCast(v, m_ObjectPtrType);
        storedV = castV;
    }
    else if (valueType->isPointerTy())
    {
        auto vPtrType = static_cast<llvm::PointerType *>(valueType);
        if (vPtrType->getElementType()->isArrayTy())
        {
            auto vArrayType = static_cast<llvm::ArrayType *>(vPtrType->getElementType());
            if (vArrayType->getElementType() == m_Int8Type)
            {
                vt = m_Builder->getInt8(std::underlying_type<ValueType>::type(ValueType::OBJECT));
                type = m_ObjectPtrPtrType;
                // convert chars[] to i8*
                auto charsPtr = m_Builder->CreateInBoundsGEP(vArrayType, v, {m_Builder->getInt64(0), m_Builder->getInt64(0)});
                // create str object
                auto strObject = m_Builder->CreateCall(m_Module->getFunction("CreateStrObject"), {charsPtr});
                //to base ptr
                storedV = m_Builder->CreateBitCast(strObject, m_ObjectPtrType);
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
        llvm::Value *chars = m_Builder->CreateGlobalString(str);
        return chars;
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