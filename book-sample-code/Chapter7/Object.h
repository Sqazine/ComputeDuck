#pragma once
#include <string>
#include <cstring>
#include <string_view>
#include "Value.h"

#define TO_STR_OBJ(obj) (static_cast<StrObject *>(obj))
#define TO_ARRAY_OBJ(obj) (static_cast<ArrayObject *>(obj))

#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)
#define IS_ARRAY_OBJ(obj) (obj->type == ObjectType::ARRAY)

enum ObjectType :uint8_t
{
    STR = ValueType::OBJECT + 1,
    ARRAY,
};

struct Object
{
    Object(ObjectType type) : type(type) {}
    ~Object() = default;

    ObjectType type;
};

struct StrObject : public Object
{
    StrObject(char *v) :Object(ObjectType::STR), value(v), len(strlen(value)) { hash = HashString(value); }
    StrObject(const char *v) :Object(ObjectType::STR), len(strlen(v))
    {
        value = new char[len + 1];
        strcpy(value, v);
        value[len] = '\0';

        hash= HashString(value);
    }
    ~StrObject() { SAFE_DELETE_ARRAY(value); }

    char *value;
    size_t len;
    size_t hash;
};

struct ArrayObject : public Object
{
    ArrayObject(Value *eles, size_t len) :Object(ObjectType::ARRAY), elements(eles), len(len) {}
    ~ArrayObject() = default;

    Value *elements;
    size_t len;
};

COMPUTEDUCK_API std::string ObjectStringify(Object *object);

extern "C" COMPUTEDUCK_API bool IsObjectEqual(Object *left, Object *right);

extern "C" COMPUTEDUCK_API StrObject *StrAdd(StrObject *left, StrObject *right);