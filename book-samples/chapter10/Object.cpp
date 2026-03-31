#include "Object.h"
#include "Allocator.h"
std::string ObjectStringify(Object *object
// ++ 新增内容
#ifndef NDEBUG
                            ,
                            bool printChunkIfIsFunctionObject
#endif
// -- 新增内容
)
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
    // ++ 新增内容
    case ObjectType::FUNCTION:
    {
        std::string result = "function(0x" + PointerAddressToString(object) + ")";
#ifndef NDEBUG
        if (printChunkIfIsFunctionObject)
        {
            result += ":\n";
            result += TO_FUNCTION_OBJ(object)->chunk.Stringify();
        }
#endif
        return result;
    }
    // -- 新增内容
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
    case ObjectType::FUNCTION:
    {
        if (TO_FUNCTION_OBJ(left)->chunk.opCodeList.size() != TO_FUNCTION_OBJ(right)->chunk.opCodeList.size())
            return false;
        if (TO_FUNCTION_OBJ(left)->parameterCount != TO_FUNCTION_OBJ(right)->parameterCount)
            return false;
        if (TO_FUNCTION_OBJ(left)->localVarCount != TO_FUNCTION_OBJ(right)->localVarCount)
            return false;
        for (int32_t i = 0; i < TO_FUNCTION_OBJ(left)->chunk.opCodeList.size(); ++i)
            if (TO_FUNCTION_OBJ(left)->chunk.opCodeList[i] != TO_FUNCTION_OBJ(right)->chunk.opCodeList[i])
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
