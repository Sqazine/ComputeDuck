#include "Object.h"
#include "Allocator.h"
std::string ObjectStringify(Object *object

#ifndef NDEBUG
                            ,
                            bool printChunkIfIsFunctionObject
#endif

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
    case ObjectType::UPVALUE:
        return TO_UPVALUE_OBJ(object)->location->Stringify();
    case ObjectType::CLOSURE:
    {
        return ObjectStringify(TO_CLOSURE_OBJ(object)->function
#ifndef NDEBUG
                               ,
                               printChunkIfIsFunctionObject
#endif
        );
    }
    case ObjectType::BUILTIN:
    {
        std::string vStr = "(0x" + PointerAddressToString(object) + ")";
        return "Builtin :" + vStr;
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
    case ObjectType::BUILTIN:
    {
        auto leftFn = TO_BUILTIN_OBJ(left)->Get();
        auto rightFn = TO_BUILTIN_OBJ(right)->Get();
       return leftFn.target<BuiltinFn>() == rightFn.target<BuiltinFn>();
    }
    case ObjectType::UPVALUE:
    {
        return TO_UPVALUE_OBJ(left)->location == TO_UPVALUE_OBJ(right)->location;
    }
    case ObjectType::CLOSURE:
    {
        auto leftClosure = TO_CLOSURE_OBJ(left);
        auto rightClosure = TO_CLOSURE_OBJ(right);
        if (!IsObjectEqual(leftClosure->function, rightClosure->function))
            return false;
        for (size_t i = 0; i < UPVALUE_COUNT; ++i)
        {
            auto upvalue1 = leftClosure->upvalues[i];
            auto upvalue2 = rightClosure->upvalues[i];
            if (upvalue1 != upvalue2)
                return false;
        }
        return true;
    }
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

void StrInsert(StrObject *left, uint32_t idx, StrObject *right)
{
    size_t length = left->len + right->len;
    char *newStr = new char[length + 1];
    memset(newStr, '\0', length + 1);
    strncpy(newStr, left->value, idx);
    newStr = strcat(newStr, right->value);
    newStr = strcat(newStr, left->value + idx);

    left->value = newStr;
    left->len = length;
}

void StrErase(StrObject *left, uint32_t idx)
{
    int32_t i = 0, j = 0;
    for (; left->value[i] != '\0'; ++i)
        if (i != idx)
            left->value[j++] = left->value[i];

    left->value[j] = '\0';
    left->len--;
}

void ArrayInsert(ArrayObject *left, uint32_t idx, const Value &element)
{
     Value *newElements = new Value[left->len + 1];
    for (uint32_t i = 0; i < idx; ++i)
        newElements[i] = left->elements[i];
    newElements[idx] = element;
    for (uint32_t i = idx+1; i < left->len; ++i)
        newElements[i] = left->elements[i];
    
    SAFE_DELETE_ARRAY(left->elements);

    left->elements = newElements;
    left->len += 1;
}

void ArrayErase(ArrayObject *left, uint32_t idx)
{
    int32_t i = 0, j = 0;
    for (; i < left->len; ++i)
        if (i != idx)
            left->elements[j++] = left->elements[i];

    left->len--;
}