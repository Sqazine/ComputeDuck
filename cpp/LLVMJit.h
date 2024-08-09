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
class COMPUTE_DUCK_API LLVMJit
{
public:
    LLVMJit(class VM* vm);
    ~LLVMJit();

    bool Compile(FunctionObject* fnObj,const std::string& fnName);

    template<typename T>
        requires(!std::is_same_v<T, void>)
    T Run(const std::string& name)
    {
        auto rt = m_Jit->GetMainJITDylib().createResourceTracker();

        auto tsm = llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context));
        m_ExitOnErr(m_Jit->AddModule(std::move(tsm), rt));
        InitModuleAndPassManager();
        
        auto symbol = m_ExitOnErr(m_Jit->LookUp(name));

        using JitFuncType = T(*)();
        JitFuncType jitFn = reinterpret_cast<JitFuncType>(symbol.getAddress());

        T v = jitFn();
        m_ExitOnErr(rt->remove());
        return v;
    }

    template<typename T>
        requires(std::is_same_v<T, void>)
    T Run(const std::string& name)
    {
        auto rt = m_Jit->GetMainJITDylib().createResourceTracker();
      
        auto tsm = llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context));
        m_ExitOnErr(m_Jit->AddModule(std::move(tsm), rt));
        InitModuleAndPassManager();

        auto symbol = m_ExitOnErr(m_Jit->LookUp(name));

        using JitFuncType = void(*)();
        JitFuncType jitFn = reinterpret_cast<JitFuncType>(symbol.getAddress());

        jitFn();
        m_ExitOnErr(rt->remove());
    }

private:
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

    struct JumpInstrSet
    {
        llvm::BasicBlock* conditionBranch{ nullptr };
        llvm::BasicBlock* bodyBranch{ nullptr };
        llvm::BasicBlock* elseBranch{ nullptr };
        llvm::BasicBlock* endBranch{ nullptr };
    };

    void ResetStatus();

    void InitModuleAndPassManager();

    llvm::Value *CreateCDValue(llvm::Value *v);

    void Push(llvm::Value *v);
    llvm::Value *Pop();

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

    class VM* m_VM{nullptr};

    llvm::StructType *m_ValueType{nullptr};
    llvm::PointerType *m_ValuePtrType{nullptr};

    llvm::StructType *m_UnionType{nullptr};

    llvm::StructType *m_ObjectType{nullptr};
    llvm::StructType *m_StrObjectType{nullptr};
    llvm::StructType *m_ArrayObjectType{nullptr};

    llvm::PointerType *m_ObjectPtrType{nullptr};
    llvm::PointerType *m_ObjectPtrPtrType{nullptr};
    llvm::PointerType *m_StrObjectPtrType{nullptr};
    llvm::PointerType *m_ArrayObjectPtrType{nullptr};

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

    std::vector<JumpInstrSet> m_JumpInstrSetTable;

    std::unordered_map<std::string, llvm::Function *> m_BuiltinFnCache;

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;

    llvm::ExitOnError m_ExitOnErr;

    std::unique_ptr<OrcJit> m_Jit;
    std::unique_ptr<llvm::legacy::FunctionPassManager> m_FPM;
};