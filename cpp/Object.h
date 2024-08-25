#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include "Utils.h"
#include "Value.h"
#include "Chunk.h"
#include <variant>
#include <type_traits>

#define TO_STR_OBJ(obj) (static_cast<StrObject *>(obj))
#define TO_ARRAY_OBJ(obj) (static_cast<ArrayObject *>(obj))
#define TO_STRUCT_OBJ(obj) (static_cast<StructObject *>(obj))
#define TO_REF_OBJ(obj) (static_cast<RefObject *>(obj))
#define TO_FUNCTION_OBJ(obj) (static_cast<FunctionObject *>(obj))
#define TO_BUILTIN_OBJ(obj) (static_cast<BuiltinObject *>(obj))

#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->type == ObjectType::ARRAY)
#define IS_STRUCT_OBJ(obj) (obj->type == ObjectType::STRUCT)
#define IS_REF_OBJ(obj) (obj->type == ObjectType::REF)
#define IS_FUNCTION_OBJ(obj) (obj->type == ObjectType::FUNCTION)
#define IS_BUILTIN_OBJ(obj) (obj->type == ObjectType::BUILTIN)

enum class ObjectType :uint8_t
{
    STR,
    ARRAY,
    STRUCT,
    REF,
    FUNCTION,
    BUILTIN,
};

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
class TypeSet
{
public:
    TypeSet() = default;
    ~TypeSet() = default;

    template <typename T>
        requires(std::is_same_v<T, ValueType> || std::is_same_v<T, ObjectType>)
    void Insert(T type)
    {
        m_ValueTypeSet.insert(type);
    }

    template<typename T>
        requires(std::is_same_v<T, ValueType> || std::is_same_v<T, ObjectType>)
    bool IsOnly(T t)
    {
        return m_ValueTypeSet.size() == 1 && m_ValueTypeSet.contains(t);
    }

    bool IsMultiplyType()
    {
        return m_ValueTypeSet.size() >= 2;
    }

    bool IsNone()
    {
        return m_ValueTypeSet.size() == 0;
    }

private:
    std::set<std::variant<ValueType, ObjectType>> m_ValueTypeSet{};
};
#endif

struct Object
{
    Object(ObjectType type) : type(type), marked(false), next(nullptr) {}
    ~Object() {}

    ObjectType type;
    bool marked;
    Object *next;
};

struct StrObject : public Object
{
    StrObject(char *v) :Object(ObjectType::STR), value(v), len(strlen(value)) {}
    StrObject(const char *v) :Object(ObjectType::STR), len(strlen(v))
    {
        value = new char[len + 1];
        strcpy(value, v);
        value[len] = '\0';
    }
    ~StrObject()
    {
        SAFE_DELETE_ARRAY(value);
    }

    char *value;
    uint32_t len;
};

struct ArrayObject : public Object
{
    ArrayObject(Value *eles, uint32_t len) :Object(ObjectType::ARRAY), elements(eles), len(len) {}
    ~ArrayObject()
    {
        // SAFE_DELETE_ARRAY(elements);
    }

    Value *elements;
    uint32_t len;
};

struct RefObject : public Object
{
    RefObject(Value *pointer) : Object(ObjectType::REF), pointer(pointer) {}
    ~RefObject() = default;

    Value *pointer;
};

struct FunctionObject : public Object
{
    FunctionObject(Chunk chunk, uint8_t localVarCount = 0, uint8_t parameterCount = 0)
        :Object(ObjectType::FUNCTION), chunk(chunk), localVarCount(localVarCount), parameterCount(parameterCount)
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        , callCount(0)
        , uuid(GenerateUUID())
#endif
    {}

    ~FunctionObject()
    {
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        SAFE_DELETE(probableReturnTypeSet);
#endif
    }

    Chunk chunk;
    uint8_t localVarCount;
    uint8_t parameterCount;

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    uint32_t callCount;
    std::set<size_t> jitCache;
    TypeSet *probableReturnTypeSet{ nullptr };//record function return types,some function with multiply return stmt may return mutiply types of value
    std::string uuid;
#endif
};

struct StructObject : public Object
{
    StructObject(const std::unordered_map<std::string, Value> &members) :Object(ObjectType::STRUCT), members(members) {}
    ~StructObject() = default;

    std::unordered_map<std::string, Value> members;
};

using BuiltinFn = std::function<bool(Value *, uint8_t, Value &)>;

struct NativeData
{
    void *nativeData{ nullptr };
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

struct BuiltinObject : public Object
{
    BuiltinObject(void *nativeData, std::function<void(void *nativeData)> destroyFunc)
        :Object(ObjectType::BUILTIN)
    {
        NativeData nd;
        nd.nativeData = nativeData;
        nd.destroyFunc = destroyFunc;
        data = nd;
    }

    template <typename T>
        requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value>)
    BuiltinObject(std::string_view name, const T &v)
        :Object(ObjectType::BUILTIN)
    {
        data = v;
    }

    ~BuiltinObject()
    {
        if (Is<NativeData>())
            Get<NativeData>().destroyFunc(Get<NativeData>().nativeData);
    }

    template<typename T>
        requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value> || std::is_same_v<T, NativeData>)
    T Get()
    {
        return std::get<T>(data);
    }

    template<typename T>
        requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value> || std::is_same_v<T, NativeData>)
    bool Is()
    {
        return std::holds_alternative<T>(data);
    }

    std::variant<NativeData, BuiltinFn, Value> data;
};

COMPUTE_DUCK_API std::string ObjectStringify(Object *object
#ifndef NDEBUG
    , bool printChunkIfIsFunctionObject = false
#endif
);

COMPUTE_DUCK_API void ObjectMark(Object *object);
COMPUTE_DUCK_API void ObjectUnMark(Object *object);

COMPUTE_DUCK_API bool IsObjectsEqual(Object *left, Object *right);

extern "C" COMPUTE_DUCK_API StrObject *StrAdd(StrObject *left, StrObject *right);
extern "C" COMPUTE_DUCK_API void StrInsert(StrObject *left, uint32_t idx, StrObject *right);
extern "C" COMPUTE_DUCK_API void StrErase(StrObject *left, uint32_t idx);

extern "C" COMPUTE_DUCK_API void ArrayInsert(ArrayObject *left, uint32_t idx, const Value &element);
extern "C" COMPUTE_DUCK_API void ArrayErase(ArrayObject *left, uint32_t idx);