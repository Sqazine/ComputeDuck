#pragma once
#include <string>
#include <cstring>
#include <string_view>
#include "Value.h"

#define TO_STR_OBJ(obj) (static_cast<StrObject *>(obj))
#define IS_STR_OBJ(obj) (obj->type == ObjectType::STR)

enum ObjectType :uint8_t
{
    STR = ValueType::OBJECT + 1,
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

COMPUTEDUCK_API std::string ObjectStringify(Object *object);

extern "C" COMPUTEDUCK_API bool IsObjectEqual(Object *left, Object *right);

extern "C" COMPUTEDUCK_API StrObject *StrAdd(StrObject *left, StrObject *right);