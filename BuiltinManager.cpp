#include "BuiltinManager.h"
#include <ctime>

BuiltinManager *BuiltinManager::GetInstance()
{
   static BuiltinManager instance;
    return &instance;
}

void BuiltinManager::Init()
{
    RegisterFunction("print", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0)
                             return false;

                         std::cout << args[0].Stringify();
                         return false;
                     });

    RegisterFunction("println", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0)
                             return false;

                         std::cout << args[0].Stringify() << std::endl;
                         return false;
                     });

    RegisterFunction("sizeof", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0 || argCount > 1)
                             Assert("[Native function 'sizeof']:Expect a argument.");

                         if (IS_ARRAY_VALUE(args[0]))
                             result = Value((double)TO_ARRAY_VALUE(args[0])->elements.size());
                         else if (IS_STR_VALUE(args[0]))
                             result = Value((double)TO_STR_VALUE(args[0])->value.size());
                         else
                             Assert("[Native function 'sizeof']:Expect a array or string argument.");
                         return true;
                     });

    RegisterFunction("insert", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0 || argCount != 3)
                             Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.");

                         if (IS_ARRAY_VALUE(args[0]))
                         {
                             ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= array->elements.size())
                                 Assert("[Native function 'insert']:Index out of array's range");

                             array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
                         }
                         else if (IS_STR_VALUE(args[0]))
                         {
                             StrObject *string = TO_STR_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= string->value.size())
                                 Assert("[Native function 'insert']:Index out of array's range");

                             string->value.insert(iIndex, args[2].Stringify());
                         }
                         else
                             Assert("[Native function 'insert']:Expect a array,table ot string argument.");
                         return false;
                     });

    RegisterFunction("erase", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0 || argCount != 2)
                             Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the corresponding index object.");

                         if (IS_ARRAY_VALUE(args[0]))
                         {
                             ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= array->elements.size())
                                 Assert("[Native function 'erase']:Index out of array's range");

                             array->elements.erase(array->elements.begin() + iIndex);
                         }
                         else if (IS_STR_VALUE(args[0]))
                         {
                             StrObject *string = TO_STR_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= string->value.size())
                                 Assert("[Native function 'erase']:Index out of array's range");

                             string->value.erase(string->value.begin() + iIndex);
                         }
                         else
                             Assert("[Native function 'erase']:Expect a array,table ot string argument.");
                         return false;
                     });

    RegisterFunction("clock", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         result = Value((double)clock() / CLOCKS_PER_SEC);
                         return true;
                     });
}
void BuiltinManager::Release()
{
    std::vector<BuiltinFunctionObject *>().swap(m_BuiltinFunctions);
    std::vector<std::string>().swap(m_BuiltinFunctionNames);

    std::vector<BuiltinVariableObject *>().swap(m_BuiltinVariables);
    std::vector<std::string>().swap(m_BuiltinVariableNames);
}

void BuiltinManager::RegisterFunction(std::string_view name, const BuiltinFn &fn)
{
    m_BuiltinFunctions.emplace_back(new BuiltinFunctionObject(name, fn));
    m_BuiltinFunctionNames.emplace_back(name);
}

void BuiltinManager::RegisterVariable(std::string_view name, const Value &value)
{
    m_BuiltinVariables.emplace_back(new BuiltinVariableObject(name, value));
    m_BuiltinVariableNames.emplace_back(name);
}