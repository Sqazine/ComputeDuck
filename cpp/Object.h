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
#ifdef BUILD_WITH_LLVM
#include <llvm/IR/Function.h>
#endif

#define TO_STR_OBJ(obj) ((StrObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_STRUCT_OBJ(obj) ((StructObject *)obj)
#define TO_REF_OBJ(obj) ((RefObject *)obj)
#define TO_FUNCTION_OBJ(obj) ((FunctionObject *)obj)
#define TO_BUILTIN_OBJ(obj) ((BuiltinObject *)obj)

#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->type == ObjectType::ARRAY)
#define IS_STRUCT_OBJ(obj) (obj->type == ObjectType::STRUCT)
#define IS_REF_OBJ(obj) (obj->type == ObjectType::REF)
#define IS_FUNCTION_OBJ(obj) (obj->type == ObjectType::FUNCTION)
#define IS_BUILTIN_OBJ(obj) (obj->type == ObjectType::BUILTIN)

enum class ObjectType:uint8_t
{
	STR,
	ARRAY,
	STRUCT,
	REF,
	FUNCTION,
	BUILTIN,
};

struct Object
{
	Object(ObjectType type): marked(false), next(nullptr), type(type){}
	~Object() {}

	ObjectType type;
	bool marked;
    Object* next;
};

struct StrObject : public Object
{
	StrObject(char* v): Object(ObjectType::STR), value(v),len(strlen(value)){}
	StrObject(const char* v): Object(ObjectType::STR),len(strlen(v))
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
	ArrayObject(Value* eles,uint32_t len): Object(ObjectType::ARRAY), elements(eles), len(len),capacity(len) {}
    ~ArrayObject(){}

	Value* elements;
    uint32_t len;
    uint32_t capacity;
};

struct RefObject : public Object
{
	RefObject(Value *pointer)	: Object(ObjectType::REF), pointer(pointer){}
	~RefObject(){}

	Value *pointer;
};

struct FunctionObject : public Object
{
	FunctionObject(Chunk chunk, uint8_t localVarCount = 0, uint8_t parameterCount = 0)
		: Object(ObjectType::FUNCTION), chunk(chunk), localVarCount(localVarCount), parameterCount(parameterCount)
#ifdef BUILD_WITH_LLVM
        ,callCount(0)
#endif
    {}
	~FunctionObject(){}

	Chunk chunk;
	uint8_t localVarCount;
	uint8_t parameterCount;
#ifdef BUILD_WITH_LLVM
    uint32_t callCount;
    std::unordered_map<uint32_t, std::string> m_FnJitCache;
#endif
};

struct StructObject : public Object
{
	StructObject(const std::unordered_map<std::string, Value> &members)	: Object(ObjectType::STRUCT), members(members){}
	~StructObject() {}

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
		: Object(ObjectType::BUILTIN)
	{
		NativeData nd;
		nd.nativeData = nativeData;
		nd.destroyFunc = destroyFunc;
		data = nd;
	}

	template <typename T>
		requires(std::is_same_v<T, BuiltinFn> || std::is_same_v<T, Value>)
	BuiltinObject(std::string_view name, const T &v)
		: Object(ObjectType::BUILTIN)
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

inline std::string Stringify(Object* object
#ifndef NDEBUG
    ,bool printChunkIfIsFunctionObject=false
#endif
)
{
	switch (object->type)
	{
	case ObjectType::STR:
	{
        auto strObj = TO_STR_OBJ(object);
		return TO_STR_OBJ(object)->value;
	}
	case ObjectType::ARRAY:
    {
        auto arrObj = TO_ARRAY_OBJ(object);
        std::string result = "[";
        if (arrObj->len!=0)
        {
            for(int32_t i=0;i< arrObj->len;++i)
                result += arrObj->elements[i].Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += "]";
        return result;
    }
	case ObjectType::STRUCT:
	{
        std::string result = "struct instance(0x" + PointerAddressToString(object) + "):\n{\n";
        for (const auto& [k, v] : TO_STRUCT_OBJ(object)->members)
            result += k + ":" + v.Stringify() + "\n";
        result = result.substr(0, result.size() - 1);
        result += "\n}\n";
        return result;
	}
    case ObjectType::REF:
    {
        return TO_REF_OBJ(object)->pointer->Stringify();
    }
    case ObjectType::FUNCTION:
    {
        std::string result= "function(0x" + PointerAddressToString(object) + ")";
#ifndef NDEBUG
        if (printChunkIfIsFunctionObject) 
        {
            result += ":\n";
            result += TO_FUNCTION_OBJ(object)->chunk.Stringify();
        }
#endif
        return result;
    }
    case ObjectType::BUILTIN:
    {
        std::string vStr;
        if (TO_BUILTIN_OBJ(object)->Is<BuiltinFn>())
            vStr = "(0x" + PointerAddressToString(object) + ")";
        else if (TO_BUILTIN_OBJ(object)->Is<NativeData>())
            vStr = "(0x" + PointerAddressToString(TO_BUILTIN_OBJ(object)->Get<NativeData>().nativeData) + ")";
        else
            vStr = TO_BUILTIN_OBJ(object)->Get<Value>().Stringify();

        return "Builtin :" + vStr;
    }
	default:
		ASSERT("Unknown object type");
		return nullptr;
	}
}

inline void Mark(Object* object)
{
	object->marked = true;
    switch (object->type)
    {
    case ObjectType::STR:
    {
		break;
    }
    case ObjectType::ARRAY:
    {
        for (int32_t i = 0; i < TO_ARRAY_OBJ(object)->capacity; ++i)
            TO_ARRAY_OBJ(object)->elements[i].Mark();
        break;
    }
    case ObjectType::STRUCT:
    {
        for (const auto& [k, v] : TO_STRUCT_OBJ(object)->members)
            v.Mark();
		break;
    }
    case ObjectType::REF:
    {
		TO_REF_OBJ(object)->pointer->Mark();
        break;
    }
    case ObjectType::FUNCTION:
    {
        for (const auto& v : TO_FUNCTION_OBJ(object)->chunk.constants)
            v.Mark();
        break;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(object)->Is<Value>())
            TO_BUILTIN_OBJ(object)->Get<Value>().Mark();
        break;
    }
    default:
		break;
    }
}

inline void UnMark(Object* object)
{
	object->marked = false;
    switch (object->type)
    {
    case ObjectType::STR:
    {
        break;
    }
    case ObjectType::ARRAY:
    {
        for (int32_t i=0;i<TO_ARRAY_OBJ(object)->capacity;++i)
            TO_ARRAY_OBJ(object)->elements[i].UnMark();
        break;
    }
    case ObjectType::STRUCT:
    {
        for (const auto& [k, v] : TO_STRUCT_OBJ(object)->members)
            v.UnMark();
        break;
    }
    case ObjectType::REF:
    {
		TO_REF_OBJ(object)->pointer->UnMark();
        break;
    }
    case ObjectType::FUNCTION:
    {
        for (const auto& v : TO_FUNCTION_OBJ(object)->chunk.constants)
            v.UnMark();
        break;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(object)->Is<Value>())
            TO_BUILTIN_OBJ(object)->Get<Value>().UnMark();
        break;
    }
    default:
        break;
    }
}

inline bool IsEqualTo(Object* left, Object* right)
{
	if (left->type != right->type)
		return false;
    switch (left->type)
    {
    case ObjectType::STR:
    {
		return TO_STR_OBJ(left)->value == TO_STR_OBJ(right)->value;
    }
    case ObjectType::ARRAY:
    {
        if (TO_ARRAY_OBJ(left)->len != TO_ARRAY_OBJ(right)->len)
            return false;
        for (size_t i = 0; i < TO_ARRAY_OBJ(left)->len; ++i)
            if (TO_ARRAY_OBJ(left)->elements[i] != TO_ARRAY_OBJ(right)->elements[i])
                return false;
        return true;
    }
    case ObjectType::STRUCT:
    {
        for (const auto& [k1, v1] : TO_STRUCT_OBJ(left)->members)
        {
            auto iter = TO_STRUCT_OBJ(right)->members.find(k1);
            if (iter == TO_STRUCT_OBJ(right)->members.end())
                return false;
        }
        return true;
    }
    case ObjectType::REF:
    {
		return *TO_REF_OBJ(left)->pointer == *TO_REF_OBJ(right)->pointer;
    }
    case ObjectType::FUNCTION:
    {
        if (TO_FUNCTION_OBJ(left)-> chunk.opCodes.size() != TO_FUNCTION_OBJ(right)->chunk.opCodes.size())
            return false;
        if (TO_FUNCTION_OBJ(left)->parameterCount != TO_FUNCTION_OBJ(right)->parameterCount)
            return false;
        if (TO_FUNCTION_OBJ(left)->localVarCount != TO_FUNCTION_OBJ(right)->localVarCount)
            return false;
        for (int32_t i = 0; i < TO_FUNCTION_OBJ(left)->chunk.opCodes.size(); ++i)
            if (TO_FUNCTION_OBJ(left)->chunk.opCodes[i] != TO_FUNCTION_OBJ(right)->chunk.opCodes[i])
                return false;
        return true;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(left)->Is<NativeData>())
        {
            auto thisNd = TO_BUILTIN_OBJ(left)->Get<NativeData>();
            auto otherNd = TO_BUILTIN_OBJ(right)->Get<NativeData>();
            return PointerAddressToString(thisNd.nativeData) == PointerAddressToString(otherNd.nativeData);
        }
        else
            return TO_BUILTIN_OBJ(left)->data.index() == TO_BUILTIN_OBJ(right)->data.index();
    }
    default:
        ASSERT("Unknown object type");
        return false;
    }
}

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