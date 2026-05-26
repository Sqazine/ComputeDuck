#pragma once

#ifdef _WIN32
#ifdef COMPUTEDUCK_BUILD_DLL
#define COMPUTEDUCK_API __declspec(dllexport)
#else
#define COMPUTEDUCK_API __declspec(dllimport)
#endif
#else
#define COMPUTEDUCK_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	COMPUTEDUCK_API void RegisterBuiltins();

#ifdef __cplusplus
}
#endif