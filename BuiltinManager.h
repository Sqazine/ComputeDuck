#pragma once
#include <vector>
#include <unordered_map>
#include "Object.h"

class COMPUTE_DUCK_API BuiltinManager
{
public:
    static BuiltinManager *GetInstance();

    template <typename T>
    void Register(std::string_view name, const T &v)
    {
        m_Builtins.emplace_back(new BuiltinObject(name, v));
        m_BuiltinNames.emplace_back(name);
    }

    void SetExecuteFilePath(std::string_view path);
    const std::string &GetExecuteFilePath() const;

    std::string ToFullPath(std::string_view filePath);

private:
    static std::unique_ptr<BuiltinManager> instance;

    BuiltinManager();
    ~BuiltinManager();

    std::string m_CurExecuteFilePath;

    friend class VM;
    friend class OpCodeCompilerImpl;
    friend class LLVMCompilerImpl;

    std::vector<BuiltinObject *> m_Builtins;
    std::vector<std::string> m_BuiltinNames;
};