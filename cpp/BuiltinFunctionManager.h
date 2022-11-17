#pragma once
#include <vector>
#include "Object.h"
class BuiltinFunctionManager
{
public:
    ~BuiltinFunctionManager();

    static void Init();
    static void Release();

    static void Register(std::string_view name, const BuiltinFn &fn);
private:
    BuiltinFunctionManager();

    friend class VM;
    friend class Compiler;

    static std::vector<BuiltinFunctionObject *> m_Builtins;
    static std::vector<std::string> m_BuiltinIdx;
};