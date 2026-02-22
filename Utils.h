#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string_view>
#include <string>
#include <vector>
#include <cassert>

#ifdef _WIN32
#ifdef COMPUTEDUCK_BUILD_DLL
#define COMPUTEDUCK_API __declspec(dllexport)
#else
#define COMPUTEDUCK_API __declspec(dllimport)
#endif
#else
#define COMPUTEDUCK_API
#endif

#define STR(x) #x

#define BUILTIN_FN_PREFIX_STR STR(cd_builtin_fn_)
#define BUILTIN_FN(x) cd_builtin_fn_##x

#define REGISTER_BUILTIN_VALUE(x) BuiltinManager::GetInstance()->Register(#x, Value((uint64_t)x))
#define REGISTER_BUILTIN_FN(x) BuiltinManager::GetInstance()->Register<BuiltinFn>(#x, cd_builtin_fn_##x)

#define UINT8_COUNT (UINT8_MAX + 1)
constexpr uint32_t STACK_MAX = 512;

#define SAFE_DELETE(x)   \
    do                   \
    {                    \
        if (x)           \
        {                \
            delete x;    \
            x = nullptr; \
        }                \
    } while (false);

#define SAFE_DELETE_ARRAY(x) \
    do                       \
    {                        \
        if (x)               \
        {                    \
            delete[] x;      \
            x = nullptr;     \
        }                    \
    } while (false);

#define ASSERT(...)                                                                 \
    do                                                                              \
    {                                                                               \
        printf("[file:%s,function:%s,line:%d]:", __FILE__, __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__);                                                        \
        printf("\n");                                                               \
        abort();                                                                    \
    } while (false);

COMPUTEDUCK_API std::string ReadFile(std::string_view path);
COMPUTEDUCK_API void WriteFile(std::string_view path, std::string_view content);

COMPUTEDUCK_API std::string PointerAddressToString(void *pointer);

COMPUTEDUCK_API void RegisterDLLs(std::string rawDllPath);

uint32_t HashString(char *str);
