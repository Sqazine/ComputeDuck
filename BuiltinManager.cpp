#include "BuiltinManager.h"
#include <ctime>
#include <filesystem>
#include "Object.h"

namespace
{
	extern "C" COMPUTE_DUCK_API Value gPrint(Value * args, uint8_t argCount)
	{
		if (argCount > 0)
			for (size_t i; i < argCount; ++i)
				std::cout << args[i].Stringify();

		return Value();
	}

	extern "C" COMPUTE_DUCK_API Value gPrintln(Value * args, uint8_t argCount)
	{
		if (argCount > 0)
		{
			for (size_t i; i < argCount; ++i)
				std::cout << args[i].Stringify();
			std::cout << std::endl;;
		}
		return Value();
	}

	extern "C" COMPUTE_DUCK_API Value gSizeofFn(Value * args, uint8_t argCount)
	{
		if (argCount == 0 || argCount > 1)
			ASSERT("[Native function 'sizeof']:Expect a argument.");

		if (IS_ARRAY_VALUE(args[0]))
			return ((double)TO_ARRAY_VALUE(args[0])->elements.size());
		else if (IS_STR_VALUE(args[0]))
			return ((double)TO_STR_VALUE(args[0])->value.size());
		else
			ASSERT("[Native function 'sizeof']:Expect a array or string argument.");
		return Value();
	}

	extern "C" COMPUTE_DUCK_API Value gInsert(Value * args, uint8_t argCount)
	{
		if (argCount == 0 || argCount != 3)
			ASSERT("[Native function 'insert']:Expect 3 arguments,the arg0 must be array or string object.The arg1 is the index object.The arg2 is the value object.");

		if (IS_ARRAY_VALUE(args[0]))
		{
			ArrayObject* array = TO_ARRAY_VALUE(args[0]);
			if (!IS_NUM_VALUE(args[1]))
				ASSERT("[Native function 'insert']:Arg1 must be integer type while insert to a array");

			size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

			if (iIndex < 0 || iIndex >= array->elements.size())
				ASSERT("[Native function 'insert']:Index out of array's range");

			array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
		}
		else if (IS_STR_VALUE(args[0]))
		{
			StrObject* string = TO_STR_VALUE(args[0]);
			if (!IS_NUM_VALUE(args[1]))
				ASSERT("[Native function 'insert']:Arg1 must be integer type while insert to a string");

			size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

			if (iIndex < 0 || iIndex >= string->value.size())
				ASSERT("[Native function 'insert']:Index out of array's range");

			string->value.insert(iIndex, args[2].Stringify());
		}
		else
			ASSERT("[Native function 'insert']:Expect a array or string argument.");
		return Value();
	}

	extern "C" COMPUTE_DUCK_API Value gErase(Value * args, uint8_t argCount)
	{
		if (argCount == 0 || argCount != 2)
			ASSERT("[Native function 'erase']:Expect 2 arguments,the arg0 must be array or string object.The arg1 is the corresponding index object.");

		if (IS_ARRAY_VALUE(args[0]))
		{
			ArrayObject* array = TO_ARRAY_VALUE(args[0]);
			if (!IS_NUM_VALUE(args[1]))
				ASSERT("[Native function 'erase']:Arg1 must be integer type while deleting array element");

			size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

			if (iIndex < 0 || iIndex >= array->elements.size())
				ASSERT("[Native function 'erase']:Index out of array's range");

			array->elements.erase(array->elements.begin() + iIndex);
		}
		else if (IS_STR_VALUE(args[0]))
		{
			StrObject* string = TO_STR_VALUE(args[0]);
			if (!IS_NUM_VALUE(args[1]))
				ASSERT("[Native function 'erase']:Arg1 must be integer type while deleting string element");

			size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

			if (iIndex < 0 || iIndex >= string->value.size())
				ASSERT("[Native function 'erase']:Index out of array's range");

			string->value.erase(string->value.begin() + iIndex);
		}
		else
			ASSERT("[Native function 'erase']:Expect a array or string argument.");
		return Value();
	}

	extern "C" COMPUTE_DUCK_API Value gClock(Value * args, uint8_t argCount)
	{
		return ((double)clock() / CLOCKS_PER_SEC);
	}
}

std::string BuiltinManager::m_CurExecuteFilePath;

BuiltinManager *BuiltinManager::GetInstance()
{
	static BuiltinManager instance;
	return &instance;
}

BuiltinManager::BuiltinManager()
{
	Register<BuiltinFn>("print", gPrint);
	Register<BuiltinFn>("println", gPrintln);
	Register<BuiltinFn>("sizeof", gSizeofFn);
	Register<BuiltinFn>("insert", gInsert);
	Register<BuiltinFn>("erase", gErase);
	Register<BuiltinFn>("clock", gClock);
}
BuiltinManager::~BuiltinManager()
{
	std::vector<BuiltinObject *>().swap(m_Builtins);
	std::vector<std::string>().swap(m_BuiltinNames);
}

#ifdef BUILD_WITH_LLVM
void BuiltinManager::RegisterLlvmFn(std::string_view name, llvm::Function *fn)
{
	m_LlvmBuiltins[name.data()] = fn;
}
#endif

void BuiltinManager::SetExecuteFilePath(std::string_view path)
{
	m_CurExecuteFilePath = path;
}
const std::string &BuiltinManager::GetExecuteFilePath() const
{
	return m_CurExecuteFilePath;
}

std::string BuiltinManager::ToFullPath(std::string_view filePath)
{
	std::filesystem::path filesysPath = filePath;
	std::string fullPath = filesysPath.string();
	if (!filesysPath.is_absolute())
		fullPath = BuiltinManager::GetInstance()->GetExecuteFilePath() + fullPath;
	return fullPath;
}
