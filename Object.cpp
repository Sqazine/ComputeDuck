#include "Object.h"

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
    case ObjectType::STRUCT:
    {
        auto structObj = TO_STRUCT_OBJ(object);
        std::string result = "struct instance(0x" + PointerAddressToString(object) + "):\n{\n";
        for (size_t i = 0; i < structObj->members->GetCapacity(); ++i)
        {
            if (structObj->members->IsValid(i))
            {
                auto key = structObj->members->GetEntries()[i].key;
                auto value = structObj->members->GetEntries()[i].value;
                result += ObjectStringify(key) + ":" + value.Stringify() + "\n";
            }
        }
        result = result.substr(0, result.size() - 1);
        result += "\n}\n";
        return result;
    }
    case ObjectType::REF:
        return TO_REF_OBJ(object)->pointer->Stringify();
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
    }
}

void MarkObject(Object *object)
{
    if (object == nullptr)
        return;

    object->marked = true;
    switch (object->type)
    {
    case ObjectType::ARRAY:
    {
        for (int32_t i = 0; i < TO_ARRAY_OBJ(object)->len; ++i)
            TO_ARRAY_OBJ(object)->elements[i].Mark();
        break;
    }
    case ObjectType::STRUCT:
    {
        TO_STRUCT_OBJ(object)->members->Mark();
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
    case ObjectType::UPVALUE:
    {
        TO_UPVALUE_OBJ(object)->closed.Mark();
        break;
    }
    case ObjectType::CLOSURE:
    {
        ClosureObject* closure =TO_CLOSURE_OBJ(object);
        MarkObject(closure->function);
        for (size_t i = 0; i < UPVALUE_COUNT; ++i)
        {
            UpvalueObject* upvalue = closure->upvalues[i];
            MarkObject(upvalue);
        }
        break;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(object)->Is<Value>())
            TO_BUILTIN_OBJ(object)->Get<Value>().Mark();
        break;
    }
    case ObjectType::STR:
    default:
        break;
    }
}

void UnMarkObject(Object *object)
{
    if (object == nullptr)
        return;

    object->marked = false;
    switch (object->type)
    {
    case ObjectType::ARRAY:
    {
        for (int32_t i = 0; i < TO_ARRAY_OBJ(object)->len; ++i)
            TO_ARRAY_OBJ(object)->elements[i].UnMark();
        break;
    }
    case ObjectType::STRUCT:
    {
        TO_STRUCT_OBJ(object)->members->UnMark();
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
    case ObjectType::UPVALUE:
    {
        TO_UPVALUE_OBJ(object)->closed.UnMark();
        break;
    }
    case ObjectType::CLOSURE:
    {
        UnMarkObject(TO_CLOSURE_OBJ(object)->function);
        for (size_t i = 0; i < UPVALUE_COUNT; ++i)
        {
            auto upvalue = TO_CLOSURE_OBJ(object)->upvalues[i];
            UnMarkObject(upvalue);
        }
        break;
    }
    case ObjectType::BUILTIN:
    {
        if (TO_BUILTIN_OBJ(object)->Is<Value>())
            TO_BUILTIN_OBJ(object)->Get<Value>().UnMark();
        break;
    }
    case ObjectType::STR:
    default:
        break;
    }
}

COMPUTEDUCK_API void DeleteObject(Object *object)
{
    switch (object->type)
    {
    case ObjectType::STR:
    {
        auto strObj = TO_STR_OBJ(object);
        SAFE_DELETE(strObj);
        return;
    }
    case ObjectType::ARRAY:
    {
        auto arrObj = TO_ARRAY_OBJ(object);
        SAFE_DELETE(arrObj);
        return;
    }
    case ObjectType::STRUCT:
    {
        auto structObj = TO_STRUCT_OBJ(object);
        SAFE_DELETE(structObj);
        return;
    }
    case ObjectType::REF:
    {
        auto refObj = TO_REF_OBJ(object);
        SAFE_DELETE(refObj);
        return;
    }
    case ObjectType::FUNCTION:
    {
        auto fnObj = TO_FUNCTION_OBJ(object);
        SAFE_DELETE(fnObj);
        return;
    }
    case ObjectType::UPVALUE:
    {
        auto upvalueObj = TO_UPVALUE_OBJ(object);
        SAFE_DELETE(upvalueObj);
        return;
    }
    case ObjectType::CLOSURE:
    {
        auto closureObj = TO_CLOSURE_OBJ(object);
        SAFE_DELETE(closureObj);
        return;
    }
    case ObjectType::BUILTIN:
    {
        auto builtinObj = TO_BUILTIN_OBJ(object);
        SAFE_DELETE(builtinObj);
        return;
    }
    default:
        return;
    }
}

bool IsObjectEqual(Object *left, Object *right)
{
    if ((left == nullptr || right == nullptr) || (left->type != right->type))
        return false;
    switch (left->type)
    {
    case ObjectType::STR:
        return TO_STR_OBJ(left)->hash == TO_STR_OBJ(right)->hash;
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
        return TO_STRUCT_OBJ(left)->members == TO_STRUCT_OBJ(right)->members;
    case ObjectType::REF:
        return *TO_REF_OBJ(left)->pointer == *TO_REF_OBJ(right)->pointer;
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
        if (TO_BUILTIN_OBJ(left)->Is<NativeData>())
        {
            auto thisNd = TO_BUILTIN_OBJ(left)->Get<NativeData>();
            auto otherNd = TO_BUILTIN_OBJ(right)->Get<NativeData>();
            return PointerAddressToString(thisNd.nativeData) == PointerAddressToString(otherNd.nativeData);
        }
        else
            return TO_BUILTIN_OBJ(left)->data.index() == TO_BUILTIN_OBJ(right)->data.index();
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
    return new StrObject(newStr);
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