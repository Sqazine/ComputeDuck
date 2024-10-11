#pragma once
#include <string>
#include <type_traits>
#include <cfloat>
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
#include <set>
#include <variant>
#endif
#include "Utils.h"

#define IS_NIL_VALUE(v) ((v).type == ValueType::NIL)
#define IS_NUM_VALUE(v) ((v).type == ValueType::NUM)
#define IS_BOOL_VALUE(v) ((v).type == ValueType::BOOL)
#define IS_OBJECT_VALUE(v) ((v).type == ValueType::OBJECT)
#define IS_STR_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STR_OBJ((v).object))
#define IS_ARRAY_VALUE(v) (IS_OBJECT_VALUE(v) && IS_ARRAY_OBJ((v).object))
#define IS_STRUCT(v) (IS_OBJECT_VALUE(v) && IS_STRUCT_OBJ((v).object))
#define IS_REF_VALUE(v) (IS_OBJECT_VALUE(v) && IS_REF_OBJ((v).object))
#define IS_FUNCTION_VALUE(v) (IS_OBJECT_VALUE(v) && IS_FUNCTION_OBJ((v).object))
#define IS_STRUCT_VALUE(v) (IS_OBJECT_VALUE(v) && IS_STRUCT_OBJ((v).object))
#define IS_BUILTIN_VALUE(v) (IS_OBJECT_VALUE(v) && IS_BUILTIN_OBJ((v).object))

#define TO_NUM_VALUE(v) ((v).stored)
#define TO_BOOL_VALUE(v) (((v).stored >= DBL_EPSILON) ? true : false)
#define TO_OBJECT_VALUE(v) ((v).object)
#define TO_STR_VALUE(v) (TO_STR_OBJ((v).object))
#define TO_ARRAY_VALUE(v) (TO_ARRAY_OBJ((v).object))
#define TO_STRUCT(v) (TO_STRUCT_OBJ((v).object))
#define TO_REF_VALUE(v) (TO_REF_OBJ((v).object))
#define TO_FUNCTION_VALUE(v) (TO_FUNCTION_OBJ((v).object))
#define TO_STRUCT_VALUE(v) (TO_STRUCT_OBJ((v).object))
#define TO_BUILTIN_VALUE(v) (TO_BUILTIN_OBJ((v).object))

#define TO_OBJECT_VALUE(v) ((v).object)

enum ValueType : uint8_t
{
    NIL=0,
    NUM,
    BOOL,
    OBJECT
};

struct COMPUTE_DUCK_API Value
{
    template <typename T>
        requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
    Value(T number) : stored(static_cast<double>(number)), type(ValueType::NUM) {}
    Value() : type(ValueType::NIL), object(nullptr) {}
    Value(bool boolean) : stored(boolean), type(ValueType::BOOL) {}
    Value(struct Object *object) : object(object), type(ValueType::OBJECT) {}
    ~Value() = default;

    std::string Stringify() const;

    void Mark() const;
    void UnMark() const;

    ValueType type;
    union
    {
        double stored;
        struct Object *object{ nullptr };
    };
};

bool operator==(const Value &left, const Value &right);
bool operator!=(const Value &left, const Value &right);

extern "C" COMPUTE_DUCK_API void FindActualValue(const Value &v,Value& result);
extern "C" COMPUTE_DUCK_API Value *GetEndOfRefValuePtr(Value *v);
extern "C" COMPUTE_DUCK_API void GetEndOfRefValue(const Value &v, Value &result);


extern "C" COMPUTE_DUCK_API void ValueAdd(const Value &l,const Value& r,Value& result);
extern "C" COMPUTE_DUCK_API double ValueSub(const Value &l, const Value &r);
extern "C" COMPUTE_DUCK_API double ValueMul(const Value &l, const Value &r);
extern "C" COMPUTE_DUCK_API double ValueDiv(const Value &l, const Value &r);

extern "C" COMPUTE_DUCK_API bool ValueGreater(const Value &l, const Value &r);
extern "C" COMPUTE_DUCK_API bool ValueLess(const Value &l, const Value &r);
extern "C" COMPUTE_DUCK_API bool ValueEqual(const Value &l, const Value &r);

extern "C" COMPUTE_DUCK_API bool ValueLogicAnd(const Value &l, const Value &r);
extern "C" COMPUTE_DUCK_API bool ValueLogicOr(const Value &l, const Value &r);

extern "C" COMPUTE_DUCK_API double ValueBitAnd(const Value &l, const Value &r);
extern "C" COMPUTE_DUCK_API double ValueBitOr(const Value &l, const Value &r);
extern "C" COMPUTE_DUCK_API double ValueBitXor(const Value &l, const Value &r);

extern "C" COMPUTE_DUCK_API bool ValueLogicNot(const Value &l);
extern "C" COMPUTE_DUCK_API double ValueBitNot(const Value &l);
extern "C" COMPUTE_DUCK_API double ValueMinus(const Value &l);

extern "C" COMPUTE_DUCK_API void GetArrayObjectElement(const Value& ds, const Value & index,Value& result);
