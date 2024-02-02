#pragma once
#include <vector>
#include <unordered_map>
#include "Object.h"
#include "Config.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Value.h"

//using LlvmBuiltinFn = std::function<bool(llvm::Value*, uint8_t, llvm::Value&)>;

class COMPUTE_DUCK_API BuiltinManager
{
public:
    static BuiltinManager *GetInstance();

    void Register(std::string_view name, const BuiltinFn &fn);
    void Register(std::string_view name, const Value &value);

    void RegisterLlvmFn(std::string_view name, llvm::Function* fn);

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
    friend class LLVMCompiler;

    std::unordered_map<std::string, llvm::Function*> m_LlvmBuiltins;

    std::vector<BuiltinObject *> m_Builtins;
    std::vector<std::string> m_BuiltinNames;
};