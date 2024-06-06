#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <string>
#include <vector>
#include <cassert>

#ifdef _WIN32
#ifdef COMPUTE_DUCK_BUILD_DLL
#define COMPUTE_DUCK_API __declspec(dllexport)
#else
#define COMPUTE_DUCK_API __declspec(dllimport)
#endif
#else
#define COMPUTE_DUCK_API
#endif

#define STACK_MAX 512

enum class RWState
{
    READ,
    WRITE,
};

#define SAFE_DELETE(x)   \
    do                   \
    {                    \
        if (x)           \
        {                \
            delete x;    \
            x = nullptr; \
        }                \
    } while (false);

#define ASSERT(...)                                                                 \
    do                                                                              \
    {                                                                               \
        printf("[file:%s,function:%s,line:%d]:", __FILE__, __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__);                                                        \
        abort();                                                                    \
    } while (false);

COMPUTE_DUCK_API std::string ReadFile(std::string_view path);

COMPUTE_DUCK_API std::string PointerAddressToString(void *pointer);

COMPUTE_DUCK_API void RegisterBuiltinFunctions(std::string rawDllPath);