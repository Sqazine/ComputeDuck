#include "Object.h"
#include "Allocator.h"
std::string ObjectStringify(Object *object)
{
    switch (object->type)
    {
    case ObjectType::STR:
        return TO_STR_OBJ(object)->value;
    case ObjectType::ARRAY:
    {
        auto arrObj = TO_ARRAY_OBJ(object);
        std::string result = "[";
        if (arrObj->len != 0)
        {
            for (int32_t i = 0; i < arrObj->len; ++i)
                result += arrObj->elements[i].Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += "]";
        return result;
    }
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
    // ++ 新增内容
    case ObjectType::ARRAY:
    {
        if (TO_ARRAY_OBJ(left)->len != TO_ARRAY_OBJ(right)->len)
            return false;
        for (size_t i = 0; i < TO_ARRAY_OBJ(left)->len; ++i)
            if (TO_ARRAY_OBJ(left)->elements[i] != TO_ARRAY_OBJ(right)->elements[i])
                return false;
        return true;
    }
    // -- 新增内容
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
    return Allocator::GetInstance()->AllocateObject<StrObject>(newStr);
}
