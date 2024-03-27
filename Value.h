#pragma once
#include <string>
#include "Utils.h"
#include "Config.h"

#define IS_NIL_VALUE(v) ((v).type == ValueType::NIL)
#define IS_NUM_VALUE(v) ((v).type == ValueType::NUM)
#define IS_BOOL_VALUE(v) ((v).type == ValueType::BOOL)
#define IS_OBJECT_VALUE(v) ((v).type == ValueType::OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ((v).object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ((v).object))
#define IS_STRUCT(v) (IS_OBJECT_VALUE(v) && IS_STRUCT_OBJ((v).object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v) && IS_REF_OBJ((v).object))
#define IS_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v) && IS_FUNCTION_OBJ((v).object))
#define IS_STRUCT_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STRUCT_OBJ((v).object))
#define IS_BUILTIN_VALUE(v) (IS_OBJECT_VALUE(v) && IS_BUILTIN_OBJ((v).object))

#define TO_NUM_VALUE(v) ((v).number)
#define TO_BOOL_VALUE(v) ((v).boolean)
#define TO_OBJECT_VALUE(v) ((v).object)
#define TO_STR_VALUE(v) (TO_STR_OBJ((v).object))
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ((v).object))
#define TO_STRUCT(v) (TO_STRUCT_OBJ((v).object))
#define TO_REF_VALUE(v) (TO_REF_OBJ((v).object))
#define TO_FUNCTION_VALUE(v) (TO_FUNCTION_OBJ((v).object))
#define TO_STRUCT_VALUE(v) (TO_STRUCT_OBJ((v).object))
#define TO_BUILTIN_VALUE(v) (TO_BUILTIN_OBJ((v).object))

enum class ValueType:uint8_t
{
	NIL,
	NUM,
	BOOL,
	OBJECT,
};

extern "C" struct COMPUTE_DUCK_API Value
{
	Value();
	Value(double number);
	Value(uint64_t number);
	Value(bool boolean);
	Value(struct Object* object);
	Value(ValueType type);
	~Value();

	std::string Stringify() const;
	void Mark() const;
	void UnMark() const;

	ValueType type;

	union
	{
		double number;
		bool boolean;
		struct Object* object{ nullptr };
	};
};

bool operator==(const Value& left, const Value& right);
bool operator!=(const Value& left, const Value& right);

extern "C" COMPUTE_DUCK_API void gValueAdd(Value* left, Value* right,Value& result);
extern "C" COMPUTE_DUCK_API void gValueSub(Value* left, Value* right,Value& result);
extern "C" COMPUTE_DUCK_API void gValueMul(Value* left, Value* right,Value& result);
extern "C" COMPUTE_DUCK_API void gValueDiv(Value * left, Value * right, Value& result);