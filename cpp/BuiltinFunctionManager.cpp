#include "BuiltinFunctionManager.h"
#include <ctime>
std::vector<BuiltinFunctionObject *> BuiltinFunctionManager::m_Builtins;
std::vector<std::string> BuiltinFunctionManager::m_BuiltinIdx;

BuiltinFunctionManager::BuiltinFunctionManager()
{
}
BuiltinFunctionManager::~BuiltinFunctionManager()
{
}

void BuiltinFunctionManager::Init()
{
    Register("print", [&](const std::vector<Value> &args, Value &result) -> bool
             {
                 if (args.empty())
                     return false;

                 std::cout << args[0].Stringify();
                 return false;
             });

    Register("println", [&](const std::vector<Value> &args, Value &result) -> bool
             {
                 if (args.empty())
                     return false;

                 std::cout << args[0].Stringify() << std::endl;
                 return false;
             });

    Register("sizeof", [&](const std::vector<Value> &args, Value &result) -> bool
             {
                 if (args.empty() || args.size() > 1)
                     Assert("[Native function 'sizeof']:Expect a argument.");

                 if (IS_ARRAY_VALUE(args[0]))
                     result = Value((double)TO_ARRAY_VALUE(args[0])->elements.size());
                 else if (IS_STR_VALUE(args[0]))
                     result = Value((double)TO_STR_VALUE(args[0])->value.size());
                 else
                     Assert("[Native function 'sizeof']:Expect a array or string argument.");
                 return true;
             });

    Register("insert", [&](const std::vector<Value> &args, Value &result) -> bool
             {
                 if (args.empty() || args.size() != 3)
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

    Register("erase", [&](const std::vector<Value> &args, Value &result) -> bool
             {
                 if (args.empty() || args.size() != 2)
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

    Register("clock", [&](const std::vector<Value> &args, Value &result) -> bool
             {
                 result = Value((double)clock() / CLOCKS_PER_SEC);
                 return true;
             });
}
void BuiltinFunctionManager::Release()
{
    std::vector<BuiltinFunctionObject *>().swap(m_Builtins);
    std::vector<std::string>().swap(m_BuiltinIdx);
}

void BuiltinFunctionManager::Register(std::string_view name, const BuiltinFn &fn)
{
    m_Builtins.emplace_back(new BuiltinFunctionObject(name, fn));
    m_BuiltinIdx.emplace_back(name);
}