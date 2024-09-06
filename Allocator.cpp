#include "Allocator.h"

extern "C" COMPUTE_DUCK_API StrObject *CreateStrObject(const char *v)
{
    return Allocator::GetInstance()->CreateObject<StrObject>(v);
}

extern "C" COMPUTE_DUCK_API ArrayObject *CreateArrayObject(Value *elements, uint32_t size)
{
    return Allocator::GetInstance()->CreateObject<ArrayObject>(elements, size);
}

extern "C" COMPUTE_DUCK_API RefObject *CreateRefObject(Value *pointer)
{
    return Allocator::GetInstance()->CreateObject<RefObject>(pointer);
}

extern "C" COMPUTE_DUCK_API Value *GetLocalVariableSlot(int16_t scopeDepth, int16_t index, bool isUpValue)
{
    return Allocator::GetInstance()->GetLocalVariableSlot(scopeDepth, index, isUpValue);
}

void Allocator::Init()
{
    m_FirstObject = nullptr;
    m_CurObjCount = 0;
    m_MaxObjCount = STACK_MAX;

    memset(m_ValueStack, 0, sizeof(Value) * STACK_MAX);
    memset(m_CallFrameStack, 0, sizeof(CallFrame) * STACK_MAX);

    ResetStack();
    ResetFrame();
}

void Allocator::Destroy()
{
    Gc(true);
}

Allocator *Allocator::GetInstance()
{
    static Allocator instance;
    return &instance;
}

void Allocator::ResetStack()
{
    m_StackTop = m_ValueStack;
}

void Allocator::ResetFrame()
{
    m_CallFrameTop = m_CallFrameStack;
}

void Allocator::Push(const Value &value)
{
#ifndef NDEBUG
    if (m_StackTop - m_ValueStack >= STACK_MAX)
        ASSERT("Stack Overflow");
#endif
    *m_StackTop++ = value;
}

Value Allocator::Pop()
{
#ifndef NDEBUG
    if (m_StackTop - m_ValueStack < 0)
        ASSERT("Stack Overflow");
#endif
    return *(--m_StackTop);
}

void Allocator::PushCallFrame(const CallFrame &callFrame)
{
    *m_CallFrameTop++ = callFrame;
}

CallFrame *Allocator::PopCallFrame()
{
    return --m_CallFrameTop;
}

CallFrame *Allocator::PeekCallFrameFromFront(int32_t distance)
{
    return &m_CallFrameStack[distance];
}

CallFrame *Allocator::PeekCallFrameFromBack(int32_t distance)
{
    return m_CallFrameTop - distance;
}

bool Allocator::IsCallFrameStackEmpty()
{
    return m_CallFrameTop == m_CallFrameStack;
}

Value *Allocator::GetStackTop() const
{
    return m_StackTop;
}

void Allocator::SetStackTop(Value *slot)
{
    m_StackTop = slot;
}

void Allocator::StackTopJumpBack(size_t slotCount)
{
    m_StackTop -= slotCount;
}

void Allocator::StackTopJump(size_t slotCount)
{
    m_StackTop += slotCount;
}

Value *Allocator::GetGlobalVariableRef(size_t index)
{
    return &m_GlobalVariables[index];
}

Value *Allocator::GetLocalVariableSlot(int16_t scopeDepth, int16_t index, bool isUpValue)
{
    Value *slot = nullptr;
    if (isUpValue == 1)
        slot = PeekCallFrameFromFront(scopeDepth)->slot + index;
    else
        slot = PeekCallFrameFromBack(scopeDepth)->slot + index;
    return slot;
}

void Allocator::Gc(bool isExitingVM)
{
    auto objNum = m_CurObjCount;
    if (!isExitingVM)
    {
        // mark all object which in stack and in context
        for (Value *slot = m_ValueStack; slot < m_StackTop; ++slot)
            slot->Mark();
        for (auto &g : m_GlobalVariables)
            g.Mark();
        for (CallFrame *slot = m_CallFrameStack; slot < m_CallFrameTop; ++slot)
            ObjectMark(slot->fn);
    }
    else
    {
        // unMark all objects while exiting vm
        for (Value *slot = m_ValueStack; slot < m_StackTop; ++slot)
            slot->UnMark();
        for (auto &g : m_GlobalVariables)
            g.UnMark();
        for (CallFrame *slot = m_CallFrameStack; slot < m_CallFrameTop; ++slot)
            ObjectUnMark(slot->fn);
    }

    // sweep objects which is not reachable
    Object **object = &m_FirstObject;
    while (*object)
    {
        if (!(*object)->marked)
        {
            Object *unreached = *object;
            *object = unreached->next;

            DeleteObject(unreached);

            m_CurObjCount--;
        }
        else
        {
            (*object)->marked = false;
            object = &(*object)->next;
        }
    }

    m_MaxObjCount = m_CurObjCount == 0 ? STACK_MAX : m_CurObjCount * 2;

    std::cout << "Collected " << objNum - m_CurObjCount << " objects," << m_CurObjCount << " remaining." << std::endl;
}

void Allocator::DeleteObject(Object *object)
{
    switch (object->type)
    {
    case ObjectType::STR:
    {
        auto strObj = TO_STR_OBJ(object);
        SAFE_DELETE(strObj);
        return;
    }
    case ObjectType::ARRAY:
    {
        auto arrObj = TO_ARRAY_OBJ(object);
        SAFE_DELETE(arrObj);
        return;
    }
    case ObjectType::STRUCT:
    {
        auto structObj = TO_STRUCT_OBJ(object);
        SAFE_DELETE(structObj);
        return;
    }
    case ObjectType::REF:
    {
        auto refObj = TO_REF_OBJ(object);
        SAFE_DELETE(refObj);
        return;
    }
    case ObjectType::FUNCTION:
    {
        auto fnObj = TO_FUNCTION_OBJ(object);
        SAFE_DELETE(fnObj);
        return;
    }
    case ObjectType::BUILTIN:
    {
        auto builtinObj = TO_BUILTIN_OBJ(object);
        SAFE_DELETE(builtinObj);
        return;
    }
    default:
        return;
    }
}
