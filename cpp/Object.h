#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include "Utils.h"
#include "Context.h"
#include "Value.h"

#define TO_STR_OBJ(obj) ((StrObject *)obj)
#define TO_ARRAY_OBJ(obj) ((ArrayObject *)obj)
#define TO_STRUCT_OBJ(obj) ((StructObject *)obj)
#define TO_REF_OBJ(obj) ((RefObject *)obj)
#define TO_LAMBDA_OBJ(obj) ((LambdaObject *)obj)

#define IS_STR_OBJ(obj) (obj->Type() == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->Type() == ObjectType::ARRAY)
#define IS_STRUCT_OBJ(obj) (obj->Type() == ObjectType::STRUCT)
#define IS_REF_OBJ(obj) (obj->Type() == ObjectType::REF)
#define IS_LAMBDA_OBJ(obj) (obj->Type() == ObjectType::LAMBDA)

enum class ObjectType
{

	STR,
	ARRAY,
	STRUCT,
	REF,
	LAMBDA
};

struct Object
{
	Object() : marked(false), next(nullptr) {}
	virtual ~Object() {}

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
	StrObject() {}
	StrObject(std::string_view value) : value(value) {}
	~StrObject() {}

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
	ArrayObject() {}
	ArrayObject(const std::vector<Value> &elements) : elements(elements) {}
	~ArrayObject() {}

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
			if (!elements[i].IsEqualTo(arrayOther->elements[i]))
				return false;

		return true;
	}

	std::vector<Value> elements;
};

struct RefObject : public Object
{
	RefObject(std::string_view name) : name(name) {}
	~RefObject() {}

	std::string Stringify() override { return name; }
	ObjectType Type() override { return ObjectType::REF; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_REF_OBJ(other))
			return false;
		return name == TO_REF_OBJ(other)->name;
	}

	std::string name;
};

struct LambdaObject : public Object
{
	LambdaObject(int64_t idx) : idx(idx) {}
	~LambdaObject() {}

	std::string Stringify() override { return "lambda:" + std::to_string(idx); }
	ObjectType Type() override { return ObjectType::LAMBDA; }
	void Mark() override { marked = true; }
	void UnMark() override { marked = false; }
	bool IsEqualTo(Object *other) override
	{
		if (!IS_LAMBDA_OBJ(other))
			return false;
		return idx == TO_LAMBDA_OBJ(other)->idx;
	}

	int64_t idx;
};

struct StructObject : public Object
{
	StructObject() {}
	StructObject(std::string_view name, const std::unordered_map<std::string, Value> &members) : name(name), members(members) {}
	~StructObject() {}

	std::string Stringify() override
	{
		std::string result = "struct instance " + name;
		if (!members.empty())
		{
			result += ":\n";
			for (const auto &[key, value] : members)
				result += key + "=" + value.Stringify() + "\n";
			result = result.substr(0, result.size() - 1);
		}
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

		if (name != TO_STRUCT_OBJ(other)->name)
			return false;

		for (auto [key1, value1] : members)
			for (auto [key2, value2] : TO_STRUCT_OBJ(other)->members)
				if (key1 == key2)
					if (value1.IsEqualTo(value2))
						return false;
		return true;
	}

	void AssignMember(std::string_view name, Value value)
	{
		auto iter = members.find(name.data());
		if (iter != members.end())
			members[name.data()] = value;
		else
			Assert("Undefine struct member:" + std::string(name));
	}

	Value GetMember(std::string_view name)
	{
		auto iter = members.find(name.data());
		if (iter != members.end())
			return iter->second;
		return g_UnknownValue;
	}
	std::string name;
	std::unordered_map<std::string, Value> members;
};