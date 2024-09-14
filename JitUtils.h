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
constexpr const char *SET_STACK_FN_STR  = "function_SetValueStack";

#define CREATE_STR_OBJECT_FN_NAME CreateStrObject
#define CREATE_STR_OBJECT_FN_NAME_STR STR2(STR3(CREATE_STR_OBJECT_FN_NAME))

#define CREATE_ARRAY_OBJECT_FN_NAME CreateArrayObject
#define CREATE_ARRAY_OBJECT_FN_NAME_STR STR2(STR3(CREATE_ARRAY_OBJECT_FN_NAME))

#define CREATE_REF_OBJECT_FN_NAME CreateRefObject
#define CREATE_REF_OBJECT_FN_NAME_STR STR2(STR3(CREATE_REF_OBJECT_FN_NAME))

#define CREATE_STRUCT_OBJECT_FN_NAME CreateStructObject
#define CREATE_STRUCT_OBJECT_FN_NAME_STR STR2(STR3(CREATE_STRUCT_OBJECT_FN_NAME))

#define GET_LOCAL_VARIABLE_SLOT_FN_NAME GetLocalVariableSlot
#define GET_LOCAL_VARIABLE_SLOT_FN_NAME_STR STR2(STR3(GET_LOCAL_VARIABLE_SLOT_FN_NAME))

#define CREATE_TABLE_FN_NAME CreateTable
#define CREATE_TABLE_FN_NAME_STR STR2(STR3(CREATE_TABLE_FN_NAME))

#define TABLE_SET_FN_NAME TableSet
#define TABLE_SET_FN_NAME_STR STR2(STR3(TABLE_SET_FN_NAME))

#define TABLE_SET_IF_FOUND_FN_NAME TableSetIfFound
#define TABLE_SET_IF_FOUND_FN_NAME_STR STR2(STR3(TABLE_SET_IF_FOUND_FN_NAME))

#define TABLE_GET_FN_NAME TableGet
#define TABLE_GET_FN_NAME_STR STR2(STR3(TABLE_GET_FN_NAME))

#define MALLOC_FN_NAME_STR "malloc"
#define STRCMP_FN_NAME_STR "malloc"
#define STR_ADD_FN_NAME_STR "StrAdd"
#define IS_OBJECT_EQUAL_FN_NAME_STR "IsObjectEqual"

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
    uint8_t returnType{};
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
std::string GenerateLocalVarName(int16_t scopeDepth,int16_t index,int16_t isUpValue);