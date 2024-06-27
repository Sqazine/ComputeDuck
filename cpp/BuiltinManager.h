#pragma once
#include <vector>
#include <unordered_map>
#include "Object.h"

class COMPUTE_DUCK_API BuiltinManager
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

    void SetExecuteFilePath(std::string_view path);
    const std::string &GetExecuteFilePath() const;

    std::string ToFullPath(std::string_view filePath);

    BuiltinObject* FindBuiltinObject(std::string_view name);

    const std::unordered_map<std::string_view, BuiltinObject*> GetBuiltinObjectList() const;

private:
    static std::unique_ptr<BuiltinManager> instance;

    BuiltinManager();
    ~BuiltinManager();

    std::string m_CurExecuteFilePath;

    std::unordered_map<std::string_view,BuiltinObject *> m_BuiltinObjects;
};