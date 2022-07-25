#include "Value.h"
#include "Object.h"

Value::Value()
	: type(ValueType::NIL)
{
}

Value::Value(double number)
	: number(number), type(ValueType::NUM)
{
}
Value::Value(bool boolean)
	: boolean(boolean), type(ValueType::BOOL)
{
}
Value::Value(Object *object)
	: object(object), type(ValueType::OBJECT)
{
}

Value::Value(ValueType type)
	: type(type)
{
}
Value ::~Value()
{
}

ValueType Value::Type() const
{
	return type;
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

bool operator==(const Value &left, const Value &right)
{
	if (left.Type() != right.Type())
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

bool operator!=(const Value &left, const Value &right)
{
	return !(left == right);
}
