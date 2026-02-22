#include "Value.h"
#include "Object.h"

std::string Value::Stringify() const
{
    switch (type)
    {
    case ValueType::NUM:
        return std::to_string(stored);
    case ValueType::BOOL:
        return stored == 1.0 ? "true" : "false";
    case ValueType::OBJECT:
        return ObjectStringify(object);
    case ValueType::NIL:
    default:
        return "nil";
    }
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

void FindActualValue(const Value &v, Value &result)
{
    GetEndOfRefValue(v, result);
    if (IS_BUILTIN_VALUE(result) && TO_BUILTIN_VALUE(result)->Is<Value>())
        result = TO_BUILTIN_VALUE(result)->Get<Value>();
}

Value *GetEndOfRefValuePtr(Value *v)
{
    auto result = v;
    while (IS_REF_VALUE(*result))
        result = TO_REF_VALUE(*result)->pointer;
    return result;
}

void GetEndOfRefValue(const Value &v, Value &result)
{
    result = v;
    while (IS_REF_VALUE(result))
        result = *TO_REF_VALUE(result)->pointer;
}

//  - * /
#define COMMON_BINARY(l, op, r)                                                                               \
    do                                                                                                        \
    {                                                                                                         \
        Value left, right;                                                                                    \
        FindActualValue(l, left);                                                                             \
        FindActualValue(r, right);                                                                            \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                        \
            return (TO_NUM_VALUE(left) op TO_NUM_VALUE(right));                                               \
        else                                                                                                  \
            ASSERT("Invalid binary op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

// > >= < <=
#define COMPARE_BINARY(l, op, r)                                               \
    do                                                                         \
    {                                                                          \
        Value left, right;                                                     \
        FindActualValue(l, left);                                              \
        FindActualValue(r, right);                                             \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                         \
            return (TO_NUM_VALUE(left) op TO_NUM_VALUE(right) ? true : false); \
        else                                                                   \
            return (false);                                                    \
    } while (0);

// and or
#define LOGIC_BINARY(l, op, r)                                                                         \
    do                                                                                                 \
    {                                                                                                  \
        Value left, right;                                                                             \
        FindActualValue(l, left);                                                                      \
        FindActualValue(r, right);                                                                     \
        if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                               \
            return (TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? true : false);                       \
        else                                                                                           \
            ASSERT("Invalid op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

#define BIT_BINARY(l, op, r)                                                                           \
    do                                                                                                 \
    {                                                                                                  \
        Value left, right;                                                                             \
        FindActualValue(l, left);                                                                      \
        FindActualValue(r, right);                                                                     \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                 \
            return ((uint64_t)TO_NUM_VALUE(left) op(uint64_t) TO_NUM_VALUE(right));                    \
        else                                                                                           \
            ASSERT("Invalid op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

COMPUTEDUCK_API void ValueAdd(const Value &l, const Value &r, Value &result)
{
    Value left, right;
    FindActualValue(l, left);
    FindActualValue(r, right);
    if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))
        result = (TO_NUM_VALUE(left) + TO_NUM_VALUE(right));
    else if (IS_STR_VALUE(right) && IS_STR_VALUE(left))
        result = (StrAdd(TO_STR_VALUE(left), TO_STR_VALUE(right)));
    else
        ASSERT("Invalid binary op:%s+%s", left.Stringify().c_str(), right.Stringify().c_str());
}

COMPUTEDUCK_API double ValueSub(const Value &l, const Value &r)
{
    COMMON_BINARY(l, -, r);
}

COMPUTEDUCK_API double ValueMul(const Value &l, const Value &r)
{
    COMMON_BINARY(l, *, r);
}

COMPUTEDUCK_API double ValueDiv(const Value &l, const Value &r)
{
    COMMON_BINARY(l, /, r);
}

COMPUTEDUCK_API bool ValueGreater(const Value &l, const Value &r)
{
    COMPARE_BINARY(l, >, r);
}

COMPUTEDUCK_API bool ValueLess(const Value &l, const Value &r)
{
    COMPARE_BINARY(l, <, r);
}

COMPUTEDUCK_API bool ValueEqual(const Value &l, const Value &r)
{
    Value left, right;
    FindActualValue(l, left);
    FindActualValue(r, right);
    return left == right;
}

COMPUTEDUCK_API bool ValueLogicAnd(const Value &l, const Value &r)
{
    LOGIC_BINARY(l, &&, r);
}

COMPUTEDUCK_API bool ValueLogicOr(const Value &l, const Value &r)
{
    LOGIC_BINARY(l, ||, r);
}

COMPUTEDUCK_API double ValueBitAnd(const Value &l, const Value &r)
{
    BIT_BINARY(l, &, r);
}

COMPUTEDUCK_API double ValueBitOr(const Value &l, const Value &r)
{
    BIT_BINARY(l, |, r);
}

COMPUTEDUCK_API double ValueBitXor(const Value &l, const Value &r)
{
    BIT_BINARY(l, ^, r);
}

COMPUTEDUCK_API bool ValueLogicNot(const Value &l)
{
    Value value;
    FindActualValue(l, value);
    if (!IS_BOOL_VALUE(value))
        ASSERT("Invalid op:'!' %s", value.Stringify().c_str());
    return (!TO_BOOL_VALUE(value));
}

COMPUTEDUCK_API double ValueBitNot(const Value &l)
{
    Value value;
    FindActualValue(l, value);
    if (!IS_NUM_VALUE(value))
        ASSERT("Invalid op:~ %s", value.Stringify().c_str());
    return (~(uint64_t)TO_NUM_VALUE(value));
}

COMPUTEDUCK_API double ValueMinus(const Value &l)
{
    Value value;
    FindActualValue(l, value);
    if (!IS_NUM_VALUE(value))
        ASSERT("Invalid op:'-' %s", value.Stringify().c_str());
    return (-TO_NUM_VALUE(value));
}

COMPUTEDUCK_API void GetArrayObjectElement(const Value &ds, const Value &index, Value &result)
{
    if (IS_ARRAY_VALUE(ds) && IS_NUM_VALUE(index))
    {
        auto array = TO_ARRAY_VALUE(ds);
        auto i = (size_t)TO_NUM_VALUE(index);
        if (!(i < 0 || i >= array->len))
            result = array->elements[i];
    }
    else
        ASSERT("Invalid index op: %s[%s]", ds.Stringify().c_str(), index.Stringify().c_str());
}
