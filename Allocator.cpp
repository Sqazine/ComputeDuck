#include "Allocator.h"
#include "BuiltinManager.h"
void Allocator::Init()
{
    m_FirstObject = nullptr;
    m_CurObjCount = 0;
    m_MaxObjCount = STACK_COUNT;

    memset(m_ValueStack, 0, sizeof(Value) * STACK_COUNT);
    memset(m_CallFrameStack, 0, sizeof(CallFrame) * STACK_COUNT);

    ResetStatus();
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

void Allocator::ResetStatus()
{
    m_StackTop = m_ValueStack;
    m_CallFrameTop = m_CallFrameStack;
}

RefObject *Allocator::AllocateIndexRefObject(Value *ptr, const Value &idxValue)
{
    if (IS_ARRAY_VALUE(*ptr))
    {
        if (!IS_NUM_VALUE(idxValue))
            ASSERT("Invalid idx for array,only integer is available.");
        auto intIdx = TO_NUM_VALUE(idxValue);
        if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE(*ptr)->len)
            ASSERT("Idx out of range.");
        return ALLOCATE_OBJECT(RefObject, &(TO_ARRAY_VALUE(*ptr)->elements[(uint64_t)intIdx]));
    }
    else
        ASSERT("Invalid indexed reference type: %s not a array value.", ptr->Stringify().c_str());
}

void Allocator::Push(const Value &value)
{
#ifndef NDEBUG
    if (m_StackTop - m_ValueStack >= STACK_COUNT)
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

CallFrame *Allocator::PeekCallFrame(int32_t distance)
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

void Allocator::StackTopJump(size_t slotCount)
{
    m_StackTop += slotCount;
}

UpvalueObject *Allocator::CaptureUpvalue(Value *local)
{
    UpvalueObject *prevUpvalue = nullptr;
    UpvalueObject *upvalue = m_OpenUpvalues;
    while (upvalue != nullptr && upvalue->location > local)
    {
        prevUpvalue = upvalue;
        upvalue = upvalue->nextUpvalue;
    }

    if (upvalue != nullptr && upvalue->location == local)
        return upvalue;

    auto createdUpvalue = ALLOCATE_OBJECT(UpvalueObject, local);
    createdUpvalue->nextUpvalue = upvalue;

    if (prevUpvalue == nullptr)
        m_OpenUpvalues = createdUpvalue;
    else
        prevUpvalue->nextUpvalue = createdUpvalue;

    return createdUpvalue;
}
void Allocator::ClosedUpvalues(Value *last)
{
    while (m_OpenUpvalues != nullptr && m_OpenUpvalues->location >= last)
    {
        UpvalueObject *upvalue = m_OpenUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        m_OpenUpvalues = upvalue->nextUpvalue;
    }
}

Value *Allocator::GetGlobalVariableSlot(size_t index)
{
    return &m_GlobalVariables[index];
}

Value *Allocator::GetLocalVariableSlot(int16_t index)
{
    return PeekCallFrame(1)->slot + index;
}

Value *Allocator::GetUpvalueVariableSlot(int16_t index, int16_t scopeDepth)
{
    return (m_CallFrameStack + scopeDepth)->slot + index;
}

void Allocator::DisableGC()
{
    m_IsGCEnabled = false;
}

void Allocator::EnableGC()
{
    m_IsGCEnabled = true;
}

void Allocator::Gc(bool deleteAll)
{
    auto objNum = m_CurObjCount;
    if (!deleteAll)
    {
        // mark all object which in stack and in context
        for (Value *slot = m_ValueStack; slot < m_StackTop; ++slot)
            slot->Mark();
        for (Value &g : m_GlobalVariables)
            g.Mark();
        for (CallFrame *slot = m_CallFrameStack; slot < m_CallFrameTop; ++slot)
            MarkObject(slot->closure);
        for (UpvalueObject *upvalue = m_OpenUpvalues; upvalue != nullptr; upvalue = upvalue->nextUpvalue)
            MarkObject(upvalue);
      
        BuiltinManager::GetInstance()->GetBuiltinObjectTable().Mark();
    }
    else
    {
        // unMark all objects while exiting vm
        for (Value &slot : m_ValueStack)
            slot.UnMark();
        for (Value &g : m_GlobalVariables)
            g.UnMark();
        for (CallFrame &slot : m_CallFrameStack)
            UnMarkObject(slot.closure);
        for (UpvalueObject *upvalue = m_OpenUpvalues; upvalue != nullptr; upvalue = upvalue->nextUpvalue)
            UnMarkObject(upvalue);

         BuiltinManager::GetInstance()->GetBuiltinObjectTable().UnMark();
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

    m_MaxObjCount = m_CurObjCount == 0 ? STACK_COUNT : m_CurObjCount * 2;

    std::cout << "Collected " << objNum - m_CurObjCount << " objects," << m_CurObjCount << " remaining." << std::endl;
}