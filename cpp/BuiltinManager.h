#pragma once
#include <vector>
#include "Object.h"
class __declspec(dllexport) BuiltinManager
{
public:
    static BuiltinManager *GetInstance();

    void Init();
    void Release();

    void RegisterFunction(std::string_view name, const BuiltinFn &fn);
    void RegisterVariable(std::string_view name, const Value &value);

private:
    static std::unique_ptr<BuiltinManager> instance;

    BuiltinManager() = default;
    ~BuiltinManager() = default;

    friend class VM;
    friend class Compiler;

    std::vector<BuiltinFunctionObject *> m_BuiltinFunctions;
    std::vector<std::string> m_BuiltinFunctionNames;

    std::vector<BuiltinVariableObject *> m_BuiltinVariables;
    std::vector<std::string> m_BuiltinVariableNames;
};