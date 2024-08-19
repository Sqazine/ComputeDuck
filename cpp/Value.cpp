#include "Value.h"
#include "Object.h"

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
        return ObjectStringify(object);
    default:
        return "nil";
    }
    return "nil";
}

void Value::Mark() const
{
    if (type == ValueType::OBJECT)
        ObjectMark(object);

}
void Value::UnMark() const
{
    if (type == ValueType::OBJECT)
        ObjectUnMark(object);
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
        return left.stored == TO_NUM_VALUE(right);
    case ValueType::BOOL:
        return left.stored == TO_BOOL_VALUE(right);
    case ValueType::OBJECT:
        return IsObjectsEqual(left.object, TO_OBJECT_VALUE(right));
    default:
        return false;
    }
}

bool operator!=(const Value &left, const Value &right)
{
    return !(left == right);
}