#pragma once

#ifdef _WIN32
#ifdef COMPUTE_DUCK_BUILD_DLL
#define COMPUTE_DUCK_API __declspec(dllexport)
#else
#define COMPUTE_DUCK_API __declspec(dllimport)
#endif
#else
#define COMPUTE_DUCK_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    COMPUTE_DUCK_API void RegisterBuiltins();

#ifdef __cplusplus
}
#endif