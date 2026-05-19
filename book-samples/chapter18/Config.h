#pragma once
#include <memory>
#include <string>
#include "Utils.h"
class COMPUTEDUCK_API Config
{
public:
    static Config *GetInstance();

    void SetExecuteFileDirectory(std::string_view path);
    const std::string &GetExecuteFileDirectory() const;

    std::string ToFullPath(std::string_view filePath);
private:
    Config() = default;
    ~Config() = default;

    std::string m_CurExecuteFileDirectory;
};