#pragma once
#include <vector>
#include <unordered_map>
#include "Object.h"
#include "Config.h"
#ifdef BUILD_WITH_LLVM
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Value.h"
#endif

// using LlvmBuiltinFn = std::function<bool(llvm::Value*, uint8_t, llvm::Value&)>;

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

#ifdef BUILD_WITH_LLVM
    void RegisterLlvmFn(std::string_view name, llvm::Function *fn);
#endif

    void SetExecuteFilePath(std::string_view path);
    const std::string &GetExecuteFilePath() const;

    std::string ToFullPath(std::string_view filePath);

private:
    static std::unique_ptr<BuiltinManager> instance;

    static std::string m_CurExecuteFilePath;

    BuiltinManager();
    ~BuiltinManager();

    friend class VM;
    friend class Compiler;
    friend class LLVMCompiler;
#ifdef BUILD_WITH_LLVM
    std::unordered_map<std::string, llvm::Function *> m_LlvmBuiltins;
#endif

    std::vector<BuiltinObject *> m_Builtins;
    std::vector<std::string> m_BuiltinNames;
};