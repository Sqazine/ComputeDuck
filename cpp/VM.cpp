#include "VM.h"
#include <iostream>
#include <ctime>

VM::VM()
{
    ResetStatus();

    m_Builtins.emplace_back(CreateObject<BuiltinObject>("print", [this](const std::vector<Value> &args, Value &result) -> bool
                                                        {
                                                            if (args.empty())
                                                                return false;

                                                            std::cout << args[0].Stringify();
                                                            return false;
                                                        }));

    m_Builtins.emplace_back(CreateObject<BuiltinObject>("println", [this](const std::vector<Value> &args, Value &result) -> bool
                                                        {
                                                            if (args.empty())
                                                                return false;

                                                            std::cout << args[0].Stringify() << std::endl;
                                                            return false;
                                                        }));

    m_Builtins.emplace_back(CreateObject<BuiltinObject>("sizeof", [this](const std::vector<Value> &args, Value &result) -> bool
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
                                                        }));

    m_Builtins.emplace_back(CreateObject<BuiltinObject>("insert", [this](const std::vector<Value> &args, Value &result) -> bool
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
                                                        }));

    m_Builtins.emplace_back(CreateObject<BuiltinObject>("erase", [this](const std::vector<Value> &args, Value &result) -> bool
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
                                                        }));

    m_Builtins.emplace_back(CreateObject<BuiltinObject>("clock", [this](const std::vector<Value> &args, Value &result) -> bool
                                                        {
                                                            result = Value((double)clock() / CLOCKS_PER_SEC);
                                                            return true;
                                                        }));
}
VM::~VM()
{
    sp = 0;
    Gc(true);
}

void VM::Run(const Chunk &chunk)
{
    ResetStatus();

    auto mainFn = CreateObject<FunctionObject>(chunk.opCodes);
    auto mainCallFrame = CallFrame(mainFn, 0);

    m_CallFrames[0] = mainCallFrame;
    m_CallFrameIndex = 1;

    for (int32_t i = 0; i < chunk.constantCount; ++i)
        m_Constants[i] = chunk.constants[i];

    Execute();
}

void VM::Execute()
{
    //  - * /
#define COMMON_BINARY(op)                                                                \
    do                                                                                   \
    {                                                                                    \
        Value left = Pop();                                                              \
        Value right = Pop();                                                             \
        if (IS_REF_VALUE(left))                                                          \
            left = *TO_REF_VALUE(left)->pointer;                                         \
        if (IS_REF_VALUE(right))                                                         \
            right = *TO_REF_VALUE(right)->pointer;                                       \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                   \
            Push(TO_NUM_VALUE(left) op TO_NUM_VALUE(right));                             \
        else                                                                             \
            Assert("Invalid binary op:" + left.Stringify() + (#op) + right.Stringify()); \
    } while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                \
    do                                                                                    \
    {                                                                                     \
        Value left = Pop();                                                               \
        Value right = Pop();                                                              \
        if (IS_REF_VALUE(left))                                                           \
            left = *TO_REF_VALUE(left)->pointer;                                          \
        if (IS_REF_VALUE(right))                                                          \
            right = *TO_REF_VALUE(right)->pointer;                                        \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                    \
            Push(TO_NUM_VALUE(left) op TO_NUM_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                              \
            Push(false);                                                                  \
    } while (0);

//and or
#define LOGIC_BINARY(op)                                                                    \
    do                                                                                      \
    {                                                                                       \
        Value left = Pop();                                                                 \
        Value right = Pop();                                                                \
        if (IS_REF_VALUE(left))                                                             \
            left = *TO_REF_VALUE(left)->pointer;                                            \
        if (IS_REF_VALUE(right))                                                            \
            right = *TO_REF_VALUE(right)->pointer;                                          \
        if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                    \
            Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                                \
            Assert("Invalid op:" + left.Stringify() + (#op) + right.Stringify());           \
    } while (0);

    while (CurCallFrame().ip < (int32_t)CurCallFrame().GetOpCodes().size() - 1)
    {
        CurCallFrame().ip++;
        int32_t &ip = CurCallFrame().ip;
        int32_t instruction = CurCallFrame().GetOpCodes()[ip];
        switch (instruction)
        {
        case OP_RETURN:
        {
            auto returnCount = CurCallFrame().GetOpCodes()[++ip];
            Value value;
            if (returnCount == 1)
                value = Pop();

            auto callFrame = PopCallFrame();
            sp = callFrame.basePtr - 1;

            Push(value);
            break;
        }
        case OP_CONSTANT:
        {
            auto idx = CurCallFrame().GetOpCodes()[++ip];
            auto value = m_Constants[idx];

            RegisterToGCRecordChain(value); //the value in constant list maybe not regisiter to the gc chain

            Push(value);
            break;
        }
        case OP_ADD:
        {
            Value left = Pop();
            Value right = Pop();
            if (IS_REF_VALUE(left))
                left = *TO_REF_VALUE(left)->pointer;
            if (IS_REF_VALUE(right))
                right = *TO_REF_VALUE(right)->pointer;
            if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))
                Push(TO_NUM_VALUE(left) + TO_NUM_VALUE(right));
            else if (IS_STR_VALUE(right) && IS_STR_VALUE(left))
                Push(CreateObject<StrObject>(TO_STR_VALUE(left)->value + TO_STR_VALUE(right)->value));
            else
                Assert("Invalid binary op:" + left.Stringify() + " + " + right.Stringify());
            break;
        }
        case OP_SUB:
        {
            COMMON_BINARY(-);
            break;
        }
        case OP_MUL:
        {
            COMMON_BINARY(*);
            break;
        }
        case OP_DIV:
        {
            COMMON_BINARY(/);
            break;
        }
        case OP_GREATER:
        {
            COMPARE_BINARY(>);
            break;
        }
        case OP_LESS:
        {
            COMPARE_BINARY(<);
            break;
        }
        case OP_EQUAL:
        {
            Value left = Pop();
            Value right = Pop();
            if (IS_REF_VALUE(left))
                left = *TO_REF_VALUE(left)->pointer;
            if (IS_REF_VALUE(right))
                right = *TO_REF_VALUE(right)->pointer;
            Push(left == right);
            break;
        }
        case OP_NOT:
        {
            auto value = Pop();
            if (IS_REF_VALUE(value))
                value = *TO_REF_VALUE(value)->pointer;
            if (!IS_BOOL_VALUE(value))
                Assert("Not a boolean value of the value:" + value.Stringify());
            Push(!TO_BOOL_VALUE(value));
            break;
        }
        case OP_MINUS:
        {
            auto value = Pop();
            if (IS_REF_VALUE(value))
                value = *TO_REF_VALUE(value)->pointer;
            if (!IS_NUM_VALUE(value))
                Assert("Not a valid op:'-'" + value.Stringify());
            Push(-TO_NUM_VALUE(value));
            break;
        }
        case OP_AND:
        {
            LOGIC_BINARY(&&);
            break;
        }
        case OP_OR:
        {
            LOGIC_BINARY(||);
            break;
        }
        case OP_ARRAY:
        {
            auto numElements = CurCallFrame().GetOpCodes()[++ip];

            auto startIndex = sp - numElements;
            auto endIndex = sp;
            auto elements = std::vector<Value>(numElements);

            for (int32_t i = startIndex; i < endIndex; ++i)
                elements[i - startIndex] = m_ValueStack[i];
            auto array = CreateObject<ArrayObject>(elements);
            sp -= numElements;

            Push(array);
            break;
        }
        case OP_INDEX:
        {
            auto index = Pop();
            auto ds = Pop();

            if (IS_ARRAY_VALUE(ds) && IS_NUM_VALUE(index))
            {
                auto array = TO_ARRAY_VALUE(ds);
                auto i = (size_t)TO_NUM_VALUE(index);
                if (i < 0 || i >= array->elements.size())
                    Push(Value());
                else
                    Push(array->elements[i]);
            }
            else
                Assert("Invalid index op:" + ds.Stringify() + "[" + index.Stringify() + "]");
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = CurCallFrame().GetOpCodes()[++ip];
            auto value = Pop();
            if (!IS_BOOL_VALUE(value))
                Assert("The if condition not a boolean value");
            if (!TO_BOOL_VALUE(value))
                ip = address;

            break;
        }
        case OP_JUMP:
        {
            auto address = CurCallFrame().GetOpCodes()[++ip];
            ip = address;
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = CurCallFrame().GetOpCodes()[++ip];
            auto value = Pop();
            if (IS_REF_VALUE(m_GlobalVariables[index])) //if is a reference object,then set the actual value which the reference object points
                *TO_REF_VALUE(m_GlobalVariables[index])->pointer = value;
            else
                m_GlobalVariables[index] = value;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = CurCallFrame().GetOpCodes()[++ip];
            Push(m_GlobalVariables[index]);
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = CurCallFrame().GetOpCodes()[++ip];

            auto value = m_ValueStack[sp - 1 - argCount];
            if (IS_FUNCTION_VALUE(value))
            {
                auto fn = TO_FUNCTION_VALUE(value);

                if (argCount != fn->parameterCount)
                    Assert("Non matching function parameters for calling arguments,parameter count:" + std::to_string(fn->parameterCount) + ",argument count:" + std::to_string(argCount));

                auto callFrame = CallFrame(fn, sp - argCount);
                PushCallFrame(callFrame);
                sp = callFrame.basePtr + fn->localVarCount;
            }
            else if (IS_BUILTIN_VALUE(value))
            {
                auto builtin = TO_BUILTIN_VALUE(value);

                std::vector<Value> args(argCount);

                for (int32_t i = sp - argCount, j = 0; i < sp && j < argCount; ++i, ++j)
                    args[j] = m_ValueStack[i];

                sp = sp - argCount - 1;

                Value returnValue;
                bool hasReturnValue = builtin->fn(args, returnValue);
                if (hasReturnValue)
                    Push(returnValue);
            }
            else
                Assert("Calling not a function or a builtinFn");

            break;
        }
        case OP_SET_LOCAL:
        {
            auto isInUpScope = CurCallFrame().GetOpCodes()[++ip];
            auto scopeDepth = CurCallFrame().GetOpCodes()[++ip];
            auto index = CurCallFrame().GetOpCodes()[++ip];
            auto value = Pop();

            auto fullIdx = 0;

            if (isInUpScope == 0)
                fullIdx = CurCallFrame().basePtr + index;
            else
                fullIdx = PeekCallFrame(scopeDepth).basePtr + index;
            if (IS_REF_VALUE(m_ValueStack[fullIdx]))
                *TO_REF_VALUE(m_ValueStack[fullIdx])->pointer = value;
            else
                m_ValueStack[fullIdx] = value;
            break;
        }
        case OP_GET_LOCAL:
        {
            auto isInUpScope = CurCallFrame().GetOpCodes()[++ip];
            auto scopeDepth = CurCallFrame().GetOpCodes()[++ip];
            auto index = CurCallFrame().GetOpCodes()[++ip];

            auto fullIdx = 0;

            if (isInUpScope == 0)
                fullIdx = CurCallFrame().basePtr + index;
            else
                fullIdx = PeekCallFrame(scopeDepth).basePtr + index;

            Push(m_ValueStack[fullIdx]);
            break;
        }
        case OP_SP_OFFSET:
        {
            auto offset = CurCallFrame().GetOpCodes()[++ip];
            sp += offset;
            break;
        }
        case OP_GET_BUILTIN:
        {
            auto idx = CurCallFrame().GetOpCodes()[++ip];
            auto builtinObj = m_Builtins[idx];
            Push(builtinObj);
            break;
        }
        case OP_GET_CURRENT_FUNCTION:
        {
            Push(CurCallFrame().fn);
            break;
        }
        case OP_STRUCT:
        {
            std::unordered_map<std::string, Value> members;
            auto memberCount = CurCallFrame().GetOpCodes()[++ip];

            auto tmpPtr = sp; //save the locale,to avoid gc system delete the tmp object before finish the struct instance creation

            for (int i = 0; i < memberCount; ++i)
            {
                auto name = TO_STR_VALUE(m_ValueStack[--tmpPtr])->value;
                auto value = m_ValueStack[--tmpPtr];
                members[name] = value;
            }

            auto structInstance = CreateObject<StructInstanceObject>(members);
            sp = tmpPtr; //recover the locale
            Push(structInstance);
            break;
        }
        case OP_GET_STRUCT:
        {
            auto memberName = Pop();
            auto instance = Pop();
            if (IS_REF_VALUE(instance))
                instance = *TO_REF_VALUE(instance)->pointer;
            auto structInstance = TO_STRUCT_INSTANCE_VALUE(instance);
            if (IS_STR_VALUE(memberName))
            {
                auto iter = structInstance->members.find(TO_STR_VALUE(memberName)->value);
                if (iter == structInstance->members.end())
                    Assert("no member named:(" + memberName.Stringify() + ") in struct instance:" + structInstance->Stringify());
                Push(iter->second);
            }
            break;
        }
        case OP_SET_STRUCT:
        {
            auto memberName = Pop();
            auto instance = Pop();
            if (IS_REF_VALUE(instance))
                instance = *TO_REF_VALUE(instance)->pointer;
            auto structInstance = TO_STRUCT_INSTANCE_VALUE(instance);
            auto value = Pop();
            if (IS_STR_VALUE(memberName))
            {
                auto iter = structInstance->members.find(TO_STR_VALUE(memberName)->value);
                if (iter == structInstance->members.end())
                    Assert("no member named:(" + memberName.Stringify() + ") in struct instance:(0x" + PointerAddressToString(structInstance) + ")");
                structInstance->members[TO_STR_VALUE(memberName)->value] = value;
            }
            break;
        }
        case OP_REF_GLOBAL:
        {
            auto index = CurCallFrame().GetOpCodes()[++ip];
            Push(CreateObject<RefObject>(&m_GlobalVariables[index]));
            break;
        }
        case OP_REF_LOCAL:
        {
            auto isInUpScope = CurCallFrame().GetOpCodes()[++ip];
            auto scopeDepth = CurCallFrame().GetOpCodes()[++ip];
            auto index = CurCallFrame().GetOpCodes()[++ip];

            auto fullIdx = 0;

            if (isInUpScope == 0)
                fullIdx = CurCallFrame().basePtr + index;
            else
                fullIdx = PeekCallFrame(scopeDepth).basePtr + index;

            Push(CreateObject<RefObject>(&m_ValueStack[fullIdx]));
            break;
        }
        default:
            break;
        }
    }
}

void VM::RegisterToGCRecordChain(const Value &value)
{
    if (IS_OBJECT_VALUE(value) && TO_OBJECT_VALUE(value)->next == nullptr) //check is null to avoid cross-reference in vm's object record chain
    {
        auto object = TO_OBJECT_VALUE(value);
        if (curObjCount >= maxObjCount)
            Gc();

        object->marked = false;

        object->next = firstObject;
        firstObject = object;

        curObjCount++;
    }
}

void VM::ResetStatus()
{
    sp = 0;
    firstObject = nullptr;
    curObjCount = 0;
    maxObjCount = INITIAL_GC_THRESHOLD;

    for (int32_t i = 0; i < STACK_MAX; ++i)
        m_ValueStack[i] = Value();

    for (int32_t i = 0; i < GLOBAL_VARIABLE_MAX; ++i)
        m_GlobalVariables[i] = Value();

    for (int32_t i = 0; i < STACK_MAX; ++i)
        m_CallFrames[i] = CallFrame();
}

void VM::Push(const Value &value)
{
    m_ValueStack[sp++] = value;
}

const Value &VM::Pop()
{
    return m_ValueStack[--sp];
}

CallFrame &VM::CurCallFrame()
{
    return m_CallFrames[m_CallFrameIndex - 1];
}
void VM::PushCallFrame(const CallFrame &callFrame)
{
    m_CallFrames[m_CallFrameIndex++] = callFrame;
}
const CallFrame &VM::PopCallFrame()
{
    return m_CallFrames[--m_CallFrameIndex];
}

CallFrame &VM::PeekCallFrame(int32_t distance)
{
    return m_CallFrames[distance];
}

void VM::Gc(bool isExitingVM)
{
    int objNum = curObjCount;
    if (!isExitingVM)
    {
        //mark all object which in stack and in context
        for (size_t i = 0; i < sp; ++i)
            m_ValueStack[i].Mark();
        for (const auto &builtin : m_Builtins)
            builtin->Mark();
        for (const auto &c : m_Constants)
            c.Mark();
        for (auto &g : m_GlobalVariables)
            g.Mark();
        for (int32_t i = 0; i < m_CallFrameIndex; ++i)
            m_CallFrames[i].fn->Mark();
    }
    else
    {
        //unMark all objects while exiting vm
        for (size_t i = 0; i < sp; ++i)
            m_ValueStack[i].UnMark();
        for (const auto &builtin : m_Builtins)
            builtin->UnMark();
        for (const auto &c : m_Constants)
            c.UnMark();
        for (auto &g : m_GlobalVariables)
            g.UnMark();
        for (int32_t i = 0; i < m_CallFrameIndex; ++i)
            m_CallFrames[i].fn->UnMark();
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