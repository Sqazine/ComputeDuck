#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <type_traits>
#include <set>

#ifndef NDEBUG
#define ERROR(newState, ...)                                                        \
    do                                                                              \
    {                                                                               \
        printf("[file:%s,function:%s,line:%d]:", __FILE__, __FUNCTION__, __LINE__); \
        printf(__VA_ARGS__);                                                        \
        printf("\n");                                                               \
        m_Module->getFunctionList().pop_back();                                     \
        jitFnDecl.state = newState;                                                 \
        return jitFnDecl;                                                           \
    } while (false);
#else
#define ERROR(...) return false;
#endif

std::string GenerateUUID();

constexpr uint32_t JIT_TRIGGER_COUNT = 2;

constexpr const char *GLOBAL_VARIABLE_STR = "m_GlobalVariables";
constexpr const char *SET_GLOBAL_VARIABLE_FN_STR = "function_SetGlobalVariables";

constexpr const char *STACK_STR = "m_ValueStack";
constexpr const char *SET_STACK_FN_STR = "function_SetValueStack";

enum JumpMode
{
    IF = 0,
    WHILE = 1,
};

enum class JitCompileState
{
    SUCCESS,
    FAIL,
    DEPEND,
};

struct JitFnDecl
{
    JitFnDecl() = default;
    ~JitFnDecl() = default;
    uint8_t returnType{ 0 };
    std::vector<uint8_t> paramTypes{};
    JitCompileState state{ JitCompileState::SUCCESS };
};

class TypeSet
{
public:
    TypeSet() = default;
    ~TypeSet() = default;

    void Insert(uint8_t type);
    void Insert(const TypeSet *other);
    bool IsOnly(uint8_t t);
    uint8_t GetOnly();
    bool IsMultiplyType();
    bool IsNone();
    size_t Hash();

private:
    std::set<uint8_t> m_ValueTypeSet{};
};

struct Value;

size_t HashValueList(Value *start, Value *end);
std::string GenerateFunctionName(const std::string &uuid, size_t returnHash, size_t paramHash);
std::string GenerateLocalVarName(int16_t scopeDepth, int16_t index, int16_t isUpValue);