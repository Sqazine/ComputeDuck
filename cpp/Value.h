#pragma once
#include <string>
#include "Utils.h"

#define IS_NIL_VALUE(v) (v.Type() == ValueType::NIL)
#define IS_NUM_VALUE(v) (v.Type() == ValueType::NUM)
#define IS_BOOL_VALUE(v) (v.Type() == ValueType::BOOL)
#define IS_OBJECT_VALUE(v) (v.Type() == ValueType::OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_STR_OBJ(v.object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_ARRAY_OBJ(v.object))
#define IS_STRUCT_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_STRUCT_OBJ(v.object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_REF_OBJ(v.object))
#define IS_LAMBDA_VALUE(v) (IS_OBJECT_VALUE(v)&&IS_LAMBDA_OBJ(v.object))

#define TO_NUM_VALUE(v) (v.number)
#define TO_BOOL_VALUE(v) (v.boolean)
#define TO_OBJECT_VALUE(v) (v.object)
#define TO_STR_VALUE(v) (TO_STR_OBJ(v.object))
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ(v.object))
#define TO_STRUCT_VALUE(v) (TO_STRUCT_OBJ(v.object))
#define TO_REF_VALUE(v) (TO_REF_OBJ(v.object))
#define TO_LAMBDA_VALUE(v) (TO_LAMBDA_OBJ(v.object))

enum class ValueType
{
	NIL,
	NUM,
	BOOL,
	OBJECT,
	UNKNOWN,
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
	bool IsEqualTo(const Value &other) const;
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

const Value g_UnknownValue=Value(ValueType::UNKNOWN);