#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include "Utils.h"
#include "Value.h"
#include "Chunk.h"

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

enum class ObjectType
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
	Object(ObjectType type)
		: marked(false), next(nullptr), type(type)
	{
	}
	virtual ~Object()
	{
	}

	virtual std::string Stringify() = 0;
	virtual void Mark() = 0;
	virtual void UnMark() = 0;
	virtual bool IsEqualTo(Object *other) = 0;

	bool marked;
	Object *next;
	ObjectType type;
};

struct StrObject : public Object
{
	StrObject(std::string_view value)
		: Object(ObjectType::STR), value(value)
	{
	}

	~StrObject() override
	{
	}

	std::string Stringify() override { return value; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_STR_OBJ(other))
			return false;
		return value == TO_STR_OBJ(other)->value;
	}

	std::string value;
};

struct ArrayObject : public Object
{
	ArrayObject(const std::vector<Value> &elements)
		: Object(ObjectType::ARRAY), elements(elements)
	{
	}

	~ArrayObject() override
	{
		std::vector<Value>().swap(elements);
	}

	std::string Stringify() override
	{
		std::string result = "[";
		if (!elements.empty())
		{
			for (const auto &e : elements)
				result += e.Stringify() + ",";
			result = result.substr(0, result.size() - 1);
		}
		result += "]";
		return result;
	}
	void Mark() override
	{
		marked = true;
		for (const auto &e : elements)
			e.Mark();
	}
	void UnMark() override
	{
		marked = false;
		for (const auto &e : elements)
			e.UnMark();
	}

	bool IsEqualTo(Object *other) override
	{
		if (!IS_ARRAY_OBJ(other))
			return false;

		ArrayObject *arrayOther = TO_ARRAY_OBJ(other);

		if (arrayOther->elements.size() != elements.size())
			return false;

		for (size_t i = 0; i < elements.size(); ++i)
			if (elements[i] != arrayOther->elements[i])
				return false;

		return true;
	}

	std::vector<Value> elements;
};

struct RefObject : public Object
{
	RefObject(Value *pointer)
		: Object(ObjectType::REF), pointer(pointer)
	{
	}
	~RefObject() override
	{
	}

	std::string Stringify() override { return pointer->Stringify(); }
	void Mark() override
	{
		marked = true;
		pointer->Mark();
	}
	void UnMark() override
	{
		marked = false;
		pointer->UnMark();
	}
	bool IsEqualTo(Object *other) override
	{
		if (!IS_REF_OBJ(other))
			return false;
		return *pointer == *TO_REF_OBJ(other)->pointer;
	}

	Value *pointer;
};

struct FunctionObject : public Object
{
	FunctionObject(const OpCodes &opCodes, int32_t localVarCount = 0, int32_t parameterCount = 0)
		: Object(ObjectType::FUNCTION), opCodes(opCodes), localVarCount(localVarCount), parameterCount(parameterCount)
	{
	}

	~FunctionObject() override
	{
		OpCodes().swap(opCodes);
	}

	std::string Stringify() override { return "function:(0x" + PointerAddressToString(this) + ")"; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_FUNCTION_OBJ(other))
			return false;

		auto otherOpCodes = TO_FUNCTION_OBJ(other)->opCodes;

		if (opCodes.size() != otherOpCodes.size())
			return false;

		for (int32_t i = 0; i < opCodes.size(); ++i)
			if (opCodes[i] != otherOpCodes[i])
				return false;
		return true;
	}

	OpCodes opCodes;
	int32_t localVarCount;
	int32_t parameterCount;
};

struct StructObject : public Object
{
	StructObject(const std::unordered_map<std::string, Value> &members)
		: Object(ObjectType::STRUCT), members(members)
	{
	}

	~StructObject() override
	{
	}

	std::string Stringify() override
	{
		std::string result = "struct instance(0x" + PointerAddressToString(this) + "):\n{\n";
		for (const auto &[k, v] : members)
			result += k + ":" + v.Stringify() + "\n";
		result = result.substr(0, result.size() - 1);
		result += "\n}\n";
		return result;
	}

	void Mark() override
	{
		marked = true;
		for (const auto &[k, v] : members)
			v.Mark();
	}
	void UnMark() override
	{
		marked = false;
		for (const auto &[k, v] : members)
			v.UnMark();
	}
	bool IsEqualTo(Object *other) override
	{
		if (!IS_STRUCT_OBJ(other))
			return false;

		for (const auto &[k1, v1] : members)
		{
			auto iter = TO_STRUCT_OBJ(other)->members.find(k1);
			if (iter == TO_STRUCT_OBJ(other)->members.end())
				return false;
		}
		return true;
	}
	std::unordered_map<std::string, Value> members;
};

using BuiltinFn = std::function<bool(Value *, uint8_t, Value &)>;

struct BuiltinObject : public Object
{
	enum class BuiltinType
	{
		FUNCTION,
		VALUE,
		RAWDATA
	};

	BuiltinObject(void *nativeData, std::function<void(void *nativeData)> destroyFunc)
		: Object(ObjectType::BUILTIN), nativeData(nativeData), destroyFunc(destroyFunc), builtinType(BuiltinType::RAWDATA)
	{
	}

	BuiltinObject(std::string_view name, const BuiltinFn &fn)
		: Object(ObjectType::BUILTIN), name(name), fn(fn), builtinType(BuiltinType::FUNCTION)
	{
	}

	BuiltinObject(std::string_view name, const Value &value)
		: Object(ObjectType::BUILTIN), name(name), value(value), builtinType(BuiltinType::VALUE)
	{
	}

	~BuiltinObject() override
	{
		if (builtinType == BuiltinType::RAWDATA)
			destroyFunc(nativeData);
	}

	std::string Stringify() override
	{

		std::string vStr;
		if (builtinType == BuiltinType::FUNCTION)
			vStr = "(0x" + PointerAddressToString(this) + ")";
		else if (builtinType == BuiltinType::RAWDATA)
			vStr = "(0x" + PointerAddressToString(nativeData) + ")";
		else
			vStr = value.Stringify();

		return "Builtin :(" + name + ":" + vStr;
	}
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_BUILTIN_OBJ(other))
			return false;

		if (builtinType == BuiltinType::RAWDATA)
			return PointerAddressToString(nativeData) == PointerAddressToString(TO_BUILTIN_OBJ(other)->nativeData);
		else
			return name == TO_BUILTIN_OBJ(other)->name;
	}

	BuiltinType builtinType;

	union
	{
		struct
		{
			void *nativeData;
			std::function<void(void *nativeData)> destroyFunc;
		};
		struct
		{
			std::string name;
			union
			{
				BuiltinFn fn;
				Value value;
			};
		};
	};
};