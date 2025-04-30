#pragma once
#include "Utils.h"
#include <string>

struct COMPUTEDUCK_API Value
{
    template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
        Value(T number) : stored(static_cast<double>(number)) {}
    Value() = default;
    ~Value() = default;

    std::string Stringify() const;

    double stored;
};

extern "C" COMPUTEDUCK_API void ValueAdd(const Value &l, const Value &r, Value &result);
extern "C" COMPUTEDUCK_API double ValueSub(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueMul(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueDiv(const Value &l, const Value &r);

extern "C" COMPUTEDUCK_API double ValueBitAnd(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueBitOr(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueBitXor(const Value &l, const Value &r);

extern "C" COMPUTEDUCK_API double ValueBitNot(const Value &l);
extern "C" COMPUTEDUCK_API double ValueMinus(const Value &l);