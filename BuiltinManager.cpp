#include "BuiltinManager.h"
#include <ctime>
#include "Value.h"
#include "Object.h"

namespace
{
    extern "C" COMPUTEDUCK_API bool BUILTIN_FN(print)(Value *args, uint8_t argCount, Value &result)
    {
        if (argCount > 0)
        {
            for (size_t i = 0; i < argCount; ++i)
            {
                std::cout << args[i].Stringify();
            }
        }
        return false;
    }

    extern "C" COMPUTEDUCK_API bool BUILTIN_FN(println)(Value *args, uint8_t argCount, Value &result)
    {
        if (argCount > 0)
        {
            for (size_t i = 0; i < argCount; ++i)
            {
                std::cout << args[i].Stringify();
            }

            std::cout << std::endl;
        }
        return false;
    }

    extern "C" COMPUTEDUCK_API bool BUILTIN_FN(sizeof)(Value *args, uint8_t argCount, Value &result)
    {
        if (argCount == 0 || argCount > 1)
            ASSERT("[Native function 'sizeof']:Expect a argument.");

        if (IS_ARRAY_VALUE(args[0]))
            result = TO_ARRAY_VALUE(args[0])->len;
        else if (IS_STR_VALUE(args[0]))
            result = TO_STR_VALUE(args[0])->len;
        else
            ASSERT("[Native function 'sizeof']:Expect a array or string argument.");
        return true;
    }

    extern "C" COMPUTEDUCK_API bool BUILTIN_FN(insert)(Value *args, uint8_t argCount, Value &result)
    {
        if (argCount == 0 || argCount != 3)
            ASSERT("[Native function 'insert']:Expect 3 arguments,the arg0 must be array or string object.The arg1 is the index object.The arg2 is the value object.");

        if (IS_ARRAY_VALUE(args[0]))
        {
            ArrayObject *array = TO_ARRAY_VALUE(args[0]);
            if (!IS_NUM_VALUE(args[1]))
                ASSERT("[Native function 'insert']:Arg1 must be integer type while insert to a array");

            size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

            if (iIndex < 0 || iIndex >= array->len)
                ASSERT("[Native function 'insert']:Index out of array's range");

            ArrayInsert(array, iIndex, args[2]);
        }
        else if (IS_STR_VALUE(args[0]))
        {
            if (!IS_NUM_VALUE(args[1]))
                ASSERT("[Native function 'insert']:Arg1 must be integer type while insert to a string");

            size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

            if (iIndex < 0 || iIndex > TO_STR_VALUE(args[0])->len)
                ASSERT("[Native function 'insert']:Index out of array's range");

            StrInsert(TO_STR_VALUE(args[0]), iIndex, TO_STR_VALUE(args[2]));
        }
        else
            ASSERT("[Native function 'insert']:Expect a array or string argument.");

        return false;
    }

    extern "C" COMPUTEDUCK_API bool BUILTIN_FN(erase)(Value *args, uint8_t argCount, Value &result)
    {
        if (argCount == 0 || argCount != 2)
            ASSERT("[Native function 'erase']:Expect 2 arguments,the arg0 must be array or string object.The arg1 is the corresponding index object.");

        if (IS_ARRAY_VALUE(args[0]))
        {
            ArrayObject *array = TO_ARRAY_VALUE(args[0]);
            if (!IS_NUM_VALUE(args[1]))
                ASSERT("[Native function 'erase']:Arg1 must be integer type while deleting array element");

            size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

            if (iIndex < 0 || iIndex >= array->len)
                ASSERT("[Native function 'erase']:Index out of array's range");

            ArrayErase(array, iIndex);
        }
        else if (IS_STR_VALUE(args[0]))
        {
            if (!IS_NUM_VALUE(args[1]))
                ASSERT("[Native function 'erase']:Arg1 must be integer type while deleting string element");

            size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

            if (iIndex < 0 || iIndex >= TO_STR_VALUE(args[0])->len)
                ASSERT("[Native function 'erase']:Index out of array's range");

            StrErase(TO_STR_VALUE(args[0]), iIndex);
        }
        else
            ASSERT("[Native function 'erase']:Expect a array or string argument.");

        return false;
    }

    extern "C" COMPUTEDUCK_API bool BUILTIN_FN(clock)(Value *args, uint8_t argCount, Value &result)
    {
        result = clock() / CLOCKS_PER_SEC;
        return true;
    }
}

BuiltinManager *BuiltinManager::GetInstance()
{
    static BuiltinManager instance;
    return &instance;
}

void BuiltinManager::Init()
{
    Allocator::GetInstance()->DisableGC();

    REGISTER_BUILTIN_FN(print);
    REGISTER_BUILTIN_FN(println);
    REGISTER_BUILTIN_FN(sizeof);
    REGISTER_BUILTIN_FN(insert);
    REGISTER_BUILTIN_FN(erase);
    REGISTER_BUILTIN_FN(clock);

    Allocator::GetInstance()->EnableGC();
}

BuiltinObject *BuiltinManager::FindBuiltinObject(StrObject* name)
{
    auto value = m_BuiltinObjectsTable.Get(name);
    if (!value)
        ASSERT("No builtin object:%s", ObjectStringify(name).c_str());
    return TO_BUILTIN_VALUE(*value);
}

HashTable& BuiltinManager::GetBuiltinObjectTable()
{
    return m_BuiltinObjectsTable;
}
