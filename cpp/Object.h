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
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
#include <set>
#endif

struct Object
{
	Object(): marked(false), next(nullptr){}
	~Object() {}

	bool marked;
    Object* next;
};

struct StrObject : public Object
{
	StrObject(char* v): value(v),len(strlen(value)){}
	StrObject(const char* v): len(strlen(v))
    {
        value = new char[len+1];
        strcpy(value, v);
        value[len] = '\0';
    }
	~StrObject()
    {
       SAFE_DELETE_ARRAY(value);
    }

	char* value;
    uint32_t len;
};

struct ArrayObject : public Object
{
	ArrayObject(Value* eles,uint32_t len): elements(eles), len(len),capacity(len) {}
    ~ArrayObject() = default;

	Value* elements;
    uint32_t len;
    uint32_t capacity;
};

struct RefObject : public Object
{
	RefObject(Value *pointer): pointer(pointer){}
    ~RefObject() = default;

	Value *pointer;
};

struct FunctionObject : public Object
{
	FunctionObject(Chunk chunk, uint8_t localVarCount = 0, uint8_t parameterCount = 0)
		: chunk(chunk), localVarCount(localVarCount), parameterCount(parameterCount)
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        , callCount(0)
        , uuid(GenerateUUID())
#endif
    {}

    ~FunctionObject() = default;

	Chunk chunk;
	uint8_t localVarCount;
	uint8_t parameterCount;

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    uint32_t callCount;
    std::set<size_t> jitCache;
    std::set<ValueType> probableReturnTypes;//record function return types,some function with multiply return stmt may return mutiply types of value
    std::string uuid;
#endif
};

struct StructObject : public Object
{
	StructObject(const std::unordered_map<std::string, Value> &members)	: members(members){}
    ~StructObject() = default;

	std::unordered_map<std::string, Value> members;
};

using BuiltinFn = std::function<bool(Value *, uint8_t, Value &)>;

struct NativeData
{
    void* nativeData{ nullptr };
    std::function<void(void* nativeData)> destroyFunc;

    template <typename T>
    T* As()
    {
        return (T*)nativeData;
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
	{
		NativeData nd;
		nd.nativeData = nativeData;
		nd.destroyFunc = destroyFunc;
		data = nd;
	}

	template <typename T>
		requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value>)
	BuiltinObject(std::string_view name, const T &v)
	{
		data = v;
	}

	~BuiltinObject()
	{
		if (Is<NativeData>())
			Get<NativeData>().destroyFunc(Get<NativeData>().nativeData);
	}

	template<typename T>
		requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value> ||std::is_same_v<T,NativeData>)
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

COMPUTE_DUCK_API inline StrObject* StrAdd(StrObject* left, StrObject* right)
{
    int length = left->len + right->len;
    char* newStr = new char[length + 1];
    memcpy(newStr, left->value, left->len);
    memcpy(newStr + left->len, right->value, right->len);
    newStr[length] = '\0';
    return new StrObject(newStr);
}

COMPUTE_DUCK_API inline void StrInsert(StrObject* left,uint32_t idx,StrObject* right)
{
    int length = left->len + right->len;
    char* newStr = new char[length+1];
    memset(newStr, '\0', sizeof(newStr));
    strncpy(newStr, left->value, idx);
    newStr = strcat(newStr, right->value);
    newStr=strcat(newStr,left->value+idx);
  
    left->value = newStr;
    left->len = length;
}

COMPUTE_DUCK_API inline void StrErase(StrObject* left, uint32_t idx)
{
    int32_t i=0, j=0;
    for (; left->value[i] != '\0'; ++i)
        if (i != idx)
            left->value[j++] = left->value[i];

    left->value[j] = '\0';
    left->len--;
}

COMPUTE_DUCK_API inline void ArrayInsert(ArrayObject* left, uint32_t idx, const Value& element)
{
    //TODO:not implement yet
}

COMPUTE_DUCK_API inline void ArrayErase(ArrayObject* left, uint32_t idx)
{
    int32_t i = 0, j = 0;
    for (;i<left->len;++i)
        if (i != idx)
            left->elements[j++] = left->elements[i];

    left->len--;
}