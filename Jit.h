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
#include <variant>

#include "Chunk.h"
#include "Value.h"
#include "Object.h"
#include "Allocator.h"

constexpr const char *g_GlobalVariablesStr = "m_GlobalVariables";
constexpr const char *g_SetGlobalVariablesFnStr = "function_SetGlobalVariables";

constexpr const char *g_StackStr = "m_ValueStack";
constexpr const char *g_SetStackFnStr = "function_SetValueStack";

class COMPUTE_DUCK_API Jit
{
public:
    Jit();
    ~Jit();

    void ResetStatus();
    bool Compile(const CallFrame &frame, const std::string &fnName);

    template<typename RetType, typename... Args>
    RetType Run(const std::string &name, Args &&...params)
    {
        auto rt = m_Executor->GetMainJITDylib().createResourceTracker();

        //set global variables
        {
            auto symbol = m_ExitOnErr(m_Executor->LookUp(g_SetGlobalVariablesFnStr));
            using SetGlobalVarFnType = void(*)(Value *);
            SetGlobalVarFnType setGlobalVarFn = reinterpret_cast<SetGlobalVarFnType>(symbol.getAddress());
            setGlobalVarFn(Allocator::GetInstance()->GetGlobalVariableRef(0));
        }

        // set stack
        {
            auto symbol = m_ExitOnErr(m_Executor->LookUp(g_SetStackFnStr));
            using SetStackFnType = void(*)(Value *);
            SetStackFnType setStackFn = reinterpret_cast<SetStackFnType>(symbol.getAddress());
            setStackFn(STACK_TOP());
        }

        auto tsm = llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context));
        m_ExitOnErr(m_Executor->AddModule(std::move(tsm), rt));
        InitModuleAndPassManager();

        auto symbol = m_ExitOnErr(m_Executor->LookUp(name));

        using JitFuncType = RetType(*)(Args...);
        JitFuncType jitFn = reinterpret_cast<JitFuncType>(symbol.getAddress());
        if constexpr (std::is_same_v<RetType, void>)
        {
            jitFn(std::forward<Args>(params)...);
            m_ExitOnErr(rt->remove());
        }
        else
        {
            RetType v = jitFn(std::forward<Args>(params)...);
            m_ExitOnErr(rt->remove());
            return v;
        }
    }

private:
    class OrcExecutor
    {
    public:
        OrcExecutor(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl);
        ~OrcExecutor();

        static std::unique_ptr<OrcExecutor> Create();

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

    template<typename VmType, typename LlvmType>
    class VariantValue
    {
    public:
        VariantValue() = default;
        VariantValue(LlvmType v) :stored(v) {}
        VariantValue(const VmType &v) :stored(v) {}
        ~VariantValue() = default;

        bool IsLlvmValue() const
        {
            return stored.index() == 1;
        }

        bool IsVmValue()
        {
            return stored.index() == 0;
        }

        LlvmType GetLlvmValue() const
        {
            return std::get<LlvmType>(stored);
        }

        const VmType &GetVmValue() const
        {
            return std::get<VmType>(stored);
        }

    private:
        std::variant<VmType, LlvmType> stored;
    };

    using StackValue = VariantValue<Value, llvm::Value *>;

    struct JumpInstrSet
    {
        llvm::BasicBlock *conditionBranch{ nullptr };
        llvm::BasicBlock *bodyBranch{ nullptr };
        llvm::BasicBlock *elseBranch{ nullptr };
        llvm::BasicBlock *endBranch{ nullptr };
    };

    enum class BranchState
    {
        IF_CONDITION,
        IF_BODY,
        IF_ELSE,
        IF_END,
        WHILE_CONDITION,
        WHILE_BODY,
        WHILE_END
    };

    void CreateSetGlobalVariablesFunction();
    void CreateGlobalVariablesDecl();

    void CreateSetStackFunction();
    void CreateStackDecl();

    void InitModuleAndPassManager();

    llvm::Value *CreateLlvmValue(llvm::Value *v);
    llvm::Value *CreateLlvmValue(const Value &value);

    void Push(llvm::Value *v);
    void Push(const Value &v);
    StackValue Pop();

    std::string GetTypeName(llvm::Type *type);

    llvm::StructType *m_ValueType{ nullptr };
    llvm::PointerType *m_ValuePtrType{ nullptr };
    llvm::StructType *m_UnionType{ nullptr };
    llvm::StructType *m_ObjectType{ nullptr };
    llvm::StructType *m_StrObjectType{ nullptr };
    llvm::StructType *m_ArrayObjectType{ nullptr };
    llvm::StructType *m_RefObjectType{ nullptr };
    llvm::PointerType *m_ObjectPtrType{ nullptr };
    llvm::PointerType *m_ObjectPtrPtrType{ nullptr };
    llvm::PointerType *m_StrObjectPtrType{ nullptr };
    llvm::PointerType *m_ArrayObjectPtrType{ nullptr };
    llvm::PointerType *m_RefObjectPtrType{ nullptr };
    llvm::FunctionType *m_BuiltinFunctionType{ nullptr };
    llvm::Type *m_Int8Type{ nullptr };
    llvm::Type *m_BoolType{ nullptr };
    llvm::Type *m_DoubleType{ nullptr };
    llvm::Type *m_Int64Type{ nullptr };
    llvm::Type *m_Int32Type{ nullptr };
    llvm::Type *m_Int16Type{ nullptr };
    llvm::Type *m_VoidType{ nullptr };
    llvm::PointerType *m_Int64PtrType{ nullptr };
    llvm::PointerType *m_Int32PtrType{ nullptr };
    llvm::PointerType *m_Int8PtrType{ nullptr };
    llvm::PointerType *m_BoolPtrType{ nullptr };
    llvm::PointerType *m_DoublePtrType{ nullptr };
    llvm::PointerType *m_Int8PtrPtrType{ nullptr };

    StackValue *m_StackTop;
    StackValue m_ValueStack[STACK_MAX];

    std::vector<JumpInstrSet> m_JumpInstrSetTable;

    std::unordered_map<std::string, llvm::Function *> m_BuiltinFnCache;

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;

    llvm::ExitOnError m_ExitOnErr;

    std::unique_ptr<OrcExecutor> m_Executor;
    std::unique_ptr<llvm::legacy::FunctionPassManager> m_FPM;
};