#include "Value.h"
#include "Object.h"

Value::Value()
	: type(ValueType::NIL), object(nullptr)
{
}

Value::Value(double number)
	: number(number), type(ValueType::NUM)
{
}

Value::Value(uint64_t number)
	: number((double)number), type(ValueType::NUM)
{
}

Value::Value(bool boolean)
	: boolean(boolean), type(ValueType::BOOL)
{
}
Value::Value(Object* object)
	: object(object), type(ValueType::OBJECT)
{
}

Value::Value(ValueType type)
	: type(type), object(nullptr)
{
}
Value ::~Value()
{
}

std::string Value::Stringify() const
{
	switch (type)
	{
	case ValueType::NIL:
		return "nil";
	case ValueType::NUM:
		return std::to_string(number);
	case ValueType::BOOL:
		return boolean ? "true" : "false";
	case ValueType::OBJECT:
		return object->Stringify();
	default:
		return "nil";
	}
	return "nil";
}

void Value::Mark() const
{
	if (type == ValueType::OBJECT)
		object->Mark();
}
void Value::UnMark() const
{
	if (type == ValueType::OBJECT)
		object->UnMark();
}

bool operator==(const Value& left, const Value& right)
{
	if (left.type != right.type)
		return false;

	switch (left.type)
	{
	case ValueType::NIL:
		return IS_NIL_VALUE(right);
	case ValueType::NUM:
	{
		if (IS_NUM_VALUE(right))
			return left.number == TO_NUM_VALUE(right);
		else
			return false;
	}
	case ValueType::BOOL:
	{
		if (IS_BOOL_VALUE(right))
			return left.boolean == TO_BOOL_VALUE(right);
		return false;
	}
	case ValueType::OBJECT:
	{
		if (IS_OBJECT_VALUE(right))
			return left.object->IsEqualTo(TO_OBJECT_VALUE(right));
		return false;
	}
	default:
		return IS_NIL_VALUE(right);
	}
	return false;
}

bool operator!=(const Value& left, const Value& right)
{
	return !(left == right);
}

void gValueAdd(Value* left, Value* right, Value& result)
{
	while (IS_REF_VALUE(*left))
		left = TO_REF_VALUE(*left)->pointer;
	while (IS_REF_VALUE(*right))
		right = TO_REF_VALUE(*right)->pointer;
	//if (IS_BUILTIN_VALUE(*left) && TO_BUILTIN_VALUE(*left)->IsBuiltinData())
	//	left = TO_BUILTIN_VALUE(*left)->GetBuiltinValue();
	//if (IS_BUILTIN_VALUE(right) && TO_BUILTIN_VALUE(right)->IsBuiltinData())
	//	right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();
	if (IS_NUM_VALUE(*right) && IS_NUM_VALUE(*left))
		result = Value(TO_NUM_VALUE(*left) + TO_NUM_VALUE(*right));
	else if (IS_STR_VALUE(*right) && IS_STR_VALUE(*left))
		result = Value(new StrObject(TO_STR_VALUE(*left)->value + TO_STR_VALUE(*right)->value));
	else
		ASSERT("Invalid binary op:%s+%s", left->Stringify().c_str(), right->Stringify().c_str());
}

//  - * /
#define COMMON_BINARY(op,left,right)                                                                         \
    do                                                                                                       \
    {                                                                                                        \
        while (IS_REF_VALUE(left))                                                                           \
            left = *TO_REF_VALUE(left)->pointer;                                                             \
        while (IS_REF_VALUE(right))                                                                          \
            right = *TO_REF_VALUE(right)->pointer;                                                           \
        if (IS_BUILTIN_VALUE(left) && TO_BUILTIN_VALUE(left)->IsBuiltinData())                               \
            left = TO_BUILTIN_VALUE(left)->GetBuiltinValue();                                                \
        if (IS_BUILTIN_VALUE(right) && TO_BUILTIN_VALUE(right)->IsBuiltinData())                             \
            right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();                                              \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                       \
            result = new Value(TO_NUM_VALUE(left) op TO_NUM_VALUE(right));                                   \
        else                                                                                                 \
            ASSERT("Invalid binary op:%s%s", (left.Stringify() + (#op)).c_str(), right.Stringify().c_str()); \
    } while (0);

void gValueSub(Value* left, Value* right, Value& result)
{
	//COMMON_BINARY(-, left, right);
}

void gValueMul(Value* left, Value* right, Value& result)
{
	//COMMON_BINARY(*, left, right);
}

void gValueDiv(Value* left, Value* right, Value& result)
{
	//COMMON_BINARY(/ , left, right);
}
