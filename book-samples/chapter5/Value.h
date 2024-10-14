#pragma once
#include "Utils.h"
#include <string>

#define IS_NIL_VALUE(v) ((v).type == ValueType::NIL)
#define IS_NUM_VALUE(v) ((v).type == ValueType::NUM)
#define IS_BOOL_VALUE(v) ((v).type == ValueType::BOOL)
#define IS_OBJECT_VALUE(v) ((v).type == ValueType::OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ((v).object))

#define TO_NUM_VALUE(v) ((v).stored)
#define TO_BOOL_VALUE(v) (((v).stored >= DBL_EPSILON) ? true : false)
#define TO_OBJECT_VALUE(v) ((v).object)
#define TO_STR_VALUE(v) (TO_STR_OBJ((v).object))

enum ValueType : uint8_t
{
    NIL = 0,
    NUM,
    BOOL,
    OBJECT
};

struct COMPUTEDUCK_API Value
{
    template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
        Value(T number) : type(ValueType::NUM), stored(static_cast<double>(number)) {}
    Value() : type(ValueType::NIL) {}
    Value(bool boolean) : stored(boolean), type(ValueType::BOOL) {}
    Value(struct Object *object) : object(object), type(ValueType::OBJECT) {}
    ~Value() = default;

    std::string Stringify() const;

    ValueType type;
    union
    {
        double stored;
        struct Object *object{nullptr};
    };
};

bool operator==(const Value &left, const Value &right);
bool operator!=(const Value &left, const Value &right);

extern "C" COMPUTEDUCK_API void ValueAdd(const Value &l, const Value &r, Value &result);
extern "C" COMPUTEDUCK_API double ValueSub(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueMul(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueDiv(const Value &l, const Value &r);

extern "C" COMPUTEDUCK_API bool ValueGreater(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API bool ValueLess(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API bool ValueEqual(const Value &l, const Value &r);

extern "C" COMPUTEDUCK_API bool ValueLogicAnd(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API bool ValueLogicOr(const Value &l, const Value &r);

extern "C" COMPUTEDUCK_API double ValueBitAnd(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueBitOr(const Value &l, const Value &r);
extern "C" COMPUTEDUCK_API double ValueBitXor(const Value &l, const Value &r);

extern "C" COMPUTEDUCK_API bool ValueLogicNot(const Value &l);
extern "C" COMPUTEDUCK_API double ValueBitNot(const Value &l);
extern "C" COMPUTEDUCK_API double ValueMinus(const Value &l);