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

    // ++ 新增内容
    std::string ToFullPath(std::string_view filePath);
    // -- 新增内容
private:
    Config() = default;
    ~Config() = default;

    std::string m_CurExecuteFileDirectory;
};