#include "Value.h"

std::string Value::Stringify() const
{
    return std::to_string(stored);
}

void ValueAdd(const Value &l, const Value &r, Value &result)
{
    result = l.stored + r.stored;
}

double ValueSub(const Value &l, const Value &r)
{
    return l.stored - r.stored;
}

double ValueMul(const Value &l, const Value &r)
{
    return l.stored * r.stored;
}

double ValueDiv(const Value &l, const Value &r)
{
    return l.stored / r.stored;
}

double ValueBitAnd(const Value &l, const Value &r)
{
    return (int64_t)l.stored & (int64_t)r.stored;
}

double ValueBitOr(const Value &l, const Value &r)
{
    return (int64_t)l.stored | (int64_t)r.stored;
}

double ValueBitXor(const Value &l, const Value &r)
{
    return (int64_t)l.stored ^ (int64_t)r.stored;
}

double ValueBitNot(const Value &l)
{
    return (double)(~(int64_t)l.stored);
}

double ValueMinus(const Value &l)
{
    return (-l.stored);
}