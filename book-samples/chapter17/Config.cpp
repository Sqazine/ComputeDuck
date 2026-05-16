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

// ++ 新增内容
std::string Config::ToFullPath(std::string_view filePath)
{
    std::filesystem::path filesysPath = filePath;
    std::string fullPath = filesysPath.string();
    if (!filesysPath.is_absolute())
        fullPath = m_CurExecuteFileDirectory + fullPath;
    return fullPath;
}
// -- 新增内容