#pragma once
#include "Utils.h"
#include "Chunk.h"
#include "OpCodeVM.h"
#ifdef BUILD_WITH_LLVM
#include "LLVMJitVM.h"
#endif
enum class InterpretFlag
{
    OPCODE,
    LLVM
};

class COMPUTE_DUCK_API VM
{
public:
    VM() = default;

    ~VM()
    {
        SAFE_DELETE(m_OpCodeVM);
#ifdef BUILD_WITH_LLVM
        SAFE_DELETE(m_LLVMJitVM);
#endif
    }

    void SetInterpretFlag(InterpretFlag flag)
    {
        m_Flag = flag;
    }

    void Run(FunctionObject * mainFn)
    {
        switch (m_Flag)
        {
        case InterpretFlag::LLVM:
#ifdef BUILD_WITH_LLVM
        {
            if (!m_LLVMJitVM)
                m_LLVMJitVM = new LLVMJitVM();
            m_LLVMJitVM->Run(mainFn);
            break;
        }
#endif
        default:
        {
            if (!m_OpCodeVM)
                m_OpCodeVM = new OpCodeVM();
            m_OpCodeVM->Run(mainFn);
            break;
        }
        }
    }

private:
    InterpretFlag m_Flag{InterpretFlag::OPCODE};
    OpCodeVM *m_OpCodeVM{nullptr};
#ifdef BUILD_WITH_LLVM
    LLVMJitVM *m_LLVMJitVM{nullptr};
#endif
};