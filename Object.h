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
#define TO_BUILTIN_FUNCTION_OBJ(obj) ((BuiltinFunctionObject *)obj)
#define TO_BUILTIN_DATA_OBJ(obj) ((BuiltinDataObject *)obj)
#define TO_BUILTIN_VARIABLE_OBJ(obj) ((BuiltinVariableObject *)obj)

#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->type == ObjectType::ARRAY)
#define IS_STRUCT_OBJ(obj) (obj->type == ObjectType::STRUCT)
#define IS_REF_OBJ(obj) (obj->type == ObjectType::REF)
#define IS_FUNCTION_OBJ(obj) (obj->type == ObjectType::FUNCTION)
#define IS_BUILTIN_FUNCTION_OBJ(obj) (obj->type == ObjectType::BUILTIN_FUNCTION)
#define IS_BUILTIN_DATA_OBJ(obj) (obj->type == ObjectType::BUILTIN_DATA)
#define IS_BUILTIN_VARIABLE_OBJ(obj) (obj->type == ObjectType::BUILTIN_VARIABLE)

enum class ObjectType
{
	STR,
	ARRAY,
	STRUCT,
	REF,
	FUNCTION,
	BUILTIN_FUNCTION,
	BUILTIN_DATA,
	BUILTIN_VARIABLE,
};

struct Object
{
	Object(ObjectType type)
		: marked(false), next(nullptr),type(type)
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
		:Object(ObjectType::STR),  value(value)
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
	RefObject(Value *pointer) :Object(ObjectType::REF),  pointer(pointer) {}

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
	StructObject(const std::unordered_map<std::string, Value> &members) :Object(ObjectType::STRUCT),  members(members) {}

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

struct BuiltinFunctionObject : public Object
{
	BuiltinFunctionObject(std::string_view name, const BuiltinFn &fn)
		: Object(ObjectType::BUILTIN_FUNCTION), name(name), fn(fn)
	{
	}

	std::string Stringify() override { return "Builtin Function:(0x" + PointerAddressToString(this) + ")"; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_BUILTIN_FUNCTION_OBJ(other))
			return false;
		return name == TO_BUILTIN_FUNCTION_OBJ(other)->name;
	}
	std::string_view name;
	BuiltinFn fn;
};

struct BuiltinDataObject : public Object
{
	BuiltinDataObject()
	:Object(ObjectType::BUILTIN_DATA)
	{
	}

	~BuiltinDataObject()
	{
		destroyFunc(nativeData);
	}

	std::string Stringify() override { return "Builtin Data:(0x" + PointerAddressToString(nativeData) + ")"; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_BUILTIN_DATA_OBJ(other))
			return false;
		return PointerAddressToString(nativeData) == PointerAddressToString(TO_BUILTIN_DATA_OBJ(other)->nativeData);
	}
	void *nativeData;
	std::function<void(void *nativeData)> destroyFunc;
};

struct BuiltinVariableObject : public Object
{
	BuiltinVariableObject(std::string_view name, const Value &v)
		: Object(ObjectType::BUILTIN_VARIABLE), name(name), value(v)
	{
	}

	std::string Stringify() override { return "Builtin Variable:(" + name + ":" + value.Stringify() + ")"; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_BUILTIN_VARIABLE_OBJ(other))
			return false;
		return name == TO_BUILTIN_VARIABLE_OBJ(other)->name && value == TO_BUILTIN_VARIABLE_OBJ(other)->value;
	}
	std::string name;
	Value value;
};