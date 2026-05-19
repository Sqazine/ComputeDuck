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
#define TO_STRUCT_OBJ(obj) (static_cast<StructObject *>(obj))
#define TO_REF_OBJ(obj) (static_cast<RefObject *>(obj))

#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->type == ObjectType::ARRAY)
#define IS_FUNCTION_OBJ(obj) (obj->type == ObjectType::FUNCTION)
#define IS_BUILTIN_OBJ(obj) (obj->type == ObjectType::BUILTIN)
#define IS_UPVALUE_OBJ(obj) (obj->type == ObjectType::UPVALUE)
#define IS_CLOSURE_OBJ(obj) (obj->type == ObjectType::CLOSURE)
#define IS_STRUCT_OBJ(obj) (obj->type == ObjectType::STRUCT)
#define IS_REF_OBJ(obj) (obj->type == ObjectType::REF)

enum ObjectType : uint8_t
{
    STR = ValueType::OBJECT + 1,
    ARRAY,
    FUNCTION,
    BUILTIN,
    UPVALUE,
    CLOSURE,
    STRUCT,
    REF,
};

struct Object
{
    Object(ObjectType type) : type(type)
    , marked(false), next(nullptr)
    {}
    ~Object() = default;

    ObjectType type;
    bool marked;
    Object *next;
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

// ++ 新增内容
struct NativeData
{
    void *nativeData{nullptr};
    std::function<void(void *nativeData)> destroyFunc;

    template <typename T>
    T *As()
    {
        return (T *)nativeData;
    }

    template <typename T>
    bool IsSameTypeAs()
    {
        return std::is_same<T, typename std::remove_reference<decltype(nativeData)>::type>::value == 1;
    }
};
// -- 新增内容

// ++ 修改内容
// struct BuiltinObject : public Object
// {
//     BuiltinObject(const BuiltinFn &v)
//         : Object(ObjectType::BUILTIN)
//     {
//         data = v;
//     }

//     ~BuiltinObject() = default;

//     BuiltinFn Get()
//     {
//         return data;
//     }

//     BuiltinFn data;
// };
struct BuiltinObject : public Object
{
    BuiltinObject(void *nativeData, std::function<void(void *nativeData)> destroyFunc)
        : Object(ObjectType::BUILTIN)
    {
        NativeData nd;
        nd.nativeData = nativeData;
        nd.destroyFunc = destroyFunc;
        data = nd;
    }

    template <typename T>
    requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value>)
        BuiltinObject(const T &v)
        : Object(ObjectType::BUILTIN)
    {
        data = v;
    }

    ~BuiltinObject()
    {
        if (Is<NativeData>())
            Get<NativeData>().destroyFunc(Get<NativeData>().nativeData);
    }

    template <typename T>
    requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value> || std::is_same_v<T, NativeData>)
        T Get()
    {
        return std::get<T>(data);
    }

    template <typename T>
    requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value> || std::is_same_v<T, NativeData>) bool Is()
    {
        return std::holds_alternative<T>(data);
    }

    std::variant<NativeData, BuiltinFn, Value> data;
};
// -- 修改内容

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

struct StructObject : public Object
{
    StructObject(HashTable *membs) : Object(ObjectType::STRUCT), members(membs) {}
    ~StructObject() { SAFE_DELETE(members); }

    HashTable *members;
};

struct RefObject : public Object
{
    RefObject(Value *pointer) : Object(ObjectType::REF), pointer(pointer) {}
    ~RefObject() = default;

    Value *pointer;
};

COMPUTEDUCK_API std::string ObjectStringify(Object *object

#ifndef NDEBUG
                                            ,
                                            bool printChunkIfIsFunctionObject = false
#endif

);

COMPUTEDUCK_API void MarkObject(Object *object);
COMPUTEDUCK_API void UnMarkObject(Object *object);
COMPUTEDUCK_API void DeleteObject(Object *object);

extern "C" COMPUTEDUCK_API bool IsObjectEqual(Object *left, Object *right);

extern "C" COMPUTEDUCK_API StrObject *StrAdd(StrObject *left, StrObject *right);

extern "C" COMPUTEDUCK_API void StrInsert(StrObject *left, uint32_t idx, StrObject *right);
extern "C" COMPUTEDUCK_API void StrErase(StrObject *left, uint32_t idx);

extern "C" COMPUTEDUCK_API void ArrayInsert(ArrayObject *left, uint32_t idx, const Value &element);
extern "C" COMPUTEDUCK_API void ArrayErase(ArrayObject *left, uint32_t idx);