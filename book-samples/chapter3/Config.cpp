#include "Config.h"
#include <filesystem>

Config *Config::GetInstance()
{
    static Config instance;
    return &instance;
}

void Config::SetExecuteFilePath(std::string_view path)
{
    m_CurExecuteFilePath = path;
}

const std::string &Config::GetExecuteFilePath() const
{
    return m_CurExecuteFilePath;
}
