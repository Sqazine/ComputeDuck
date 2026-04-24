#include "Config.h"
#include <filesystem>

Config *Config::GetInstance()
{
    static Config instance;
    return &instance;
}

void Config::SetExecuteFileDirectory(std::string_view path)
{
    m_CurExecuteFileDirectory = path;
}

const std::string &Config::GetExecuteFileDirectory() const
{
    return m_CurExecuteFileDirectory;
}
