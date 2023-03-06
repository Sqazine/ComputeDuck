#pragma once

#ifdef COMPUTE_DUCK_BUILD_DLL
#define COMPUTE_DUCK_API __declspec(dllexport)
#else
#define COMPUTE_DUCK_API __declspec(dllimport)
#endif