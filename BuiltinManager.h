#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "Utils.h"
#include "Object.h"
#include "Allocator.h"

class COMPUTEDUCK_API BuiltinManager
{
public:
    static BuiltinManager *GetInstance();

    void Init();

    template <typename T>
    requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value>) void Register(StrObject* name, const T &v)
    {
        auto isFound = m_BuiltinObjectsTable.Find(name);
        if (isFound)
            ASSERT("Redefined builtin:%s", ObjectStringify(name).c_str());
        m_BuiltinObjectsTable.Set(name, ALLOCATE_OBJECT(BuiltinObject, v));
    }

    BuiltinObject *FindBuiltinObject(StrObject* name);

    HashTable& GetBuiltinObjectTable();

private:
    BuiltinManager() = default;
    ~BuiltinManager() = default;

    HashTable m_BuiltinObjectsTable;
};