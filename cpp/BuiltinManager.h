#pragma once
#include <vector>
#include "Object.h"
class BuiltinManager
{
public:
    ~BuiltinManager() {}

    static void Init();
    static void Release();

    static void RegisterFunction(std::string_view name, const BuiltinFn &fn);
    static void RegisterVariable(std::string_view name, const Value &value);

private:
    BuiltinManager() {}

    friend class VM;
    friend class Compiler;

    static std::vector<BuiltinFunctionObject *> m_BuiltinFunctions;
    static std::vector<std::string> m_BuiltinFunctionNames;

    static std::vector<BuiltinVariableObject *> m_BuiltinVariables;
    static std::vector<std::string> m_BuiltinVariableNames;
};