#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>

#define ASSERT(...)                                                                 \
    do                                                                              \
    {                                                                               \
        printf("[file:%s,function:%s,line:%d]:", __FILE__, __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__);                                                        \
        exit(1);                                                                    \
    } while (false);

inline std::string ReadFile(std::string_view path)
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

inline std::string PointerAddressToString(void *pointer)
{
    std::stringstream sstr;
    sstr << pointer;
    std::string address = sstr.str();
    return address;
}