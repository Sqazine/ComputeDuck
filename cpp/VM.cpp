#include "VM.h"

VM::VM()
	: m_Context(nullptr)
{
	ResetStatus();

	m_NativeFunctions["println"] = [this](std::vector<Object *> args) -> Object *
	{
		if (args.empty())
			return nullptr;

		std::cout << args[0]->Stringify() << std::endl;
		return nullptr;
	};

	m_NativeFunctions["sizeof"] = [this](std::vector<Object *> args) -> Object *
	{
		if (args.empty() || args.size() > 1)
			Assert("[Native function 'sizeof']:Expect a argument.");

		if (IS_ARRAY_OBJ(args[0]))
			return CreateNumObject(TO_ARRAY_OBJ(args[0])->elements.size());
		else if (IS_STR_OBJ(args[0]))
			return CreateNumObject(TO_STR_OBJ(args[0])->value.size());
		else
			Assert("[Native function 'sizeof']:Expect a array or string argument.");
		return CreateNilObject();
	};

	m_NativeFunctions["insert"] = [this](std::vector<Object *> args) -> Object *
	{
		if (args.empty() || args.size() != 3)
			Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.");

		if (IS_ARRAY_OBJ(args[0]))
		{
			ArrayObject *array = TO_ARRAY_OBJ(args[0]);
			if (!IS_NUM_OBJ(args[1]))
				Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

			int64_t iIndex = TO_NUM_OBJ(args[1])->value;

			if (iIndex < 0 || iIndex >= array->elements.size())
				Assert("[Native function 'insert']:Index out of array's range");

			array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
		}
		else if (IS_STR_OBJ(args[0]))
		{
			StrObject *string = TO_STR_OBJ(args[0]);
			if (!IS_NUM_OBJ(args[1]))
				Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

			int64_t iIndex = TO_NUM_OBJ(args[1])->value;

			if (iIndex < 0 || iIndex >= string->value.size())
				Assert("[Native function 'insert']:Index out of array's range");

			string->value.insert(iIndex, args[2]->Stringify());
		}
		else
			Assert("[Native function 'insert']:Expect a array,table ot string argument.");
		return nullptr;
	};

	m_NativeFunctions["erase"] = [this](std::vector<Object *> args) -> Object *
	{
		if (args.empty() || args.size() != 2)
			Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the corresponding index object.");

		if (IS_ARRAY_OBJ(args[0]))
		{
			ArrayObject *array = TO_ARRAY_OBJ(args[0]);
			if (!IS_NUM_OBJ(args[1]))
				Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

			int64_t iIndex = TO_NUM_OBJ(args[1])->value;

			if (iIndex < 0 || iIndex >= array->elements.size())
				Assert("[Native function 'erase']:Index out of array's range");

			array->elements.erase(array->elements.begin() + iIndex);
		}
		else if (IS_STR_OBJ(args[0]))
		{
			StrObject *string = TO_STR_OBJ(args[0]);
			if (!IS_NUM_OBJ(args[1]))
				Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

			int64_t iIndex = TO_NUM_OBJ(args[1])->value;

			if (iIndex < 0 || iIndex >= string->value.size())
				Assert("[Native function 'erase']:Index out of array's range");

			string->value.erase(string->value.begin() + iIndex);
		}
		else
			Assert("[Native function 'erase']:Expect a array,table ot string argument.");
		return nullptr;
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

NumObject *VM::CreateNumObject(double value)
{
	if (curObjCount == maxObjCount)
		Gc();

	NumObject *object = new NumObject(value);
	object->marked = false;

	object->next = firstObject;
	firstObject = object;

	curObjCount++;

	return object;
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
BoolObject *VM::CreateBoolObject(bool value)
{
	if (curObjCount == maxObjCount)
		Gc();

	BoolObject *object = new BoolObject(value);
	object->marked = false;

	object->next = firstObject;
	firstObject = object;

	curObjCount++;

	return object;
}

NilObject *VM::CreateNilObject()
{
	if (curObjCount == maxObjCount)
		Gc();

	NilObject *object = new NilObject();
	object->marked = false;

	object->next = firstObject;
	firstObject = object;

	curObjCount++;

	return object;
}
ArrayObject *VM::CreateArrayObject(const std::vector<Object *> &elements)
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
									 const std::unordered_map<std::string, Object *> &members)
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

RefObject *VM::CreateRefObject(std::string_view address)
{
	if (curObjCount == maxObjCount)
		Gc();

	RefObject *refObject = new RefObject(address);
	refObject->marked = false;

	refObject->next = firstObject;
	firstObject = refObject;

	curObjCount++;

	return refObject;
}

Object *VM::Execute(Frame frame)
{
	// + - * /
#define COMMON_BINARY(op)                                                                     \
	do                                                                                        \
	{                                                                                         \
		Object *left = PopObject();                                                           \
		Object *right = PopObject();                                                          \
		if (IS_NUM_OBJ(right) && IS_NUM_OBJ(left))                                            \
			PushObject(CreateNumObject(TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value)); \
		else                                                                                  \
			Assert("Invalid binary op:" + left->Stringify() + (#op) + right->Stringify());    \
	} while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                                                        \
	do                                                                                                                            \
	{                                                                                                                             \
		Object *left = PopObject();                                                                                               \
		Object *right = PopObject();                                                                                              \
		if (IS_NUM_OBJ(right) && IS_NUM_OBJ(left))                                                                                \
			PushObject(TO_NUM_OBJ(left)->value op TO_NUM_OBJ(right)->value ? CreateBoolObject(true) : CreateBoolObject(false));   \
		else                                                                                                                      \
			PushObject(CreateBoolObject(false));                                                                                  \
	} while (0);

//and or
#define LOGIC_BINARY(op)                                                                                                               \
	do                                                                                                                                 \
	{                                                                                                                                  \
		Object *left = PopObject();                                                                                                    \
		Object *right = PopObject();                                                                                                   \
		if (IS_BOOL_OBJ(right) && IS_BOOL_OBJ(left))                                                                                   \
			PushObject(((BoolObject *)left)->value op((BoolObject *)right)->value ? CreateBoolObject(true) : CreateBoolObject(false)); \
		else                                                                                                                           \
			Assert("Invalid op:" + left->Stringify() + (#op) + right->Stringify());                                                    \
	} while (0);

	for (size_t ip = 0; ip < frame.m_Codes.size(); ++ip)
	{
		uint8_t instruction = frame.m_Codes[ip];
		switch (instruction)
		{
		case OP_RETURN:
			if(m_Context->m_UpContext)
			{
				Context* tmp=m_Context->m_UpContext;
				delete m_Context;
				m_Context=tmp;
			}
			return PopObject();
			break;
		case OP_NEW_NUM:
			PushObject(CreateNumObject(frame.m_Nums[frame.m_Codes[++ip]]));
			break;
		case OP_NEW_STR:
			PushObject(CreateStrObject(frame.m_Strings[frame.m_Codes[++ip]]));
			break;
		case OP_NEW_TRUE:
			PushObject(CreateBoolObject(true));
			break;
		case OP_NEW_FALSE:
			PushObject(CreateBoolObject(false));
			break;
		case OP_NEW_NIL:
			PushObject(CreateNilObject());
			break;
		case OP_NEG:
		{
			Object *object = PopObject();
			if (IS_NUM_OBJ(object))
				PushObject(CreateNumObject(-TO_NUM_OBJ(object)->value));
			else
				Assert("Invalid op:'-'" + object->Stringify());
			break;
		}
		case OP_NOT:
		{
			Object *object = PopObject();
			if (IS_BOOL_OBJ(object))
				PushObject(CreateBoolObject(!TO_BOOL_OBJ(object)->value));
			else
				Assert("Invalid op:'not' " + object->Stringify());
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
			Object *left = PopObject();
			Object *right = PopObject();
			PushObject(CreateBoolObject(left->IsEqualTo(right)));
			break;
		}
		case OP_NOT_EQUAL:
		{
			Object *left = PopObject();
			Object *right = PopObject();
			PushObject(CreateBoolObject(!left->IsEqualTo(right)));
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
			Object *value = PopObject();
			m_Context->DefineVariableByName(frame.m_Strings[frame.m_Codes[++ip]], value);
			break;
		}
		case OP_SET_VAR:
		{
			std::string name = frame.m_Strings[frame.m_Codes[++ip]];

			Object *value = PopObject();
			Object *variable = m_Context->GetVariableByName(name);

			if (IS_REF_OBJ(variable))
			{
				m_Context->AssignVariableByAddress(TO_REF_OBJ(variable)->address, value);
				TO_REF_OBJ(variable)->address = PointerAddressToString(value); //update ref address
			}
			else
				m_Context->AssignVariableByName(name, value);
			break;
		}
		case OP_GET_VAR:
		{
			std::string name = frame.m_Strings[frame.m_Codes[++ip]];

			Object *varObject = m_Context->GetVariableByName(name);
			//create a struct object
			if (varObject == nullptr)
			{
				if (frame.HasStructFrame(name))
					PushObject(Execute(frame.GetStructFrame(name)));
				else
					Assert("No struct definition:" + name);
			}
			else if (IS_REF_OBJ(varObject))
			{
				varObject = m_Context->GetVariableByAddress(TO_REF_OBJ(varObject)->address);
				PushObject(varObject);
			}
			else
				PushObject(varObject);
			break;
		}
		case OP_NEW_ARRAY:
		{
			std::vector<Object *> elements;
			int64_t arraySize = (int64_t)frame.m_Nums[frame.m_Codes[++ip]];
			for (int64_t i = 0; i < arraySize; ++i)
				elements.insert(elements.begin(), PopObject());
			PushObject(CreateArrayObject(elements));
			break;
		}
		case OP_NEW_STRUCT:
		{
			PushObject(CreateStructObject(frame.m_Strings[frame.m_Codes[++ip]], m_Context->m_Values));
			break;
		}
		case OP_GET_INDEX_VAR:
		{
			Object *index = PopObject();
			Object *object = PopObject();
			if (IS_ARRAY_OBJ(object))
			{
				ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
				if (!IS_NUM_OBJ(index))
					Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

				int64_t iIndex = (int64_t)TO_NUM_OBJ(index)->value;

				if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
					Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

				PushObject(arrayObject->elements[iIndex]);
			}
			else if (IS_STR_OBJ(object))
			{
				StrObject *strObject = TO_STR_OBJ(object);
				if (!IS_NUM_OBJ(index))
					Assert("Invalid index op.The index type of the string object must ba a int num type,but got:" + index->Stringify());

				int64_t iIndex = (int64_t)TO_NUM_OBJ(index)->value;

				if (iIndex < 0 || iIndex >= (int64_t)strObject->value.size())
					Assert("Index out of string range,string size:" + std::to_string(strObject->value.size()) + ",index:" + std::to_string(iIndex));

				PushObject(CreateStrObject(strObject->value.substr(iIndex, 1)));
			}
			else
				Assert("Invalid index op.The indexed object isn't a array or a string object:" + object->Stringify());
			break;
		}
		case OP_SET_INDEX_VAR:
		{
			Object *index = PopObject();
			Object *object = PopObject();
			Object *assigner = PopObject();

			if (IS_ARRAY_OBJ(object))
			{
				ArrayObject *arrayObject = TO_ARRAY_OBJ(object);
				if (!IS_NUM_OBJ(index))
					Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index->Stringify());

				int64_t iIndex = TO_NUM_OBJ(index)->value;

				if (iIndex < 0 || iIndex >= (int64_t)arrayObject->elements.size())
					Assert("Index out of array range,array size:" + std::to_string(arrayObject->elements.size()) + ",index:" + std::to_string(iIndex));

				arrayObject->elements[iIndex] = assigner;
			}
			else
				Assert("Invalid index op.The indexed object isn't a array object:" + object->Stringify());
			break;
		}
		case OP_GET_STRUCT_VAR:
		{
			std::string memberName = frame.m_Strings[frame.m_Codes[++ip]];
			Object *stackTop = PopObject();
			if (!IS_STRUCT_OBJ(stackTop))
				Assert("Not a class object of the callee of:" + memberName);
			StructObject *structObj = TO_STRUCT_OBJ(stackTop);
			PushObject(structObj->GetMember(memberName));
			break;
		}
		case OP_SET_STRUCT_VAR:
		{
			std::string memberName = frame.m_Strings[frame.m_Codes[++ip]];
			Object *stackTop = PopObject();
			if (!IS_STRUCT_OBJ(stackTop))
				Assert("Not a class object of the callee of:" + memberName);
			StructObject *structObj = TO_STRUCT_OBJ(stackTop);

			Object *assigner = PopObject();

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
			bool isJump = !TO_BOOL_OBJ(PopObject())->value;
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
			NumObject *argCount = TO_NUM_OBJ(PopObject());

			if (frame.HasFunctionFrame(fnName))
				PushObject(Execute(frame.GetFunctionFrame(fnName)));
			else if (HasNativeFunction(fnName))
			{
				std::vector<Object *> args;
				for (int64_t i = 0; i < argCount->value; ++i)
					args.insert(args.begin(), PopObject());

				Object *result = GetNativeFunction(fnName)(args);
				if (result)
					PushObject(result);
			}
			else
				Assert("No function:" + fnName);

			break;
		}
		case OP_REF:
		{
			PushObject(CreateRefObject(PointerAddressToString(PopObject())));
			break;
		}
		default:
			break;
		}
	}

	return CreateNilObject();
}

void VM::ResetStatus()
{
	sp = 0;
	firstObject = nullptr;
	curObjCount = 0;
	maxObjCount = INIT_OBJ_NUM_MAX;

	std::array<Object *, STACK_MAX>().swap(m_ObjectStack);

	if (m_Context != nullptr)
	{
		delete m_Context;
		m_Context = nullptr;
	}
	m_Context = new Context();
}

std::function<Object *(std::vector<Object *>)> VM::GetNativeFunction(std::string_view fnName)
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

void VM::PushObject(Object *object)
{
	m_ObjectStack[sp++] = object;
}
Object *VM::PopObject()
{
	return m_ObjectStack[--sp];
}

void VM::Gc()
{
	int objNum = curObjCount;

	//mark all object which in stack;
	for (size_t i = 0; i < sp; ++i)
		m_ObjectStack[i]->Mark();

	//sweep objects which is not reachable
	Object **object = &firstObject;
	while (*object)
	{
		if (!((*object)->marked))
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

	std::cout << "Collected " << objNum - curObjCount << " objects," << curObjCount << " remaining." << std::endl;
}