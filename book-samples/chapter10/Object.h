#pragma once
#include <string>
#include <cstring>
#include <string_view>
#include "Value.h"
#include "Chunk.h"

#define TO_STR_OBJ(obj) (static_cast<StrObject *>(obj))
#define TO_ARRAY_OBJ(obj) (static_cast<ArrayObject *>(obj))
// ++ 新增内容
#define TO_FUNCTION_OBJ(obj) (static_cast<FunctionObject *>(obj))
// -- 新增内容

#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->type == ObjectType::ARRAY)
// ++ 新增内容
#define IS_FUNCTION_OBJ(obj) (obj->type == ObjectType::FUNCTION)
// -- 新增内容

enum ObjectType :uint8_t
{
    STR = ValueType::OBJECT + 1,
    ARRAY,
    // ++ 新增内容
    FUNCTION,
    // -- 新增内容
};

struct Object
{
    Object(ObjectType type) : type(type) {}
    ~Object() = default;

    ObjectType type;
};

struct StrObject : public Object
{
    StrObject(char *v) :Object(ObjectType::STR), value(v), len(strlen(value)) { hash = HashString(value); }
    StrObject(const char *v) :Object(ObjectType::STR), len(strlen(v))
    {
        value = new char[len + 1];
        strcpy(value, v);
        value[len] = '\0';

        hash= HashString(value);
    }
    ~StrObject() { SAFE_DELETE_ARRAY(value); }

    char *value;
    size_t len;
    size_t hash;
};

struct ArrayObject : public Object
{
    ArrayObject(Value *eles, size_t len) :Object(ObjectType::ARRAY), elements(eles), len(len) {}
    ~ArrayObject() = default;

    Value *elements;
    size_t len;
};

// ++ 新增内容
struct FunctionObject : public Object
{
    FunctionObject(Chunk chunk, uint8_t localVarCount = 0, uint8_t parameterCount = 0)
        : Object(ObjectType::FUNCTION), chunk(chunk), localVarCount(localVarCount), parameterCount(parameterCount)
    {
    }

    ~FunctionObject() = default;

    Chunk chunk;
    uint8_t localVarCount;
    uint8_t parameterCount;
};
// -- 新增内容

COMPUTEDUCK_API std::string ObjectStringify(Object *object
// ++ 新增内容
#ifndef NDEBUG
                                            ,
                                            bool printChunkIfIsFunctionObject = false
#endif
// -- 新增内容
);

extern "C" COMPUTEDUCK_API bool IsObjectEqual(Object *left, Object *right);

extern "C" COMPUTEDUCK_API StrObject *StrAdd(StrObject *left, StrObject *right);