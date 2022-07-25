#pragma once
#include <string>
#include "Utils.h"

#define IS_NIL_VALUE(v) (v.Type() == ValueType::NIL)
#define IS_NUM_VALUE(v) (v.Type() == ValueType::NUM)
#define IS_BOOL_VALUE(v) (v.Type() == ValueType::BOOL)
#define IS_OBJECT_VALUE(v) (v.Type() == ValueType::OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_STR_OBJ(v.object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_ARRAY_OBJ(v.object))
#define IS_STRUCT(v) (IS_OBJECT_VALUE(v)&&IS_STRUCT_OBJ(v.object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_REF_OBJ(v.object))
#define IS_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_FUNCTION_OBJ(v.object))
#define IS_BUILTIN_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_BUILTIN_OBJ(v.object))
#define IS_CLOSURE_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_CLOSURE_OBJ(v.object))
#define IS_STRUCT_INSTANCE_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_STRUCT_INSTANCE_OBJ(v.object))

#define TO_NUM_VALUE(v) (v.number)
#define TO_BOOL_VALUE(v) (v.boolean)
#define TO_OBJECT_VALUE(v) (v.object)
#define TO_STR_VALUE(v) (TO_STR_OBJ(v.object))
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ(v.object))
#define TO_STRUCT(v) (TO_STRUCT_OBJ(v.object))
#define TO_REF_VALUE(v) (TO_REF_OBJ(v.object))
#define TO_FUNCTION_VALUE(v) (TO_FUNCTION_OBJ(v.object))
#define TO_BUILTIN_VALUE(v) (TO_BUILTIN_OBJ(v.object))
#define TO_CLOSURE_VALUE(v) (TO_CLOSURE_OBJ(v.object))
#define TO_STRUCT_INSTANCE_VALUE(v) (TO_STRUCT_INSTANCE_OBJ(v.object))

enum class ValueType
{
	NIL,
	NUM,
	BOOL,
	OBJECT,
};

struct Value
{
	Value();
	Value(double number);
	Value(bool boolean);
	Value(struct Object *object);
	Value(ValueType type);
	~Value();

	ValueType Type() const;
	std::string Stringify() const;
	void Mark() const;
	void UnMark() const;
	ValueType type;

	union
	{
		double number;
		bool boolean;
		struct Object *object;
	};
};

bool operator==(const Value& left, const Value& right);
bool operator!=(const Value& left, const Value& right);