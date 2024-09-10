#include "Utils.h"
#include <random>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#elif __APPLE__
#warning "Apple platform not implement yet"
#endif

std::string ReadFile(std::string_view path)
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

void WriteFile(std::string_view path,std::string_view content)
{
    std::fstream f;
	f.open(path.data(),std::ios::out);
	f<<content;
	f.close();
}

std::string PointerAddressToString(void *pointer)
{
    std::stringstream sstr;
    sstr << pointer;
    std::string address = sstr.str();
    return address;
}

void RegisterDLLs(std::string rawDllPath)
{
    using RegFn = void (*)();

    if (rawDllPath.find(".") == std::string::npos) // no file suffix
    {
#ifdef _WIN32
        rawDllPath = "./lib" + rawDllPath + ".dll";
#elif __linux__
        rawDllPath = "./lib" + rawDllPath + ".so";
#elif __APPLE__
#error "Apple platform not implement yet"
#endif
    }

#ifdef _WIN32
    HINSTANCE hInstance = GetModuleHandle(rawDllPath.c_str());
    if (!hInstance)
    {
        hInstance = LoadLibrary(rawDllPath.c_str());
        if (!hInstance)
            ASSERT("Failed to load dll library:%s", rawDllPath.c_str());

        RegFn RegisterBuiltins = (RegFn)(GetProcAddress(hInstance, "RegisterBuiltins"));

        RegisterBuiltins();
    }
#elif __linux__
    void *handle;
    double (*cosine)(double);
    char *error;

    handle = dlopen(dllpath.c_str(), RTLD_LAZY);
    if (!handle)
        ASSERT("Failed to load dll library:%s", dllpath.c_str());

    RegFn RegisterBuiltins = (RegFn)(dlsym(handle, "RegisterBuiltins"));
    RegisterBuiltins();
#elif __APPLE__
#error "Apple platform not implement yet"
#endif
}

uint32_t HashString(char *str)
{
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < strlen(str); i++)
    {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return hash;
}

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
std::string GenerateUUID()
{
    std::random_device rd;
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<uint64_t> dis;

    uint64_t part1 = dis(generator);
    uint64_t part2 = dis(generator);

    std::ostringstream oss;
    oss << std::hex << part1 << part2;
    return oss.str();
}
#endif