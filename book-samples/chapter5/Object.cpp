#include "Object.h"
#include "Allocator.h"
std::string ObjectStringify(Object *object)
{
    switch (object->type)
    {
    case ObjectType::STR:
        return TO_STR_OBJ(object)->value;
    default:
        ASSERT("Unknown object type");
    }
}

bool IsObjectEqual(Object *left, Object *right)
{
    if (left->type != right->type)
        return false;
    switch (left->type)
    {
    case ObjectType::STR:
        return TO_STR_OBJ(left)->hash == TO_STR_OBJ(right)->hash;
    default:
        ASSERT("Unknown object type");
        return false;
    }
}

StrObject *StrAdd(StrObject *left, StrObject *right)
{
    size_t length = left->len + right->len;
    char *newStr = new char[length + 1];
    memcpy(newStr, left->value, left->len);
    memcpy(newStr + left->len, right->value, right->len);
    newStr[length] = '\0';
    return Allocator::GetInstance()->CreateObject<StrObject>(newStr);
}
