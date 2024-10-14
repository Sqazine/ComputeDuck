#include "Value.h"

std::string Value::Stringify() const
{
    switch (type)
    {
    case ValueType::NUM:
        return std::to_string(stored);
    case ValueType::BOOL:
        return stored == 1.0 ? "true" : "false";
    case ValueType::NIL:
    default:
        return "nil";
    }
}

// - * /
#define COMMON_BINARY(l, op, r)                                                                        \
    do                                                                                                 \
    {                                                                                                  \
        if (IS_NUM_VALUE(r) && IS_NUM_VALUE(l))                                                        \
            return (TO_NUM_VALUE(l) op TO_NUM_VALUE(r));                                               \
        else                                                                                           \
            ASSERT("Invalid binary op:%s %s %s", l.Stringify().c_str(), (#op), r.Stringify().c_str()); \
    } while (0);

// > >= < <=
#define COMPARE_BINARY(l, op, r)                                        \
    do                                                                  \
    {                                                                   \
        if (IS_NUM_VALUE(r) && IS_NUM_VALUE(l))                         \
            return (TO_NUM_VALUE(l) op TO_NUM_VALUE(r) ? true : false); \
        else                                                            \
            return (false);                                             \
    } while (0);

// and or
#define LOGIC_BINARY(l, op, r)                                                                  \
    do                                                                                          \
    {                                                                                           \
        if (IS_BOOL_VALUE(r) && IS_BOOL_VALUE(l))                                               \
            return (TO_BOOL_VALUE(l) op TO_BOOL_VALUE(r) ? true : false);                       \
        else                                                                                    \
            ASSERT("Invalid op:%s %s %s", l.Stringify().c_str(), (#op), r.Stringify().c_str()); \
    } while (0);

#define BIT_BINARY(l, op, r)                                                                    \
    do                                                                                          \
    {                                                                                           \
        if (IS_NUM_VALUE(r) && IS_NUM_VALUE(l))                                                 \
            return (double)((uint64_t)TO_NUM_VALUE(l) op(uint64_t) TO_NUM_VALUE(r));            \
        else                                                                                    \
            ASSERT("Invalid op:%s %s %s", l.Stringify().c_str(), (#op), r.Stringify().c_str()); \
    } while (0);

void ValueAdd(const Value &l, const Value &r, Value &result)
{
    if (IS_NUM_VALUE(r) && IS_NUM_VALUE(l))
        result = (TO_NUM_VALUE(l) + TO_NUM_VALUE(r));
    else
        ASSERT("Invalid binary op:%s+%s", l.Stringify().c_str(), r.Stringify().c_str());
}

double ValueSub(const Value &l, const Value &r)
{
    COMMON_BINARY(l, -, r);
}

double ValueMul(const Value &l, const Value &r)
{
    COMMON_BINARY(l, *, r);
}

double ValueDiv(const Value &l, const Value &r)
{
    COMMON_BINARY(l, /, r);
}

bool ValueGreater(const Value &l, const Value &r)
{
    COMPARE_BINARY(l, >, r);
}

bool ValueLess(const Value &l, const Value &r)
{
    COMPARE_BINARY(l, <, r);
}

bool ValueEqual(const Value &l, const Value &r)
{
    return l.type == r.type && l.stored == r.stored;
}

bool ValueLogicAnd(const Value &l, const Value &r)
{
    LOGIC_BINARY(l, &&, r);
}

bool ValueLogicOr(const Value &l, const Value &r)
{
    LOGIC_BINARY(l, ||, r);
}

double ValueBitAnd(const Value &l, const Value &r)
{
    BIT_BINARY(l, &, r);
}

double ValueBitOr(const Value &l, const Value &r)
{
    BIT_BINARY(l, |, r);
}

double ValueBitXor(const Value &l, const Value &r)
{
    BIT_BINARY(l, ^, r);
}

bool ValueLogicNot(const Value &l)
{
    if (!IS_BOOL_VALUE(l))
        ASSERT("Invalid op:'!' %s", l.Stringify().c_str());
    return (!TO_BOOL_VALUE(l));
}

double ValueBitNot(const Value &l)
{
    if (!IS_NUM_VALUE(l))
        ASSERT("Invalid op:~ %s", l.Stringify().c_str());
    return (double)(~(uint64_t)TO_NUM_VALUE(l));
}

double ValueMinus(const Value &l)
{
    if (!IS_NUM_VALUE(l))
        ASSERT("Invalid op:'-' %s", l.Stringify().c_str());
    return (-TO_NUM_VALUE(l));
}