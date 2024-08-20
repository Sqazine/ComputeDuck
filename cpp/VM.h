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
#include "Jit.h"
class COMPUTE_DUCK_API VM
{
public:
    VM() = default;
    ~VM();

    void Run(FunctionObject *fn);
private:
    void Execute();

    Value FindActualValue(const Value &v);
    Value *GetEndOfRefValue(Value *v);
    Value GetEndOfRefValue(const Value &v);

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    void RunJit(FunctionObject *fn, size_t argCount);
    Jit *m_Jit{ nullptr };
#endif
};

