#pragma once
#include <cstdint>
#ifdef _WIN32
#ifdef COMPUTE_DUCK_BUILD_DLL
#define COMPUTE_DUCK_API __declspec(dllexport)
#else
#define COMPUTE_DUCK_API __declspec(dllimport)
#endif
#else
#define COMPUTE_DUCK_API
#endif

constexpr uint32_t STACK_MAX = 512;

#define ASSERT(...)                                                                 \
    do                                                                              \
    {                                                                               \
        printf("[file:%s,function:%s,line:%d]:", __FILE__, __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__);                                                        \
        printf("\n");                                                               \
        abort();                                                                    \
    } while (false);