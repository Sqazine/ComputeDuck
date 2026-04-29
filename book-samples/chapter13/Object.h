#pragma once
#include <string>
#include <cstring>
#include <string_view>
#include <functional>
#include <variant>
#include "Value.h"
#include "Chunk.h"
#include "HashTable.h"

#define TO_STR_OBJ(obj) (static_cast<StrObject *>(obj))
#define TO_ARRAY_OBJ(obj) (static_cast<ArrayObject *>(obj))
#define TO_FUNCTION_OBJ(obj) (static_cast<FunctionObject *>(obj))
#define TO_BUILTIN_OBJ(obj) (static_cast<BuiltinObject *>(obj))
#define TO_UPVALUE_OBJ(obj) (static_cast<UpvalueObject *>(obj))
#define TO_CLOSURE_OBJ(obj) (static_cast<ClosureObject *>(obj))
// ++ 新增内容
#define TO_STRUCT_OBJ(obj) (static_cast<StructObject *>(obj))
// -- 新增内容

#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->type == ObjectType::ARRAY)
#define IS_FUNCTION_OBJ(obj) (obj->type == ObjectType::FUNCTION)
#define IS_BUILTIN_OBJ(obj) (obj->type == ObjectType::BUILTIN)
#define IS_UPVALUE_OBJ(obj) (obj->type == ObjectType::UPVALUE)
#define IS_CLOSURE_OBJ(obj) (obj->type == ObjectType::CLOSURE)
// ++ 新增内容
#define IS_STRUCT_OBJ(obj) (obj->type == ObjectType::STRUCT)
// -- 新增内容

enum ObjectType : uint8_t
{
    STR = ValueType::OBJECT + 1,
    ARRAY,
    FUNCTION,
    BUILTIN,
    UPVALUE,
    CLOSURE,
    // ++ 新增内容
    STRUCT,
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
    StrObject(char *v) : Object(ObjectType::STR), value(v), len(strlen(value)) { hash = HashString(value); }
    StrObject(const char *v) : Object(ObjectType::STR), len(strlen(v))
    {
        value = new char[len + 1];
        strcpy(value, v);
        value[len] = '\0';

        hash = HashString(value);
    }
    ~StrObject() { SAFE_DELETE_ARRAY(value); }

    char *value;
    size_t len;
    size_t hash;
};

struct ArrayObject : public Object
{
    ArrayObject(Value *eles, size_t len) : Object(ObjectType::ARRAY), elements(eles), len(len) {}
    ~ArrayObject() { SAFE_DELETE_ARRAY(elements); }

    Value *elements;
    size_t len;
};

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

using BuiltinFn = std::function<bool(Value *, uint8_t, Value &)>;

struct BuiltinObject : public Object
{
    BuiltinObject(const BuiltinFn &v)
        : Object(ObjectType::BUILTIN)
    {
        data = v;
    }

    ~BuiltinObject() = default;

    BuiltinFn Get()
    {
        return data;
    }

    BuiltinFn data;
};

struct UpvalueObject : public Object
{
    UpvalueObject(Value *slot) : Object(ObjectType::UPVALUE), location(slot) {}
    ~UpvalueObject() = default;

    Value *location;
    Value closed;
    UpvalueObject *nextUpvalue{nullptr};
};

struct ClosureObject : public Object
{
    ClosureObject(FunctionObject *fn) : Object(ObjectType::CLOSURE), function(fn)
    {
        memset(upvalues, 0, sizeof(UpvalueObject *) * UPVALUE_COUNT);
    }
    ~ClosureObject() = default;

    FunctionObject *function;
    UpvalueObject *upvalues[UPVALUE_COUNT]{};
};

// ++ 新增内容
struct StructObject : public Object
{
    StructObject(HashTable *membs) : Object(ObjectType::STRUCT), members(membs) {}
    ~StructObject() { SAFE_DELETE(members); }

    HashTable *members;
};
// -- 新增内容

COMPUTEDUCK_API std::string ObjectStringify(Object *object

#ifndef NDEBUG
                                            ,
                                            bool printChunkIfIsFunctionObject = false
#endif

);

extern "C" COMPUTEDUCK_API bool IsObjectEqual(Object *left, Object *right);

extern "C" COMPUTEDUCK_API StrObject *StrAdd(StrObject *left, StrObject *right);

extern "C" COMPUTEDUCK_API void StrInsert(StrObject *left, uint32_t idx, StrObject *right);
extern "C" COMPUTEDUCK_API void StrErase(StrObject *left, uint32_t idx);

extern "C" COMPUTEDUCK_API void ArrayInsert(ArrayObject *left, uint32_t idx, const Value &element);
extern "C" COMPUTEDUCK_API void ArrayErase(ArrayObject *left, uint32_t idx);