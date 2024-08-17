#pragma once
#include <memory>
#include <string>
#include "Utils.h"
class COMPUTE_DUCK_API Config
{
public:
    static Config *GetInstance();

    void SetExecuteFilePath(std::string_view path);
    const std::string &GetExecuteFilePath() const;

    std::string ToFullPath(std::string_view filePath);

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    void SetUseJit(bool b);
    bool IsUseJit();

private:
    bool m_UseJit{true};
#endif

private:
    Config() = default;
    ~Config() = default;

    std::string m_CurExecuteFilePath;
};