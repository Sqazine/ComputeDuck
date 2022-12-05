#include "VM.h"
#include <iostream>
#include "BuiltinManager.h"
VM::VM()
{
    ResetStatus();
}
VM::~VM()
{
    m_StackTop = m_ValueStack;
    Gc(true);
}

void VM::Run(const Chunk &chunk)
{
    ResetStatus();

    auto mainFn = CreateObject<FunctionObject>(chunk.opCodes);
    auto mainCallFrame = CallFrame(mainFn, 0);
    mainCallFrame.slot = m_StackTop;

    m_CallFrames[0] = mainCallFrame;
    m_CallFrameTop = m_CallFrames;
    m_CallFrameTop++;

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
        if (IS_BUILTIN_VARIABLE_VALUE(left))                                             \
            left = TO_BUILTIN_VARIABLE_VALUE(left)->value;                               \
        if (IS_BUILTIN_VARIABLE_VALUE(right))                                            \
            right = TO_BUILTIN_VARIABLE_VALUE(right)->value;                             \
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
        if (IS_BUILTIN_VARIABLE_VALUE(left))                                              \
            left = TO_BUILTIN_VARIABLE_VALUE(left)->value;                                \
        if (IS_BUILTIN_VARIABLE_VALUE(right))                                             \
            right = TO_BUILTIN_VARIABLE_VALUE(right)->value;                              \
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
        if (IS_BUILTIN_VARIABLE_VALUE(left))                                                \
            left = TO_BUILTIN_VARIABLE_VALUE(left)->value;                                  \
        if (IS_BUILTIN_VARIABLE_VALUE(right))                                               \
            right = TO_BUILTIN_VARIABLE_VALUE(right)->value;                                \
        if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                    \
            Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                                \
            Assert("Invalid op:" + left.Stringify() + (#op) + right.Stringify());           \
    } while (0);

    while (1)
    {
        auto frame = m_CallFrameTop - 1;

        int32_t instruction = *frame->ip++;
        switch (instruction)
        {
        case OP_RETURN:
        {
            auto returnCount = *frame->ip++;
            Value value;
            if (returnCount == 1)
                value = Pop();

            auto callFrame = PopCallFrame();

            if (m_CallFrameTop == m_CallFrames)
                return;

            frame = m_CallFrameTop - 1;

            m_StackTop = callFrame->slot - 1;

            Push(value);
            break;
        }
        case OP_CONSTANT:
        {
            auto idx = *frame->ip++;
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
            if (IS_BUILTIN_VARIABLE_VALUE(left))
                left = TO_BUILTIN_VARIABLE_VALUE(left)->value;
            if (IS_BUILTIN_VARIABLE_VALUE(right))
                right = TO_BUILTIN_VARIABLE_VALUE(right)->value;
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
            if (IS_BUILTIN_VARIABLE_VALUE(left))
                left = TO_BUILTIN_VARIABLE_VALUE(left)->value;
            if (IS_BUILTIN_VARIABLE_VALUE(right))
                right = TO_BUILTIN_VARIABLE_VALUE(right)->value;
            Push(left == right);
            break;
        }
        case OP_NOT:
        {
            auto value = Pop();
            if (IS_REF_VALUE(value))
                value = *TO_REF_VALUE(value)->pointer;
            if (IS_BUILTIN_VARIABLE_VALUE(value))
                value = TO_BUILTIN_VARIABLE_VALUE(value)->value;
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
            if (IS_BUILTIN_VARIABLE_VALUE(value))
                value = TO_BUILTIN_VARIABLE_VALUE(value)->value;
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
            auto numElements = *frame->ip++;

            m_StackTop -= numElements;

            auto elements = std::vector<Value>(numElements);

            int32_t i = 0;
            for (Value *p = m_StackTop; p < m_StackTop + numElements && i < numElements; ++p, ++i)
                elements[i] = *p;
            auto array = CreateObject<ArrayObject>(elements);

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
            auto address = *frame->ip++;
            auto value = Pop();
            if (!IS_BOOL_VALUE(value))
                Assert("The if condition not a boolean value");
            if (!TO_BOOL_VALUE(value))
                frame->ip = frame->fn->opCodes.data() + address + 1;

            break;
        }
        case OP_JUMP:
        {
            auto address = *frame->ip++;
            frame->ip = frame->fn->opCodes.data() + address + 1;
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = *frame->ip++;
            auto value = Pop();
            if (IS_REF_VALUE(m_GlobalVariables[index])) //if is a reference object,then set the actual value which the reference object points
                *TO_REF_VALUE(m_GlobalVariables[index])->pointer = value;
            else
                m_GlobalVariables[index] = value;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *frame->ip++;
            Push(m_GlobalVariables[index]);
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = *frame->ip++;

            auto value = *(m_StackTop - argCount - 1);
            if (IS_FUNCTION_VALUE(value))
            {
                auto fn = TO_FUNCTION_VALUE(value);

                if (argCount != fn->parameterCount)
                    Assert("Non matching function parameters for calling arguments,parameter count:" + std::to_string(fn->parameterCount) + ",argument count:" + std::to_string(argCount));

                auto callFrame = CallFrame(fn, m_StackTop - argCount);

                *m_CallFrameTop++ = callFrame;

                m_StackTop = callFrame.slot + fn->localVarCount;
            }
            else if (IS_BUILTIN_FUNCTION_VALUE(value))
            {
                auto builtin = TO_BUILTIN_FUNCTION_VALUE(value);

                int32_t j = 0;
                Value *slot = m_StackTop - argCount;

                m_StackTop -= (argCount + 1);

                Value returnValue;
                bool hasReturnValue = builtin->fn(slot,argCount, returnValue);
                if (hasReturnValue)
                {
                    RegisterToGCRecordChain(returnValue);
                    Push(returnValue);
                }
            }
            else
                Assert("Calling not a function or a builtinFn");

            break;
        }
        case OP_SET_LOCAL:
        {
            auto isInUpScope = *frame->ip++;
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;
            auto value = Pop();

            Value *slot = nullptr;

            if (isInUpScope == 0)
                slot = frame->slot + index;
            else
                slot = PeekCallFrame(scopeDepth)->slot + index;

            if (IS_REF_VALUE((*slot)))
                *TO_REF_VALUE((*slot))->pointer = value;
            else
                *slot = value;
            break;
        }
        case OP_GET_LOCAL:
        {
            auto isInUpScope = *frame->ip++;
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;

            Value *slot = nullptr;

            if (isInUpScope == 0)
                slot = (frame->slot + index);
            else
                slot = (PeekCallFrame(scopeDepth)->slot + index);

            Push(*slot);
            break;
        }
        case OP_SP_OFFSET:
        {
            auto offset = *frame->ip++;
            m_StackTop += offset;
            break;
        }
        case OP_GET_BUILTIN_FUNCTION:
        {
            auto idx = *frame->ip++;
            auto builtinObj = BuiltinManager::m_BuiltinFunctions[idx];
            Push(builtinObj);
            break;
        }
        case OP_GET_BUILTIN_VARIABLE:
        {
            auto idx = *frame->ip++;
            auto builtinObj = BuiltinManager::m_BuiltinVariables[idx];
            Push(builtinObj);
            break;
        }
        case OP_STRUCT:
        {
            std::unordered_map<std::string, Value> members;
            auto memberCount = *frame->ip++;

            auto tmpPtr = m_StackTop; //save the locale,to avoid gc system delete the tmp object before finish the struct instance creation

            for (int i = 0; i < memberCount; ++i)
            {
                auto name = TO_STR_VALUE((*--tmpPtr))->value;
                auto value = *--tmpPtr;
                members[name] = value;
            }

            auto structInstance = CreateObject<StructObject>(members);
            m_StackTop = tmpPtr; //recover the locale
            Push(structInstance);
            break;
        }
        case OP_GET_STRUCT:
        {
            auto memberName = Pop();
            auto instance = Pop();
            if (IS_REF_VALUE(instance))
                instance = *TO_REF_VALUE(instance)->pointer;
            auto structInstance = TO_STRUCT_VALUE(instance);
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
            auto structInstance = TO_STRUCT_VALUE(instance);
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
            auto index = *frame->ip++;
            Push(CreateObject<RefObject>(&m_GlobalVariables[index]));
            break;
        }
        case OP_REF_LOCAL:
        {
            auto isInUpScope = *frame->ip++;
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;

            Value *slot = nullptr;

            if (isInUpScope == 0)
                slot = frame->slot + index;
            else
                slot = PeekCallFrame(scopeDepth)->slot + index;

            Push(CreateObject<RefObject>(slot));
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            auto index = *frame->ip++;
            auto idxValue = Pop();
            if (IS_ARRAY_VALUE(m_GlobalVariables[index]))
            {
                if (!IS_NUM_VALUE(idxValue))
                    Assert("Invalid idx for array,only integer is available.");
                auto intIdx = TO_NUM_VALUE(idxValue);
                if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE(m_GlobalVariables[index])->elements.size())
                    Assert("Idx out of range.");
                Push(CreateObject<RefObject>(&(TO_ARRAY_VALUE(m_GlobalVariables[index])->elements[intIdx])));
            }
            else
                Assert("Invalid indexed reference type:" + m_GlobalVariables[index].Stringify() + " not a table or array value.");
            break;
        }
         case OP_REF_INDEX_LOCAL:
        {
            auto isInUpScope = *frame->ip++;
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;

            auto idxValue = Pop();

            Value *slot = nullptr;
            if (isInUpScope == 0)
                slot = frame->slot + index;
            else
                slot = PeekCallFrame(scopeDepth)->slot + index;

            if (IS_ARRAY_VALUE((*slot)))
            {
                if (!IS_NUM_VALUE(idxValue))
                    Assert("Invalid idx for array,only integer is available.");
                auto intIdx = TO_NUM_VALUE(idxValue);
                if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE((*slot))->elements.size())
                    Assert("Idx out of range.");
                Push(CreateObject<RefObject>(&(TO_ARRAY_VALUE((*slot))->elements[intIdx])));
            }
            else
                Assert("Invalid indexed reference type:" + slot->Stringify() + " not a table or array value.");
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
        if (m_CurObjCount >= m_MaxObjCount)
            Gc();

        object->marked = false;

        object->next = m_FirstObject;
        m_FirstObject = object;

        m_CurObjCount++;
    }
}

void VM::ResetStatus()
{
    m_StackTop = m_ValueStack;
    m_FirstObject = nullptr;
    m_CurObjCount = 0;
    m_MaxObjCount = INITIAL_GC_THRESHOLD;

    for (int32_t i = 0; i < STACK_MAX; ++i)
        m_ValueStack[i] = Value();

    for (int32_t i = 0; i < GLOBAL_VARIABLE_MAX; ++i)
        m_GlobalVariables[i] = Value();

    for (int32_t i = 0; i < STACK_MAX; ++i)
        m_CallFrames[i] = CallFrame();
}

void VM::Push(const Value &value)
{
    *m_StackTop++ = value;
}

const Value &VM::Pop()
{
    return *(--m_StackTop);
}

CallFrame *VM::PopCallFrame()
{
    return --m_CallFrameTop;
}

CallFrame *VM::PeekCallFrame(int32_t distance)
{
    return m_CallFrameTop - distance;
}

void VM::Gc(bool isExitingVM)
{
    int objNum = m_CurObjCount;
    if (!isExitingVM)
    {
        //mark all object which in stack and in context
        for (Value *slot = m_ValueStack; slot < m_StackTop; ++slot)
            slot->Mark();
        for (const auto &c : m_Constants)
            c.Mark();
        for (auto &g : m_GlobalVariables)
            g.Mark();
        for (CallFrame *slot = m_CallFrames; slot < m_CallFrameTop; ++slot)
            slot->fn->Mark();
    }
    else
    {
        //unMark all objects while exiting vm
        for (Value *slot = m_ValueStack; slot < m_StackTop; ++slot)
            slot->UnMark();
        for (const auto &c : m_Constants)
            c.UnMark();
        for (auto &g : m_GlobalVariables)
            g.UnMark();
        for (CallFrame *slot = m_CallFrames; slot < m_CallFrameTop; ++slot)
            slot->fn->UnMark();
    }

    //sweep objects which is not reachable
    Object **object = &m_FirstObject;
    while (*object)
    {
        if (!(*object)->marked)
        {
            Object *unreached = *object;
            *object = unreached->next;

            delete unreached;
            unreached = nullptr;
            m_CurObjCount--;
        }
        else
        {
            (*object)->marked = false;
            object = &(*object)->next;
        }
    }

    m_MaxObjCount = m_CurObjCount == 0 ? INITIAL_GC_THRESHOLD : m_CurObjCount * 2;

    std::cout << "Collected " << objNum - m_CurObjCount << " objects," << m_CurObjCount << " remaining." << std::endl;
}