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

bool Value::IsValid() const
{
    return (type<=ValueType::OBJECT && type>=ValueType::NUM) && object;
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


Value FindActualValue(const Value &v)
{
    auto value = GetEndOfRefValue(v);
    if (IS_BUILTIN_VALUE(value) && TO_BUILTIN_VALUE(value)->Is<Value>())
        value = TO_BUILTIN_VALUE(value)->Get<Value>();
    return value;
}

Value *GetEndOfRefValue(Value *v)
{
    auto result = v;
    while (IS_REF_VALUE(*result))
        result = TO_REF_VALUE(*result)->pointer;
    return result;
}

Value GetEndOfRefValue(const Value &v)
{
    auto value = v;
    while (IS_REF_VALUE(value))
        value = *TO_REF_VALUE(value)->pointer;
    return value;
}