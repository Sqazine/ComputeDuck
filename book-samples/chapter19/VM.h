#pragma once
#include "Utils.h"
#include "Object.h"

// ++ 新增内容
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
#include "Jit.h"
#endif
// -- 新增内容

class COMPUTEDUCK_API VM
{
public:
    VM() = default;
    // ++ 修改内容
    // ~VM() = default;
    ~VM();
    // -- 修改内容

    void Run(FunctionObject *fn);

private:
    void Execute();

// ++ 新增内容
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    void RunJit(const struct CallFrame &frame);
    template <typename T>
    void ExecuteJitFunction(const CallFrame &frame, const std::string &fnName);
    class Jit *m_Jit{nullptr};
#endif
// -- 新增内容
};

// ++ 新增内容
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
#include "VM.inl"
#endif
// -- 新增内容
