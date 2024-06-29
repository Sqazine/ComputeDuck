#pragma once

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/JITSymbol.h>
#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm-c/Core.h>

#include "Chunk.h"
#include "Value.h"
#include "Object.h"
class COMPUTE_DUCK_API LLVMJitVM
{
public:
    LLVMJitVM();
    ~LLVMJitVM();

    void Run(FunctionObject *mainFn);

protected:
    struct CallFrame
    {
        CallFrame() = default;
        ~CallFrame() = default;

        CallFrame(FunctionObject *fn, llvm::Function *llvmFn, llvm::Value **slot)
            : fn(fn), ip(fn->chunk.opCodes.data()), llvmFn(llvmFn), slot(slot)
        {
        }

        bool IsEnd()
        {
            if ((ip - fn->chunk.opCodes.data()) < fn->chunk.opCodes.size())
                return false;
            return true;
        }

        std::shared_ptr<FunctionObject> fn{nullptr};
        llvm::Function *llvmFn;
        int16_t *ip{nullptr};
        llvm::Value **slot{nullptr};
    };

    class OrcJit
    {
    public:
        OrcJit(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl);
        ~OrcJit();

        static std::unique_ptr<OrcJit> Create();

        const llvm::DataLayout &GetDataLayout() const;

        llvm::orc::JITDylib &GetMainJITDylib();

        llvm::Error AddModule(llvm::orc::ThreadSafeModule tsm, llvm::orc::ResourceTrackerSP rt = nullptr);

        llvm::Expected<llvm::JITEvaluatedSymbol> LookUp(llvm::StringRef name);

    private:
        std::unique_ptr<llvm::orc::ExecutionSession> m_Es;
        llvm::DataLayout m_DataLayout;
        llvm::orc::MangleAndInterner m_Mangle;
        llvm::orc::RTDyldObjectLinkingLayer m_ObjectLayer;
        llvm::orc::IRCompileLayer m_CompileLayer;
        llvm::orc::JITDylib &m_MainJD;
    };

private:
    void ResetStatus();

    void CompileToLLVMIR(const CallFrame &callFrame);

    void InitModuleAndPassManager();

    llvm::Value *CreateCDValue(llvm::Value *v);

    void Push(llvm::Value *v);
    llvm::Value *Pop();

    void PushCallFrame(const CallFrame &callFrame);
    CallFrame *PopCallFrame();
    CallFrame *PeekCallFrame(int32_t distance);

    std::string GetTypeName(llvm::Type* type);

    template <typename T>
    requires(std::is_same_v<T, llvm::CallInst> || std::is_same_v<T, llvm::Function>)
        T *AddBuiltinFnOrCallParamAttributes(T *fnOrCallInst)
    {
        fnOrCallInst->addRetAttr(llvm::Attribute::ZExt);
        fnOrCallInst->addParamAttr(0, llvm::Attribute::NoUndef);
        fnOrCallInst->addParamAttr(1, llvm::Attribute::NoUndef);
        fnOrCallInst->addParamAttr(1, llvm::Attribute::ZExt);
        fnOrCallInst->addParamAttr(2, llvm::Attribute::NoUndef);
        fnOrCallInst->addParamAttr(2, llvm::Attribute::NonNull);

        return fnOrCallInst;
    }

    llvm::StructType *m_ValueType{nullptr};
    llvm::PointerType *m_ValuePtrType{nullptr};

    llvm::StructType *mUnionType{nullptr};

    llvm::StructType *m_ObjectType{nullptr};
    llvm::StructType *m_StrObjectType{nullptr};

    llvm::PointerType *m_ObjectPtrType{nullptr};
    llvm::PointerType *m_ObjectPtrPtrType{nullptr};
    llvm::PointerType *m_StrObjectPtrType{nullptr};

    llvm::FunctionType *m_BuiltinFunctionType{nullptr};

    llvm::Type *m_Int8Type{nullptr};
    llvm::Type *m_BoolType{nullptr};
    llvm::Type *m_DoubleType{nullptr};
    llvm::Type *m_Int64Type{nullptr};
    llvm::Type *m_Int32Type{nullptr};
    llvm::Type *m_VoidType{nullptr};
    llvm::PointerType *m_Int64PtrType{nullptr};
    llvm::PointerType *m_Int32PtrType{nullptr};
    llvm::PointerType *m_Int8PtrType{nullptr};
    llvm::PointerType *m_BoolPtrType{nullptr};
    llvm::PointerType *m_DoublePtrType{nullptr};

    llvm::PointerType *m_Int8PtrPtrType{nullptr};

    llvm::Value *m_GlobalVariables[STACK_MAX];

    llvm::Value **m_StackTop;
    llvm::Value *m_ValueStack[STACK_MAX];

    CallFrame *m_CallFrameTop;
    CallFrame m_CallFrameStack[STACK_MAX];

    std::unordered_map<std::string, llvm::Function *> m_BuiltinFnCache;

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;

    llvm::ExitOnError m_ExitOnErr;

    std::unique_ptr<OrcJit> m_Jit;
    std::unique_ptr<llvm::legacy::FunctionPassManager> m_FPM;
};