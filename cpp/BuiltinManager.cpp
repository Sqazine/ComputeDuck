#include "BuiltinManager.h"
#include <ctime>
#include "Object.h"

namespace
{
	extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(print)(Value *args, uint8_t argCount, Value &result)
	{
		if (argCount > 0)
		{
			for (size_t i = 0; i < argCount; ++i)
			{
				std::cout << args[i].Stringify();
				if (i < argCount - 1)
					std::cout << ",";
			}
		}
		return false;
	}

	extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(println)(Value *args, uint8_t argCount, Value &result)
	{
		if (argCount > 0)
		{
			for (size_t i = 0; i < argCount; ++i)
			{
				std::cout << args[i].Stringify();
				if (i < argCount - 1)
					std::cout << ",";
			}

			std::cout << std::endl;
		}
		return false;
	}

	extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(sizeof)(Value *args, uint8_t argCount, Value &result)
	{
		if (argCount == 0 || argCount > 1)
			ASSERT("[Native function 'sizeof']:Expect a argument.");

		if (IS_ARRAY_VALUE(args[0]))
			result = Value(TO_ARRAY_VALUE(args[0])->len);
		else if (IS_STR_VALUE(args[0]))
			result = Value(TO_STR_VALUE(args[0])->len);
		else
			ASSERT("[Native function 'sizeof']:Expect a array or string argument.");
		return true;
	}

	extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(insert)(Value *args, uint8_t argCount, Value &result)
	{
		if (argCount == 0 || argCount != 3)
			ASSERT("[Native function 'insert']:Expect 3 arguments,the arg0 must be array or string object.The arg1 is the index object.The arg2 is the value object.");

        if (IS_ARRAY_VALUE(args[0]))
        {
            ArrayObject* array = TO_ARRAY_VALUE(args[0]);
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

	extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(erase)(Value *args, uint8_t argCount, Value &result)
	{
		if (argCount == 0 || argCount != 2)
			ASSERT("[Native function 'erase']:Expect 2 arguments,the arg0 must be array or string object.The arg1 is the corresponding index object.");

        if (IS_ARRAY_VALUE(args[0]))
        {
            ArrayObject* array = TO_ARRAY_VALUE(args[0]);
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

	extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(clock)(Value *args, uint8_t argCount, Value &result)
	{
		result = Value(clock() / CLOCKS_PER_SEC);
		return true;
	}
}

BuiltinManager *BuiltinManager::GetInstance()
{
	static BuiltinManager instance;
	return &instance;
}

BuiltinManager::BuiltinManager()
{
	Register<BuiltinFn>("print", BUILTIN_FN(print));
	Register<BuiltinFn>("println", BUILTIN_FN(println));
	Register<BuiltinFn>("sizeof", BUILTIN_FN(sizeof));
	Register<BuiltinFn>("insert", BUILTIN_FN(insert));
	Register<BuiltinFn>("erase", BUILTIN_FN(erase));
	Register<BuiltinFn>("clock", BUILTIN_FN(clock));
}
BuiltinManager::~BuiltinManager()
{
	std::unordered_map<std::string_view, BuiltinObject *>().swap(m_BuiltinObjects);
}

BuiltinObject *BuiltinManager::FindBuiltinObject(std::string_view name)
{
	auto iter = m_BuiltinObjects.find(name);
	if (iter == m_BuiltinObjects.end())
		ASSERT("No builtin object:%s", name.data());
	return iter->second;
}

const std::unordered_map<std::string_view, BuiltinObject *> BuiltinManager::GetBuiltinObjectList() const
{
	return m_BuiltinObjects;
}
