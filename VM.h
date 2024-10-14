#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>
#include "Value.h"
#include "Object.h"
#include "Utils.h"
#include "Chunk.h"
#include "Allocator.h"

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
#include "Jit.h"
#endif

class COMPUTE_DUCK_API VM
{
public:
    VM() = default;
    ~VM();

    void Run(FunctionObject *fn);
private:
    void Execute();

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    void RunJit(const struct CallFrame& frame);
    template<typename T>
    void ExecuteJitFunction(const CallFrame& frame,const std::string& fnName);
    class Jit *m_Jit{ nullptr };
#endif
};

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
#include "VM.inl"
#endif