#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>

inline void Assert(std::string_view msg)
{
    std::cout << msg << std::endl;
    exit(1);
}

inline std::string ReadFile(std::string_view path)
{
    std::fstream file;
    file.open(path.data(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        Assert("Failed to open file:" + std::string(path));

    std::stringstream sstream;
    sstream << file.rdbuf();

    file.close();

    return sstream.str();
}

inline std::string PointerAddressToString(void *pointer)
{
    std::stringstream sstr;
    sstr << pointer;
    std::string address = sstr.str();
    return address;
}