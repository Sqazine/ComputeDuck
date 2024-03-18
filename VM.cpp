#include "VM.h"
#include <iostream>
#include "BuiltinManager.h"
VM::VM()
{
    ResetStatus();
}
VM::~VM()
{
    Gc(true);
}

void VM::Run(Chunk *chunk)
{
    ResetStatus();

    auto mainFn = CreateObject<FunctionObject>(chunk->opCodes);
    auto mainCallFrame = CallFrame(mainFn, 0);
    mainCallFrame.slot = m_StackTop;

    m_CallFrames[0] = mainCallFrame;
    m_CallFrameTop = m_CallFrames;
    m_CallFrameTop++;

    m_Chunk = chunk;

    Execute();
}

void VM::Execute()
{
    //  - * /
#define COMMON_BINARY(op)                                                                                    \
    do                                                                                                       \
    {                                                                                                        \
        Value left = Pop();                                                                                  \
        Value right = Pop();                                                                                 \
        while (IS_REF_VALUE(left))                                                                           \
            left = *TO_REF_VALUE(left)->pointer;                                                             \
        while (IS_REF_VALUE(right))                                                                          \
            right = *TO_REF_VALUE(right)->pointer;                                                           \
        if (IS_BUILTIN_VALUE(left) && TO_BUILTIN_VALUE(left)->IsBuiltinData())                               \
            left = TO_BUILTIN_VALUE(left)->GetBuiltinValue();                                                \
        if (IS_BUILTIN_VALUE(right) && TO_BUILTIN_VALUE(right)->IsBuiltinData())                             \
            right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();                                              \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                       \
            Push(TO_NUM_VALUE(left) op TO_NUM_VALUE(right));                                                 \
        else                                                                                                 \
            ASSERT("Invalid binary op:%s%s", (left.Stringify() + (#op)).c_str(), right.Stringify().c_str()); \
    } while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                \
    do                                                                                    \
    {                                                                                     \
        Value left = Pop();                                                               \
        Value right = Pop();                                                              \
        while (IS_REF_VALUE(left))                                                        \
            left = *TO_REF_VALUE(left)->pointer;                                          \
        while (IS_REF_VALUE(right))                                                       \
            right = *TO_REF_VALUE(right)->pointer;                                        \
        if (IS_BUILTIN_VALUE(left) && TO_BUILTIN_VALUE(left)->IsBuiltinData())            \
            left = TO_BUILTIN_VALUE(left)->GetBuiltinValue();                             \
        if (IS_BUILTIN_VALUE(right) && TO_BUILTIN_VALUE(right)->IsBuiltinData())          \
            right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();                           \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                    \
            Push(TO_NUM_VALUE(left) op TO_NUM_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                              \
            Push(false);                                                                  \
    } while (0);

// and or
#define LOGIC_BINARY(op)                                                                              \
    do                                                                                                \
    {                                                                                                 \
        Value left = Pop();                                                                           \
        Value right = Pop();                                                                          \
        while (IS_REF_VALUE(left))                                                                    \
            left = *TO_REF_VALUE(left)->pointer;                                                      \
        while (IS_REF_VALUE(right))                                                                   \
            right = *TO_REF_VALUE(right)->pointer;                                                    \
        if (IS_BUILTIN_VALUE(left) && TO_BUILTIN_VALUE(left)->IsBuiltinData())                        \
            left = TO_BUILTIN_VALUE(left)->GetBuiltinValue();                                         \
        if (IS_BUILTIN_VALUE(right) && TO_BUILTIN_VALUE(right)->IsBuiltinData())                      \
            right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();                                       \
        if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                              \
            Push(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false));           \
        else                                                                                          \
            ASSERT("Invalid op:%s%s", (left.Stringify() + (#op)).c_str(), right.Stringify().c_str()); \
    } while (0);

#define BIT_BINARY(op)                                                                           \
    do                                                                                           \
    {                                                                                            \
        Value left = Pop();                                                                      \
        Value right = Pop();                                                                     \
        while (IS_REF_VALUE(left))                                                               \
            left = *TO_REF_VALUE(left)->pointer;                                                 \
        while (IS_REF_VALUE(right))                                                              \
            right = *TO_REF_VALUE(right)->pointer;                                               \
        if (IS_BUILTIN_VALUE(left))                                                              \
            left = TO_BUILTIN_VALUE(left)->GetBuiltinValue();                                    \
        if (IS_BUILTIN_VALUE(right))                                                             \
            right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();                                  \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                           \
            Push((uint64_t)TO_NUM_VALUE(left) op(uint64_t) TO_NUM_VALUE(right));                 \
        else                                                                                     \
            ASSERT("Invalid op:%s(#op)%s", left.Stringify().c_str(), right.Stringify().c_str()); \
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
            auto value = m_Chunk->constants[idx];

            RegisterToGCRecordChain(value); // the value in constant list maybe not regisiter to the gc chain

            Push(value);
            break;
        }
        case OP_ADD:
        {
            Value left = Pop();
            Value right = Pop();
            while (IS_REF_VALUE(left))
                left = *TO_REF_VALUE(left)->pointer;
            while (IS_REF_VALUE(right))
                right = *TO_REF_VALUE(right)->pointer;
            if (IS_BUILTIN_VALUE(left) && TO_BUILTIN_VALUE(left)->IsBuiltinData())
                left = TO_BUILTIN_VALUE(left)->GetBuiltinValue();
            if (IS_BUILTIN_VALUE(right) && TO_BUILTIN_VALUE(right)->IsBuiltinData())
                right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();
            if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))
                Push(TO_NUM_VALUE(left) + TO_NUM_VALUE(right));
            else if (IS_STR_VALUE(right) && IS_STR_VALUE(left))
                Push(CreateObject<StrObject>(TO_STR_VALUE(left)->value + TO_STR_VALUE(right)->value));
            else
                ASSERT("Invalid binary op:%s+%s", left.Stringify().c_str(), right.Stringify().c_str());
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
            while (IS_REF_VALUE(left))
                left = *TO_REF_VALUE(left)->pointer;
            while (IS_REF_VALUE(right))
                right = *TO_REF_VALUE(right)->pointer;
            if (IS_BUILTIN_VALUE(left) && TO_BUILTIN_VALUE(left)->IsBuiltinData())
                left = TO_BUILTIN_VALUE(left)->GetBuiltinValue();
            if (IS_BUILTIN_VALUE(right) && TO_BUILTIN_VALUE(right)->IsBuiltinData())
                right = TO_BUILTIN_VALUE(right)->GetBuiltinValue();
            Push(left == right);
            break;
        }
        case OP_NOT:
        {
            auto value = Pop();
            while (IS_REF_VALUE(value))
                value = *TO_REF_VALUE(value)->pointer;
            if (IS_BUILTIN_VALUE(value) && TO_BUILTIN_VALUE(value)->IsBuiltinData())
                value = TO_BUILTIN_VALUE(value)->GetBuiltinValue();
            if (!IS_BOOL_VALUE(value))
                ASSERT("Not a boolean value of the value: %s", value.Stringify().c_str());
            Push(!TO_BOOL_VALUE(value));
            break;
        }
        case OP_MINUS:
        {
            auto value = Pop();
            while (IS_REF_VALUE(value))
                value = *TO_REF_VALUE(value)->pointer;
            if (IS_BUILTIN_VALUE(value) && TO_BUILTIN_VALUE(value)->IsBuiltinData())
                value = TO_BUILTIN_VALUE(value)->GetBuiltinValue();
            if (!IS_NUM_VALUE(value))
                ASSERT("Not a valid op:'-' %s", value.Stringify().c_str());
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
        case OP_BIT_AND:
        {
            BIT_BINARY(&);
            break;
        }
        case OP_BIT_OR:
        {
            BIT_BINARY(|);
            break;
        }
        case OP_BIT_XOR:
        {
            BIT_BINARY(^);
            break;
        }
        case OP_BIT_NOT:
        {
            Value value = Pop();

            while (IS_REF_VALUE(value))
                value = *TO_REF_VALUE(value)->pointer;
            if (IS_BUILTIN_VALUE(value) && TO_BUILTIN_VALUE(value)->IsBuiltinData())
                value = TO_BUILTIN_VALUE(value)->GetBuiltinValue();
            if (IS_NUM_VALUE(value))
                Push(~(uint64_t)TO_NUM_VALUE(value));
            else
                ASSERT("Invalid op:~ %s", value.Stringify().c_str());
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
                ASSERT("Invalid index op: %s[%s]", ds.Stringify().c_str(), index.Stringify().c_str());
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = *frame->ip++;
            auto value = Pop();
            if (!IS_BOOL_VALUE(value))
                ASSERT("The if condition not a boolean value");
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

            auto ptr = &m_GlobalVariables[index];

            if (IS_REF_VALUE((*ptr))) // if is a reference object,then set the actual value which the reference object points
            {
                while (IS_REF_VALUE((*ptr)))
                    ptr = TO_REF_VALUE((*ptr))->pointer;
                *ptr = value;
            }
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
                    ASSERT("Non matching function parameters for calling arguments,parameter count:%d,argument count:%d", fn->parameterCount, argCount);

                auto callFrame = CallFrame(fn, m_StackTop - argCount);

                *m_CallFrameTop++ = callFrame;

                m_StackTop = callFrame.slot + fn->localVarCount;
            }
            else if (IS_BUILTIN_VALUE(value))
            {
                auto builtin = TO_BUILTIN_VALUE(value);

                if (!builtin->IsBuiltinFn())
                    ASSERT("Not a valid builtin function");

                int32_t j = 0;
                Value *slot = m_StackTop - argCount;

                m_StackTop -= (argCount + 1);

                Value returnValue = builtin->GetBuiltinFn()(slot, argCount);
              
                RegisterToGCRecordChain(returnValue);
                Push(returnValue);
            }
            else
                ASSERT("Calling not a function or a builtinFn");

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
            {
                while (IS_REF_VALUE((*slot)))
                {
                    slot = TO_REF_VALUE((*slot))->pointer;
                }
                *slot = value;
            }
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
        case OP_GET_BUILTIN:
        {
            auto idx = *frame->ip++;
            auto builtinObj = BuiltinManager::GetInstance()->m_Builtins[idx];
            Push(builtinObj);
            break;
        }
        case OP_STRUCT:
        {
            std::unordered_map<std::string, Value> members;
            auto memberCount = *frame->ip++;

            auto tmpPtr = m_StackTop; // save the locale,to avoid gc system delete the tmp object before finish the struct instance creation

            for (int i = 0; i < memberCount; ++i)
            {
                auto name = TO_STR_VALUE((*--tmpPtr))->value;
                auto value = *--tmpPtr;
                members[name] = value;
            }

            auto structInstance = CreateObject<StructObject>(members);
            m_StackTop = tmpPtr; // recover the locale
            Push(structInstance);
            break;
        }
        case OP_GET_STRUCT:
        {
            auto memberName = Pop();
            auto instance = Pop();
            while (IS_REF_VALUE(instance))
                instance = *TO_REF_VALUE(instance)->pointer;
            auto structInstance = TO_STRUCT_VALUE(instance);
            if (IS_STR_VALUE(memberName))
            {
                auto iter = structInstance->members.find(TO_STR_VALUE(memberName)->value);
                if (iter == structInstance->members.end())
                    ASSERT("no member named:(%s) in struct instance:%s", memberName.Stringify().c_str(), structInstance->Stringify().c_str());
                Push(iter->second);
            }
            break;
        }
        case OP_SET_STRUCT:
        {
            auto memberName = Pop();
            auto instance = Pop();
            while (IS_REF_VALUE(instance))
                instance = *TO_REF_VALUE(instance)->pointer;
            auto structInstance = TO_STRUCT_VALUE(instance);
            auto value = Pop();
            if (IS_STR_VALUE(memberName))
            {
                auto iter = structInstance->members.find(TO_STR_VALUE(memberName)->value);
                if (iter == structInstance->members.end())
                    ASSERT("no member named:(%s) in struct instance:(0x%s)", memberName.Stringify().c_str(), PointerAddressToString(structInstance).c_str());
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

            auto ptr = &m_GlobalVariables[index];

            while (IS_REF_VALUE((*ptr)))
                ptr = TO_REF_VALUE((*ptr))->pointer;

            if (IS_ARRAY_VALUE((*ptr)))
            {
                if (!IS_NUM_VALUE(idxValue))
                    ASSERT("Invalid idx for array,only integer is available.");
                auto intIdx = TO_NUM_VALUE(idxValue);
                if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE((*ptr))->elements.size())
                    ASSERT("Idx out of range.");
                Push(CreateObject<RefObject>(&(TO_ARRAY_VALUE((*ptr))->elements[(uint64_t)intIdx])));
            }
            else
                ASSERT("Invalid indexed reference type: %s not a array value.", ptr->Stringify().c_str());
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

            while (IS_REF_VALUE((*slot)))
                slot = TO_REF_VALUE((*slot))->pointer;

            if (IS_ARRAY_VALUE((*slot)))
            {
                if (!IS_NUM_VALUE(idxValue))
                    ASSERT("Invalid idx for array,only integer is available.");
                auto intIdx = TO_NUM_VALUE(idxValue);
                if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE((*slot))->elements.size())
                    ASSERT("Idx out of range.");
                Push(CreateObject<RefObject>(&(TO_ARRAY_VALUE((*slot))->elements[(uint64_t)intIdx])));
            }
            else
                ASSERT("Invalid indexed reference type: %s not a array value.", slot->Stringify().c_str());
            break;
        }
        default:
            return;
        }
    }
}

void VM::RegisterToGCRecordChain(const Value &value)
{
    if (IS_OBJECT_VALUE(value) && TO_OBJECT_VALUE(value)->next == nullptr) // check is null to avoid cross-reference in vm's object record chain
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
        // mark all object which in stack and in context
        for (Value *slot = m_ValueStack; slot < m_StackTop; ++slot)
            slot->Mark();
        for (const auto &c : m_Chunk->constants)
            c.Mark();
        for (auto &g : m_GlobalVariables)
            g.Mark();
        for (CallFrame *slot = m_CallFrames; slot < m_CallFrameTop; ++slot)
            slot->fn->Mark();
    }
    else
    {
        // unMark all objects while exiting vm
        for (Value *slot = m_ValueStack; slot < m_StackTop; ++slot)
            slot->UnMark();
        for (const auto &c : m_Chunk->constants)
            c.UnMark();
        for (auto &g : m_GlobalVariables)
            g.UnMark();
        for (CallFrame *slot = m_CallFrames; slot < m_CallFrameTop; ++slot)
            slot->fn->UnMark();
    }

    // sweep objects which is not reachable
    Object **object = &m_FirstObject;
    while (*object)
    {
        if (!(*object)->marked)
        {
            Object *unreached = *object;
            *object = unreached->next;

            SAFE_DELETE(unreached);

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