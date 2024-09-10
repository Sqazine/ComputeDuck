#include "Object.h"

std::string ObjectStringify(Object *object
#ifndef NDEBUG
    , bool printChunkIfIsFunctionObject
#endif
)
{
    switch (object->type)
    {
    case ObjectType::STR:
    {
        auto strObj = TO_STR_OBJ(object);
        return TO_STR_OBJ(object)->value;
    }
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
    case ObjectType::STRUCT:
    {
        auto structObj=TO_STRUCT_OBJ(object);
        std::string result = "struct instance(0x" + PointerAddressToString(object) + "):\n{\n";
        for (size_t i = 0; i < structObj->members.GetCapacity(); ++i)
        {
            if(structObj->members.IsValid(i))
                result += ObjectStringify(structObj->members.GetEntries()[i].key) + ":" + structObj->members.GetEntries()[i].value.Stringify() + "\n";
        }
        result = result.substr(0, result.size() - 1);
        result += "\n}\n";
        return result;
    }
    case ObjectType::REF:
    {
        return TO_REF_OBJ(object)->pointer->Stringify();
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
    case ObjectType::BUILTIN:
    {
        std::string vStr;
        if (TO_BUILTIN_OBJ(object)->Is<BuiltinFn>())
            vStr = "(0x" + PointerAddressToString(object) + ")";
        else if (TO_BUILTIN_OBJ(object)->Is<NativeData>())
            vStr = "(0x" + PointerAddressToString(TO_BUILTIN_OBJ(object)->Get<NativeData>().nativeData) + ")";
        else
            vStr = TO_BUILTIN_OBJ(object)->Get<Value>().Stringify();

        return "Builtin :" + vStr;
    }
    default:
        ASSERT("Unknown object type");
        return nullptr;
    }
}

void ObjectMark(Object *object)
{
    object->marked = true;
    switch (object->type)
    {
    case ObjectType::STR:
    {
        break;
    }
    case ObjectType::ARRAY:
    {
        for (int32_t i = 0; i < TO_ARRAY_OBJ(object)->len; ++i)
            TO_ARRAY_OBJ(object)->elements[i].Mark();
        break;
    }
    case ObjectType::STRUCT:
    {
        TO_STRUCT_OBJ(object)->members.Mark();
        break;
    }
    case ObjectType::REF:
    {
        TO_REF_OBJ(object)->pointer->Mark();
        break;
    }
    case ObjectType::FUNCTION:
    {
        for (const auto &v : TO_FUNCTION_OBJ(object)->chunk.constants)
            v.Mark();
        break;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(object)->Is<Value>())
            TO_BUILTIN_OBJ(object)->Get<Value>().Mark();
        break;
    }
    default:
        break;
    }
}

void ObjectUnMark(Object *object)
{
    object->marked = false;
    switch (object->type)
    {
    case ObjectType::STR:
    {
        break;
    }
    case ObjectType::ARRAY:
    {
        for (int32_t i = 0; i < TO_ARRAY_OBJ(object)->len; ++i)
            TO_ARRAY_OBJ(object)->elements[i].UnMark();
        break;
    }
    case ObjectType::STRUCT:
    {
        TO_STRUCT_OBJ(object)->members.UnMark();
        break;
    }
    case ObjectType::REF:
    {
        TO_REF_OBJ(object)->pointer->UnMark();
        break;
    }
    case ObjectType::FUNCTION:
    {
        for (const auto &v : TO_FUNCTION_OBJ(object)->chunk.constants)
            v.UnMark();
        break;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(object)->Is<Value>())
            TO_BUILTIN_OBJ(object)->Get<Value>().UnMark();
        break;
    }
    default:
        break;
    }
}

bool IsObjectsEqual(Object *left, Object *right)
{
    if (left->type != right->type)
        return false;
    switch (left->type)
    {
    case ObjectType::STR:
    {
        return strcmp(TO_STR_OBJ(left)->value, TO_STR_OBJ(right)->value) == 0;
    }
    case ObjectType::ARRAY:
    {
        if (TO_ARRAY_OBJ(left)->len != TO_ARRAY_OBJ(right)->len)
            return false;
        for (size_t i = 0; i < TO_ARRAY_OBJ(left)->len; ++i)
            if (TO_ARRAY_OBJ(left)->elements[i] != TO_ARRAY_OBJ(right)->elements[i])
                return false;
        return true;
    }
    case ObjectType::STRUCT:
    {
        return TO_STRUCT_OBJ(left)->members== TO_STRUCT_OBJ(right)->members;
    }
    case ObjectType::REF:
    {
        return *TO_REF_OBJ(left)->pointer == *TO_REF_OBJ(right)->pointer;
    }
    case ObjectType::FUNCTION:
    {
        if (TO_FUNCTION_OBJ(left)->chunk.opCodes.size() != TO_FUNCTION_OBJ(right)->chunk.opCodes.size())
            return false;
        if (TO_FUNCTION_OBJ(left)->parameterCount != TO_FUNCTION_OBJ(right)->parameterCount)
            return false;
        if (TO_FUNCTION_OBJ(left)->localVarCount != TO_FUNCTION_OBJ(right)->localVarCount)
            return false;
        for (int32_t i = 0; i < TO_FUNCTION_OBJ(left)->chunk.opCodes.size(); ++i)
            if (TO_FUNCTION_OBJ(left)->chunk.opCodes[i] != TO_FUNCTION_OBJ(right)->chunk.opCodes[i])
                return false;
        return true;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(left)->Is<NativeData>())
        {
            auto thisNd = TO_BUILTIN_OBJ(left)->Get<NativeData>();
            auto otherNd = TO_BUILTIN_OBJ(right)->Get<NativeData>();
            return PointerAddressToString(thisNd.nativeData) == PointerAddressToString(otherNd.nativeData);
        }
        else
            return TO_BUILTIN_OBJ(left)->data.index() == TO_BUILTIN_OBJ(right)->data.index();
    }
    default:
        ASSERT("Unknown object type");
        return false;
    }
}


StrObject *StrAdd(StrObject *left, StrObject *right)
{
    int length = left->len + right->len;
    char *newStr = new char[length + 1];
    memcpy(newStr, left->value, left->len);
    memcpy(newStr + left->len, right->value, right->len);
    newStr[length] = '\0';
    return new StrObject(newStr);
}

void StrInsert(StrObject *left, uint32_t idx, StrObject *right)
{
    int length = left->len + right->len;
    char *newStr = new char[length + 1];
    memset(newStr, '\0', sizeof(newStr));
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
    //TODO:not implement yet
}

void ArrayErase(ArrayObject *left, uint32_t idx)
{
    int32_t i = 0, j = 0;
    for (; i < left->len; ++i)
        if (i != idx)
            left->elements[j++] = left->elements[i];

    left->len--;
}