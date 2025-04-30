#include "Value.h"

std::string Value::Stringify() const
{
    return std::to_string(stored);
}

void ValueAdd(const Value &l, const Value &r, Value &result)
{
    result.stored = l.stored + r.stored;
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