#pragma once
#include <string>
#include <vector>
#ifdef COMPUTE_DUCK_BUILD_DLL
#define COMPUTE_DUCK_API __declspec(dllexport)
#else
#define COMPUTE_DUCK_API __declspec(dllimport)
#endif

#define SAFE_DELETE(x)  do { delete x; x = nullptr; } while (false);