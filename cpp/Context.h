#pragma once
#include <string_view>
#include <unordered_map>
#include "Value.h"
class Context
{
public:
    Context();
    Context(Context *upContext);
    ~Context();

    void DefineVariableByName(std::string_view name, const Value &value);

    void AssignVariableByName(std::string_view name, const Value &value);
    Value GetVariableByName(std::string_view name);

private:
    friend class VM;

    std::unordered_map<std::string, Value> m_Values;
    Context *m_UpContext;
};