#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "Utils.h"
#include "Object.h"

class COMPUTEDUCK_API BuiltinManager
{
public:
    static BuiltinManager *GetInstance();

    template <typename T>
        requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value>)
    void Register(std::string_view name, const T &v)
    {
        auto iter = m_BuiltinObjects.find(name);
        if (iter != m_BuiltinObjects.end())
            ASSERT("Redefined builtin:%s", name.data());
        m_BuiltinObjects[name] = new BuiltinObject(name, v);
    }

    BuiltinObject *FindBuiltinObject(std::string_view name);

    const std::unordered_map<std::string_view, BuiltinObject *> GetBuiltinObjectList() const;

private:
    BuiltinManager();
    ~BuiltinManager();

    std::unordered_map<std::string_view, BuiltinObject *> m_BuiltinObjects;
};