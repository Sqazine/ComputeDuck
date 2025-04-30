#pragma once
#include <memory>
#include <string>
#include "Utils.h"
class COMPUTEDUCK_API Config
{
public:
    static Config *GetInstance();

    void SetExecuteFilePath(std::string_view path);
    const std::string& GetExecuteFilePath() const;
private:
    Config() = default;
    ~Config() = default;

    std::string m_CurExecuteFilePath;
};