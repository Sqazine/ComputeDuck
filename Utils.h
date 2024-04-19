#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <string>
#include <vector>
#ifdef _WIN32
#ifdef COMPUTE_DUCK_BUILD_DLL
#define COMPUTE_DUCK_API __declspec(dllexport)
#else
#define COMPUTE_DUCK_API __declspec(dllimport)
#endif
#else
#define COMPUTE_DUCK_API
#endif

enum class RWState
{
    READ,
    WRITE,
};

#define SAFE_DELETE(x) \
    do                 \
    {                  \
        delete x;      \
        x = nullptr;   \
    } while (false);

#define ASSERT(...)                                                                 \
    do                                                                              \
    {                                                                               \
        printf("[file:%s,function:%s,line:%d]:", __FILE__, __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__);                                                        \
        abort();                                                                    \
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