#include "Value.h"
#include "Object.h"

Value::Value()
	: type(ValueType::NIL), object(nullptr)
{
}

Value::Value(bool boolean)
	: stored(boolean), type(ValueType::BOOL)
{
}
Value::Value(Object *object)
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
		return std::to_string(stored);
	case ValueType::BOOL:
		return stored == 1.0 ? "true" : "false";
	case ValueType::OBJECT:
		return ::Stringify(object);
	default:
		return "nil";
	}
	return "nil";
}

void Value::Mark() const
{
	if (type == ValueType::OBJECT)
		::Mark(object);
}
void Value::UnMark() const
{
	if (type == ValueType::OBJECT)
		::UnMark(object);
}

bool operator==(const Value &left, const Value &right)
{
	if (left.type != right.type)
		return false;

	switch (left.type)
	{
	case ValueType::NIL:
		return IS_NIL_VALUE(right);
	case ValueType::NUM:
	{
		return left.stored == TO_NUM_VALUE(right);
	}
	case ValueType::BOOL:
	{
		return left.stored == TO_BOOL_VALUE(right);
	}
	case ValueType::OBJECT:
	{
		return IsEqualTo(left.object,TO_OBJECT_VALUE(right));
	}
	default:
		return false;
	}
}

bool operator!=(const Value &left, const Value &right)
{
	return !(left == right);
}