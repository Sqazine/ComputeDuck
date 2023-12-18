#include "BuiltinManager.h"
#include <ctime>
#include <filesystem>

std::string BuiltinManager::m_CurExecuteFilePath;

BuiltinManager* BuiltinManager::GetInstance()
{
	static BuiltinManager instance;
	return &instance;
}

BuiltinManager::BuiltinManager()
{
	Register("print", [&](Value* args, uint8_t argCount, Value& result) -> bool
		{
			if (argCount == 0)
				return false;

			std::cout << args[0].Stringify();
			return false;
		});

	Register("println", [&](Value* args, uint8_t argCount, Value& result) -> bool
		{
			if (argCount == 0)
				return false;

			std::cout << args[0].Stringify() << std::endl;
			return false;
		});

	Register("sizeof", [&](Value* args, uint8_t argCount, Value& result) -> bool
		{
			if (argCount == 0 || argCount > 1)
				ASSERT("[Native function 'sizeof']:Expect a argument.");

			if (IS_ARRAY_VALUE(args[0]))
				result = Value((double)TO_ARRAY_VALUE(args[0])->elements.size());
			else if (IS_STR_VALUE(args[0]))
				result = Value((double)TO_STR_VALUE(args[0])->value.size());
			else
				ASSERT("[Native function 'sizeof']:Expect a array or string argument.");
			return true;
		});

	Register("insert", [&](Value* args, uint8_t argCount, Value& result) -> bool
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
			return false;
		});

	Register("erase", [&](Value* args, uint8_t argCount, Value& result) -> bool
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
			return false;
		});

	Register("clock", [&](Value* args, uint8_t argCount, Value& result) -> bool
		{
			result = Value((double)clock() / CLOCKS_PER_SEC);
			return true;
		});
}
BuiltinManager::~BuiltinManager()
{
	std::vector<BuiltinObject*>().swap(m_Builtins);
	std::vector<std::string>().swap(m_BuiltinNames);
}

void BuiltinManager::Register(std::string_view name, const BuiltinFn& fn)
{
	m_Builtins.emplace_back(new BuiltinObject(name, fn));
	m_BuiltinNames.emplace_back(name);
}

void BuiltinManager::Register(std::string_view name, const Value& value)
{
	m_Builtins.emplace_back(new BuiltinObject(name, value));
	m_BuiltinNames.emplace_back(name);
}

void BuiltinManager::SetExecuteFilePath(std::string_view path)
{
	m_CurExecuteFilePath = path;
}
const std::string& BuiltinManager::GetExecuteFilePath() const
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
