#pragma once

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM

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
#include "JitUtils.h"
#include "Chunk.h"
#include "Value.h"
#include "Object.h"
#include "Allocator.h"

template <typename T>
concept IsStackValue = std::is_same_v<T, llvm::Value *> ||
    std::is_same_v<T, Value>;

class COMPUTE_DUCK_API Jit
{
public:
    Jit();
    ~Jit();

    void ResetStatus();
    JitFnDecl Compile(const CallFrame &frame, const std::string &fnName);

    template<typename RetType, typename... Args>
    RetType Run(const std::string &name, Args &&...params)
    {
        auto rt = m_Executor->GetMainJITDylib().createResourceTracker();

        //set global variables
        {
            auto symbol = m_ExitOnErr(m_Executor->LookUp(SET_GLOBAL_VARIABLE_FN_STR));
            using SetGlobalVarFnType = void(*)(Value *);
            SetGlobalVarFnType setGlobalVarFn = reinterpret_cast<SetGlobalVarFnType>(symbol.getAddress());
            setGlobalVarFn(Allocator::GetInstance()->GetGlobalVariableRef(0));
        }

        // set stack
        {
            auto symbol = m_ExitOnErr(m_Executor->LookUp(SET_STACK_FN_STR));
            using SetStackFnType = void(*)(Value *);
            SetStackFnType setStackFn = reinterpret_cast<SetStackFnType>(symbol.getAddress());
            setStackFn(STACK_TOP());
        }

        m_Executor->AddModule(llvm::orc::ThreadSafeModule(std::move(m_Module), std::move(m_Context)));
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
        OrcExecutor(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl)
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

        ~OrcExecutor()
        {
            if (auto Err = m_Es->endSession())
                m_Es->reportError(std::move(Err));
        }

        static std::unique_ptr<OrcExecutor> Create()
        {
            auto epc = llvm::orc::SelfExecutorProcessControl::Create();
            if (!epc)
                return nullptr;

            auto es = std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));

            llvm::orc::JITTargetMachineBuilder JTMB(es->getExecutorProcessControl().getTargetTriple());

            auto dataLayout = JTMB.getDefaultDataLayoutForTarget();
            if (!dataLayout)
                return nullptr;

            return std::make_unique<OrcExecutor>(std::move(es), std::move(JTMB), std::move(*dataLayout));
        }

        const llvm::DataLayout &GetDataLayout() const
        {
            return m_DataLayout;
        }

        llvm::orc::JITDylib &GetMainJITDylib()
        {
            return m_MainJD;
        }

        llvm::Error AddModule(llvm::orc::ThreadSafeModule tsm, llvm::orc::ResourceTrackerSP rt = nullptr)
        {
            if (!rt)
                rt = m_MainJD.getDefaultResourceTracker();
            return m_CompileLayer.add(rt, std::move(tsm));
        }

        llvm::Expected<llvm::JITEvaluatedSymbol> LookUp(llvm::StringRef name)
        {
            return m_Es->lookup({&m_MainJD}, m_Mangle(name.str()));
        }

    private:
        std::unique_ptr<llvm::orc::ExecutionSession> m_Es;
        llvm::DataLayout m_DataLayout;
        llvm::orc::MangleAndInterner m_Mangle;
        llvm::orc::RTDyldObjectLinkingLayer m_ObjectLayer;
        llvm::orc::IRCompileLayer m_CompileLayer;
        llvm::orc::JITDylib &m_MainJD;
    };


    class StackValue
    {
    public:
        StackValue() = default;
        StackValue(llvm::Value *v) :stored(v) {}
        StackValue(const Value &v) :stored(v) {}
        ~StackValue() = default;

        template <IsStackValue T>
        bool Is()
        {
            return std::holds_alternative<T>(stored);
        }

        template <IsStackValue T>
        T Get()
        {
            return std::get<T>(stored);
        }

    private:
        std::variant<Value, llvm::Value *> stored;
    };

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
    void InitTypes();
    void InitInternalFunctions();

    llvm::Value *CreateLlvmValue(llvm::Value *v);
    llvm::Value *CreateLlvmValue(const Value &value);

    void Push(llvm::Value *v);
    void Push(const Value &v);
    StackValue Pop();

    std::string GetTypeName(llvm::Type *type);

    std::pair<llvm::Type *, uint8_t> GetTypeFromValue(const Value &v);
    llvm::Type *GetLlvmTypeFromValueType(uint8_t v);
    uint8_t GetValueTypeFromLlvmType(llvm::Type *v);

    bool IsObjectType(llvm::Type* type);

    llvm::StructType *m_UnionType{ nullptr };

    llvm::StructType *m_ValueType{ nullptr };
    llvm::PointerType *m_ValuePtrType{ nullptr };

    llvm::StructType *m_ObjectType{ nullptr };

    llvm::PointerType *m_ObjectPtrType{ nullptr };
    llvm::PointerType *m_ObjectPtrPtrType{ nullptr };

    llvm::StructType *m_StrObjectType{ nullptr };
    llvm::PointerType *m_StrObjectPtrType{ nullptr };

    llvm::StructType *m_ArrayObjectType{ nullptr };
    llvm::PointerType *m_ArrayObjectPtrType{ nullptr };

    llvm::StructType *m_RefObjectType{ nullptr };
    llvm::PointerType *m_RefObjectPtrType{ nullptr };

    llvm::StructType *m_StructObjectType{ nullptr };
    llvm::PointerType *m_StructObjectPtrType{ nullptr };

    llvm::FunctionType *m_BuiltinFunctionType{ nullptr };

    llvm::Type *m_Int8Type{ nullptr };
    llvm::PointerType *m_Int8PtrType{ nullptr };
    llvm::PointerType *m_Int8PtrPtrType{ nullptr };

    llvm::Type *m_BoolType{ nullptr };
    llvm::PointerType *m_BoolPtrType{ nullptr };

    llvm::Type *m_DoubleType{ nullptr };
    llvm::PointerType *m_DoublePtrType{ nullptr };

    llvm::Type *m_Int64Type{ nullptr };
    llvm::PointerType *m_Int64PtrType{ nullptr };

    llvm::Type *m_Int32Type{ nullptr };
    llvm::PointerType *m_Int32PtrType{ nullptr };

    llvm::Type *m_Int16Type{ nullptr };
    llvm::Type *m_VoidType{ nullptr };

    llvm::StructType *m_EntryType{ nullptr };
    llvm::PointerType *m_EntryPtrType{ nullptr };

    llvm::StructType *m_TableType{ nullptr };
    llvm::PointerType *m_TablePtrType{ nullptr };

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

#endif