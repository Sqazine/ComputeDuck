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

#define STR(x) #x

#define BUILTIN_FN_PREFIX_STR STR(cd_builtin_fn_)
#define BUILTIN_FN(name) cd_builtin_fn_##name

#define REGISTER_BUILTIN_VALUE(x) BuiltinManager::GetInstance()->Register(#x, Value((uint64_t)x))
#define REGISTER_BUILTIN_FN(x) BuiltinManager::GetInstance()->Register<BuiltinFn>(#x, BUILTIN_FN(x))

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

COMPUTE_DUCK_API std::string ReadFile(std::string_view path);
COMPUTE_DUCK_API void WriteFile(std::string_view path, std::string_view content);

COMPUTE_DUCK_API std::string PointerAddressToString(void *pointer);

COMPUTE_DUCK_API void RegisterDLLs(std::string rawDllPath);

uint32_t HashString(char *str);
