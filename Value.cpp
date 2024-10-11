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
    case ValueType::BOOL:
        return left.stored == TO_NUM_VALUE(right);
    case ValueType::OBJECT:
        return IsObjectEqual(left.object, TO_OBJECT_VALUE(right));
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

Value *GetEndOfRefValuePtr(Value *v)
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

//  - * /
#define COMMON_BINARY(l, op, r)                                                                               \
    do                                                                                                        \
    {                                                                                                         \
        auto left = FindActualValue(l);                                                                       \
        auto right = FindActualValue(r);                                                                      \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                        \
            return (TO_NUM_VALUE(left) op TO_NUM_VALUE(right));                                               \
        else                                                                                                  \
            ASSERT("Invalid binary op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

// > >= < <=
#define COMPARE_BINARY(l, op, r)                                                             \
    do                                                                                       \
    {                                                                                        \
        Value left = FindActualValue(l);                                                     \
        Value right = FindActualValue(r);                                                    \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                       \
            return (TO_NUM_VALUE(left) op TO_NUM_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                                 \
            return (false);                                                                  \
    } while (0);

// and or
#define LOGIC_BINARY(l, op, r)                                                                         \
    do                                                                                                 \
    {                                                                                                  \
        Value left = FindActualValue(l);                                                               \
        Value right = FindActualValue(r);                                                              \
        if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                               \
            return (TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false));         \
        else                                                                                           \
            ASSERT("Invalid op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

#define BIT_BINARY(l, op, r)                                                                           \
    do                                                                                                 \
    {                                                                                                  \
        Value left = FindActualValue(l);                                                               \
        Value right = FindActualValue(r);                                                              \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                 \
            return ((uint64_t)TO_NUM_VALUE(left) op(uint64_t) TO_NUM_VALUE(right));                    \
        else                                                                                           \
            ASSERT("Invalid op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

COMPUTE_DUCK_API Value ValueAdd(const Value &l, const Value &r)
{
    auto left = FindActualValue(l);
    auto right = FindActualValue(r);
    if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))
        return (TO_NUM_VALUE(left) + TO_NUM_VALUE(right));
    else if (IS_STR_VALUE(right) && IS_STR_VALUE(left))
        return (StrAdd(TO_STR_VALUE(left), TO_STR_VALUE(right)));
    else
        ASSERT("Invalid binary op:%s+%s", left.Stringify().c_str(), right.Stringify().c_str());
}

COMPUTE_DUCK_API Value ValueSub(const Value &l, const Value &r)
{
    COMMON_BINARY(l, -, r);
}

COMPUTE_DUCK_API Value ValueMul(const Value &l, const Value &r)
{
    COMMON_BINARY(l, *, r);
}

COMPUTE_DUCK_API Value ValueDiv(const Value &l, const Value &r)
{
    COMMON_BINARY(l, /, r);
}

COMPUTE_DUCK_API Value ValueGreater(const Value &l, const Value &r)
{
    COMPARE_BINARY(l, >, r);
}

COMPUTE_DUCK_API Value ValueLess(const Value &l, const Value &r)
{
    COMPARE_BINARY(l, <, r);
}

COMPUTE_DUCK_API Value ValueEqual(const Value &l, const Value &r)
{
    Value left = FindActualValue(l);
    Value right = FindActualValue(r);
    return left == right;
}

COMPUTE_DUCK_API Value ValueLogicAnd(const Value &l, const Value &r)
{
    LOGIC_BINARY(l, &&, r);
}

COMPUTE_DUCK_API Value ValueLogicOr(const Value &l, const Value &r)
{
    LOGIC_BINARY(l, ||, r);
}

COMPUTE_DUCK_API Value ValueBitAnd(const Value &l, const Value &r)
{
    BIT_BINARY(l, &, r);
}

COMPUTE_DUCK_API Value ValueBitOr(const Value &l, const Value &r)
{
    BIT_BINARY(l, &, r);
}

COMPUTE_DUCK_API Value ValueBitXor(const Value &l, const Value &r)
{
    BIT_BINARY(l, ^, r);
}

COMPUTE_DUCK_API Value ValueLogicNot(const Value &l)
{
    auto value = FindActualValue(l);
    if (!IS_BOOL_VALUE(value))
        ASSERT("Invalid op:'!' %s", value.Stringify().c_str());
    return (!TO_BOOL_VALUE(value));
}

COMPUTE_DUCK_API Value ValueBitNot(const Value &l)
{
    Value value = FindActualValue(l);
    if (!IS_NUM_VALUE(value))
        ASSERT("Invalid op:~ %s", value.Stringify().c_str());
    return (~(uint64_t)TO_NUM_VALUE(value));
}

COMPUTE_DUCK_API Value ValueMinus(const Value &l)
{
    auto value = FindActualValue(l);
    if (!IS_NUM_VALUE(value))
        ASSERT("Invalid op:'-' %s", value.Stringify().c_str());
    return (-TO_NUM_VALUE(value));
}
