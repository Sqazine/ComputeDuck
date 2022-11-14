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

#define IS_STR_OBJ(obj) (obj->Type() == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->Type() == ObjectType::ARRAY)
#define IS_STRUCT_OBJ(obj) (obj->Type() == ObjectType::STRUCT)
#define IS_REF_OBJ(obj) (obj->Type() == ObjectType::REF)
#define IS_FUNCTION_OBJ(obj) (obj->Type() == ObjectType::FUNCTION)
#define IS_BUILTIN_OBJ(obj) (obj->Type() == ObjectType::BUILTIN)

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
	Object()
		: marked(false), next(nullptr)
	{
	}
	virtual ~Object()
	{
	}

	virtual std::string Stringify() = 0;
	virtual ObjectType Type() = 0;
	virtual void Mark() = 0;
	virtual void UnMark() = 0;
	virtual bool IsEqualTo(Object *other) = 0;

	bool marked;
	Object *next;
};

struct StrObject : public Object
{
	StrObject(std::string_view value)
		: value(value)
	{
	}
	~StrObject()
	{
	}

	std::string Stringify() override { return value; }
	ObjectType Type() override { return ObjectType::STR; }
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
		: elements(elements)
	{
	}
	~ArrayObject()
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
	ObjectType Type() override { return ObjectType::ARRAY; }
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
	RefObject(Value *pointer) : pointer(pointer) {}
	~RefObject() {}

	std::string Stringify() override { return pointer->Stringify(); }
	ObjectType Type() override { return ObjectType::REF; }
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
		: opCodes(opCodes), localVarCount(localVarCount), parameterCount(parameterCount)
	{
	}
	~FunctionObject()
	{
	}

	std::string Stringify() override { return "function:(0x" + PointerAddressToString(this) + ")"; }
	ObjectType Type() override { return ObjectType::FUNCTION; }
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

using BuiltinFn = std::function<bool(const std::vector<Value> &, Value &)>;

struct BuiltinObject : public Object
{
	BuiltinObject(std::string_view name, const BuiltinFn &fn)
		: name(name), fn(fn)
	{
	}
	~BuiltinObject()
	{
	}

	std::string Stringify() override { return "builtin function:(0x" + PointerAddressToString(this) + ")"; }
	ObjectType Type() override { return ObjectType::BUILTIN; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_BUILTIN_OBJ(other))
			return false;
		return name == TO_BUILTIN_OBJ(other)->name;
	}
	std::string_view name;
	BuiltinFn fn;
};

struct StructObject : public Object
{
	StructObject(const std::unordered_map<std::string, Value> &members) : members(members) {}
	~StructObject() {}

	std::string Stringify() override
	{
		std::string result = "struct instance(0x" + PointerAddressToString(this) + "):\n{\n";
		for (const auto &[k, v] : members)
			result += k + ":" + v.Stringify() + "\n";
		result = result.substr(0, result.size() - 1);
		result+="\n}\n";
		return result;
	}
	ObjectType Type() override { return ObjectType::STRUCT; }
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