#pragma once
#include <vector>
#include "Object.h"
#include "Config.h"
class COMPUTE_DUCK_API BuiltinManager
{
public:
    static BuiltinManager *GetInstance();

    void Register(std::string_view name, const BuiltinFn &fn);
    void Register(std::string_view name, const Value &value);

    void SetExecuteFilePath(std::string_view path);
    const std::string& GetExecuteFilePath() const;

    std::string ToFullPath(std::string_view filePath);
private:
    static std::unique_ptr<BuiltinManager> instance;

    static std::string m_CurExecuteFilePath;

    BuiltinManager();
    ~BuiltinManager();

    friend class VM;
    friend class Compiler;

    std::vector<BuiltinObject *> m_Builtins;
    std::vector<std::string> m_BuiltinNames;
};