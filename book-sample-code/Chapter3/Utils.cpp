#include "Utils.h"

std::string ReadFile(std::string_view path)
{
    std::fstream file;
    file.open(path.data(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        ASSERT("Failed to open file:%s", path.data());

    std::stringstream sstream;
    sstream << file.rdbuf();

    file.close();

    return sstream.str();
}

void WriteFile(std::string_view path,std::string_view content)
{
    std::fstream f;
	f.open(path.data(),std::ios::out);
	f<<content;
	f.close();
}