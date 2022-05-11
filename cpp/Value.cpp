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
        return "null";
    case ValueType::NUM:
        return std::to_string(number);
    case ValueType::BOOL:
        return boolean ? "true" : "false";
    case ValueType::OBJECT:
        return object->Stringify();
    default:
        return "null";
    }
    return "null";
}

bool Value::IsEqualTo(const Value &other) const
{
    switch (type)
    {
    case ValueType::NIL:
        return IS_NIL_VALUE(other);
    case ValueType::UNKNOWN:
    {
        if(other.type==ValueType::UNKNOWN)
            return true;
        return false;
    }
    case ValueType::NUM:
    {
        if (IS_NUM_VALUE(other))
            return number == TO_NUM_VALUE(other);
        else
            return false;
    }
    case ValueType::BOOL:
    {
        if (IS_BOOL_VALUE(other))
            return boolean == TO_BOOL_VALUE(other);
        return false;
    }
    case ValueType::OBJECT:
    {
        if (IS_OBJECT_VALUE(other))
            return object->IsEqualTo(TO_OBJECT_VALUE(other));
        return false;
    }
    default:
        return IS_NIL_VALUE(other);
    }
    return false;
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