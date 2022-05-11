#include "VM.h"

VM::VM()
	: m_Context(nullptr)
{
	ResetStatus();

	m_NativeFunctions["print"] = [this](std::vector<Value> args) -> Value
	{
		if (args.empty())
			return g_UnknownValue;

		std::cout << args[0].Stringify();
		return g_UnknownValue;
	};

	m_NativeFunctions["println"] = [this](std::vector<Value> args) -> Value
	{
		if (args.empty())
			return g_UnknownValue;

		std::cout << args[0].Stringify() << std::endl;
		return g_UnknownValue;
	};

	m_NativeFunctions["sizeof"] = [this](std::vector<Value> args) -> Value
	{
		if (args.empty() || args.size() > 1)
			Assert("[Native function 'sizeof']:Expect a argument.");

		if ( IS_ARRAY_VALUE(args[0]))
			return Value((double)TO_ARRAY_VALUE(args[0])->elements.size());
		else if (IS_STR_VALUE(args[0]))
			return Value((double)TO_STR_VALUE(args[0])->value.size());
		else
			Assert("[Native function 'sizeof']:Expect a array or string argument.");
		return g_UnknownValue;
	};

	m_NativeFunctions["insert"] = [this](std::vector<Value> args) -> Value
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
		return g_UnknownValue;
	};

	m_NativeFunctions["erase"] = [this](std::vector<Value> args) -> Value
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
		return g_UnknownValue;
	};
}
VM::~VM()
{
	if (m_Context)
	{
		delete m_Context;
		m_Context = nullptr;
	}
	sp = 0;
	Gc();
}

StrObject *VM::CreateStrObject(std::string_view value)
{
	if (curObjCount == maxObjCount)
		Gc();

	StrObject *object = new StrObject(value);
	object->marked = false;

	object->next = firstObject;
	firstObject = object;

	curObjCount++;

	return object;
}

ArrayObject *VM::CreateArrayObject(const std::vector<Value> &elements)
{
	if (curObjCount == maxObjCount)
		Gc();

	ArrayObject *object = new ArrayObject(elements);
	object->marked = false;

	object->next = firstObject;
	firstObject = object;

	curObjCount++;

	return object;
}

StructObject *VM::CreateStructObject(std::string_view name,
									 const std::unordered_map<std::string, Value> &members)
{
	if (curObjCount == maxObjCount)
		Gc();

	StructObject *object = new StructObject(name, members);
	object->marked = false;

	object->next = firstObject;
	firstObject = object;

	curObjCount++;

	return object;
}

RefObject *VM::CreateRefObject(std::string_view name)
{
	if (curObjCount == maxObjCount)
		Gc();

	RefObject *refObject = new RefObject(name);
	refObject->marked = false;

	refObject->next = firstObject;
	firstObject = refObject;

	curObjCount++;

	return refObject;
}

LambdaObject *VM::CreateLambdaObject(int64_t idx)
{
	if (curObjCount == maxObjCount)
		Gc();

	LambdaObject *lambdaObject = new LambdaObject(idx);
	lambdaObject->marked = false;

	lambdaObject->next = firstObject;
	firstObject = lambdaObject;

	curObjCount++;

	return lambdaObject;
}

Value VM::Execute(Frame frame)
{
	// + - * /
#define COMMON_BINARY(op)                                                                \
	do                                                                                   \
	{                                                                                    \
		Value left = Pop();                                                              \
		Value right = Pop();                                                             \
		if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                   \
			Push(Value(TO_NUM_VALUE(left) op TO_NUM_VALUE(right)));                      \
		else                                                                             \
			Assert("Invalid binary op:" + left.Stringify() + (#op) + right.Stringify()); \
	} while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                \
	do                                                                                    \
	{                                                                                     \
		Value left = Pop();                                                               \
		Value right = Pop();                                                              \
		if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                    \
			Push(TO_NUM_VALUE(left) op TO_NUM_VALUE(right) ? Value(true) : Value(false)); \
		else                                                                              \
			Push(Value(false));                                                           \
	} while (0);

//and or
#define LOGIC_BINARY(op)                                                                    \
	do                                                                                      \
	{                                                                                       \
		Value left = Pop();                                                                 \
		Value right = Pop();                                                                \
		if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                    \
			Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
		else                                                                                \
			Assert("Invalid op:" + left.Stringify() + (#op) + right.Stringify());           \
	} while (0);

	for (size_t ip = 0; ip < frame.m_Codes.size(); ++ip)
	{
		uint8_t instruction = frame.m_Codes[ip];
		switch (instruction)
		{
		case OP_RETURN:
			if (m_Context->m_UpContext)
			{
				Context *tmp = m_Context->m_UpContext;
				delete m_Context;
				m_Context = tmp;
			}
			return Pop();
			break;
		case OP_NEW_NUM:
			Push(Value(frame.m_Nums[frame.m_Codes[++ip]]));
			break;
		case OP_NEW_STR:
			Push(CreateStrObject(frame.m_Strings[frame.m_Codes[++ip]]));
			break;
		case OP_NEW_TRUE:
			Push(Value(true));
			break;
		case OP_NEW_FALSE:
			Push(Value(false));
			break;
		case OP_NEW_NIL:
			Push(Value());
			break;
		case OP_NEG:
		{
			auto value = Pop();
			if (IS_NUM_VALUE(value))
				Push(Value(-TO_NUM_VALUE(value)));
			else
				Assert("Invalid op:'-'" + value.Stringify());
			break;
		}
		case OP_NOT:
		{
			auto value = Pop();
			if (IS_BOOL_VALUE(value))
				Push(Value(!TO_BOOL_VALUE(value)));
			else
				Assert("Invalid op:'not' " + value.Stringify());
			break;
		}
		case OP_ADD:
			COMMON_BINARY(+);
			break;
		case OP_SUB:
			COMMON_BINARY(-);
			break;
		case OP_MUL:
			COMMON_BINARY(*);
			break;
		case OP_DIV:
			COMMON_BINARY(/);
			break;
		case OP_GREATER:
			COMPARE_BINARY(>);
			break;
		case OP_LESS:
			COMPARE_BINARY(<);
			break;
		case OP_GREATER_EQUAL:
			COMPARE_BINARY(>=);
			break;
		case OP_LESS_EQUAL:
			COMPARE_BINARY(<=);
			break;
		case OP_EQUAL:
		{
			Value left = Pop();
			Value right = Pop();
			Push(Value(left.IsEqualTo(right)));
			break;
		}
		case OP_NOT_EQUAL:
		{
			Value left = Pop();
			Value right = Pop();
			Push(Value(!left.IsEqualTo(right)));
			break;
		}
		case OP_AND:
			LOGIC_BINARY(&&);
			break;
		case OP_OR:
			LOGIC_BINARY(||);
			break;
		case OP_DEFINE_VAR:
		{
			Value value = Pop();
			m_Context->DefineVariableByName(frame.m_Strings[frame.m_Codes[++ip]], value);
			break;
		}
		case OP_SET_VAR:
		{
			std::string name = frame.m_Strings[frame.m_Codes[++ip]];

			Value value = Pop();
			Value variable = m_Context->GetVariableByName(name);

			if (IS_REF_VALUE(variable))
				m_Context->AssignVariableByName(TO_REF_VALUE(variable)->name, value);
			else
				m_Context->AssignVariableByName(name, value);
			break;
		}
		case OP_GET_VAR:
		{
			std::string name = frame.m_Strings[frame.m_Codes[++ip]];

			Value varValue = m_Context->GetVariableByName(name);
			//create a struct object
			if (varValue.IsEqualTo(g_UnknownValue))
			{
				if (frame.HasStructFrame(name))
					Push(Execute(frame.GetStructFrame(name)));
				else
					Assert("No struct definition:" + name);
			}
			else if (IS_REF_VALUE(varValue))
			{
				varValue = m_Context->GetVariableByName(TO_REF_VALUE(varValue)->name);
				Push(varValue);
			}
			else
				Push(varValue);
			break;
		}
		case OP_NEW_ARRAY:
		{
			std::vector<Value> elements;
			int64_t arraySize = (int64_t)frame.m_Nums[frame.m_Codes[++ip]];
			for (int64_t i = 0; i < arraySize; ++i)
				elements.insert(elements.begin(), Pop());
			Push(CreateArrayObject(elements));
			break;
		}
		case OP_NEW_STRUCT:
		{
			Push(CreateStructObject(frame.m_Strings[frame.m_Codes[++ip]], m_Context->m_Values));
			break;
		}
		case OP_NEW_LAMBDA:
		{
			Push(CreateLambdaObject(frame.m_Nums[frame.m_Codes[++ip]]));
			break;
		}
		case OP_GET_INDEX_VAR:
		{
			Value index = Pop();
			Value object = Pop();
			if (IS_ARRAY_VALUE(object))
			{
				ArrayObject *arrayObject = TO_ARRAY_VALUE(object);
				if (!IS_NUM_VALUE(index))
					Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());

				size_t iIndex = (size_t)TO_NUM_VALUE(index);

				if (iIndex < 0 || iIndex >= arrayObject->elements.size())
					Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

				Push(arrayObject->elements[iIndex]);
			}
			else if (IS_STR_VALUE(object))
			{
				StrObject *strObject = TO_STR_VALUE(object);
				if (!IS_NUM_VALUE(index))
					Assert("Invalid index op.The index type of the string object must ba a int num type,but got:" + index.Stringify());

				size_t iIndex = (size_t)TO_NUM_VALUE(index);

				if (iIndex < 0 || iIndex >= strObject->value.size())
					Assert("Index out of string range,string size:" + std::to_string(strObject->value.size()) + ",index:" + std::to_string(iIndex));

				Push(CreateStrObject(strObject->value.substr(iIndex, 1)));
			}
			else
				Assert("Invalid index op.The indexed object isn't a array or a string object:" + object.Stringify());
			break;
		}
		case OP_SET_INDEX_VAR:
		{
			Value index = Pop();
			Value object = Pop();
			Value assigner = Pop();

			if (IS_ARRAY_VALUE(object))
			{
				ArrayObject *arrayObject = TO_ARRAY_VALUE(object);
				if (!IS_NUM_VALUE(index))
					Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());

				size_t iIndex = (size_t)TO_NUM_VALUE(index);

				if (iIndex < 0 || iIndex >= arrayObject->elements.size())
					Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

				arrayObject->elements[iIndex] = assigner;
			}
			else if (IS_STR_VALUE(object))
			{
				StrObject *strObject = TO_STR_VALUE(object);
				if (!IS_NUM_VALUE(index))
					Assert("Invalid index op.The index type of the string object must ba a int num type,but got:" + index.Stringify());

				size_t iIndex = (size_t)TO_NUM_VALUE(index);

				if (iIndex < 0 || iIndex >= strObject->value.size())
					Assert("Index out of string range,string size:" + std::to_string(strObject->value.size()) + ",index:" + std::to_string(iIndex));

				if (!TO_STR_VALUE(assigner))
					Assert("The assigner isn't a string.");

				strObject->value.insert(iIndex, TO_STR_VALUE(assigner)->value);
			}
			else
				Assert("Invalid index op.The indexed object isn't a array object:" + object.Stringify());
			break;
		}
		case OP_GET_STRUCT_VAR:
		{
			std::string memberName = frame.m_Strings[frame.m_Codes[++ip]];
			Value stackTop = Pop();
			if (!IS_STRUCT_VALUE(stackTop))
				Assert("Not a struct object of the callee of:" + memberName);
			StructObject *structObj = TO_STRUCT_VALUE(stackTop);
			Push(structObj->GetMember(memberName));
			break;
		}
		case OP_SET_STRUCT_VAR:
		{
			std::string memberName = frame.m_Strings[frame.m_Codes[++ip]];
			Value stackTop = Pop();
			if (!IS_STRUCT_VALUE(stackTop))
				Assert("Not a struct object of the callee of:" + memberName);
			StructObject *structObj = TO_STRUCT_VALUE(stackTop);
			Value assigner = Pop();
			structObj->AssignMember(memberName, assigner);
			break;
		}
		case OP_ENTER_SCOPE:
		{
			m_Context = new Context(m_Context);
			break;
		}
		case OP_EXIT_SCOPE:
		{
			Context *tmp = m_Context->m_UpContext;
			delete m_Context;
			m_Context = tmp;
			break;
		}
		case OP_JUMP_IF_FALSE:
		{
			bool isJump = !TO_BOOL_VALUE(Pop());
			uint64_t address = (uint64_t)(frame.m_Nums[frame.m_Codes[++ip]]);

			if (isJump)
				ip = address;
			break;
		}
		case OP_JUMP:
		{
			uint64_t address = (uint64_t)(frame.m_Nums[frame.m_Codes[++ip]]);
			ip = address;
			break;
		}
		case OP_FUNCTION_CALL:
		{
			std::string fnName = frame.m_Strings[frame.m_Codes[++ip]];
			auto argCount = TO_NUM_VALUE(Pop());

			if (frame.HasFunctionFrame(fnName))
				Push(Execute(frame.GetFunctionFrame(fnName)));
			else if (HasNativeFunction(fnName))
			{
				std::vector<Value> args;
				for (int64_t i = 0; i < argCount; ++i)
					args.insert(args.begin(), Pop());

				Value result = GetNativeFunction(fnName)(args);
				if (!result.IsEqualTo(g_UnknownValue))
					Push(result);
			}
			else if (IS_LAMBDA_VALUE(m_Context->GetVariableByName(fnName))) //lambda
			{
				auto lambdaObject = TO_LAMBDA_VALUE(m_Context->GetVariableByName(fnName));
				Push(Execute(frame.GetLambdaFrame(lambdaObject->idx)));
			}
			else
				Assert("No function:" + fnName);
			break;
		}
		case OP_STRUCT_LAMBDA_CALL:
		{
			std::string fnName = frame.m_Strings[frame.m_Codes[++ip]];
			auto stackTop = Pop();
			auto argCount = TO_NUM_VALUE(Pop());
			if (!IS_STRUCT_VALUE(stackTop))
				Assert("Cannot call a struct lambda function:" + fnName + ",the callee isn't a struct object");
			auto structObj = TO_STRUCT_VALUE(stackTop);

			auto member = structObj->GetMember(fnName);
			if (member.IsEqualTo(g_UnknownValue))
				Assert("No member in struct:" + structObj->name);
			if (!IS_LAMBDA_VALUE(member))
				Assert("Not a lambda function:" + fnName + " in struct:" + structObj->name);
			auto lambdaObject = TO_LAMBDA_VALUE(member);
			Push(Execute(frame.GetLambdaFrame(lambdaObject->idx)));
			break;
		}
		case OP_REF:
		{
			Push(CreateRefObject(frame.m_Strings[frame.m_Codes[++ip]]));
			break;
		}
		default:
			break;
		}
	}

	return Value();
}

void VM::ResetStatus()
{
	sp = 0;
	firstObject = nullptr;
	curObjCount = 0;
	maxObjCount = INITIAL_GC_THRESHOLD;

	std::array<Value, STACK_MAX>().swap(m_ValueStack);

	if (m_Context != nullptr)
	{
		delete m_Context;
		m_Context = nullptr;
	}
	m_Context = new Context();
}

std::function<Value(std::vector<Value>)> VM::GetNativeFunction(std::string_view fnName)
{
	auto iter = m_NativeFunctions.find(fnName.data());
	if (iter != m_NativeFunctions.end())
		return iter->second;
	Assert(std::string("No native function:") + fnName.data());

	return nullptr;
}
bool VM::HasNativeFunction(std::string_view name)
{
	auto iter = m_NativeFunctions.find(name.data());
	if (iter != m_NativeFunctions.end())
		return true;
	return false;
}

void VM::Push(Value value)
{
	m_ValueStack[sp++] = value;
}
Value VM::Pop()
{
	return m_ValueStack[--sp];
}

void VM::Gc()
{
	int objNum = curObjCount;

	//mark all object which in stack and in context
	for (size_t i = 0; i < sp; ++i)
		m_ValueStack[i].Mark();
	if (m_Context)
	{
		auto contextPtr = m_Context;
		for (const auto &[k, v] : contextPtr->m_Values)
			v.Mark();
		while (contextPtr->m_UpContext)
		{
			contextPtr = contextPtr->m_UpContext;
			for (const auto &[k, v] : contextPtr->m_Values)
				v.Mark();
		}
	}

	//sweep objects which is not reachable
	Object **object = &firstObject;
	while (*object)
	{
		if (!(*object)->marked)
		{
			Object *unreached = *object;
			*object = unreached->next;

			delete unreached;
			unreached = nullptr;
			curObjCount--;
		}
		else
		{
			(*object)->marked = false;
			object = &(*object)->next;
		}
	}

	maxObjCount = curObjCount == 0 ? INITIAL_GC_THRESHOLD : curObjCount * 2;

	std::cout << "Collected " << objNum - curObjCount << " objects," << curObjCount << " remaining." << std::endl;
}