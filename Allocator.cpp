#include "Allocator.h"

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

RefObject *Allocator::CreateIndexRefObject(Value *ptr, const Value &idxValue)
{
    if (IS_ARRAY_VALUE(*ptr))
    {
        if (!IS_NUM_VALUE(idxValue))
            ASSERT("Invalid idx for array,only integer is available.");
        auto intIdx = TO_NUM_VALUE(idxValue);
        if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE(*ptr)->len)
            ASSERT("Idx out of range.");
        return Allocator::GetInstance()->CreateObject<RefObject>(&(TO_ARRAY_VALUE(*ptr)->elements[(uint64_t)intIdx]));
    }
    else
        ASSERT("Invalid indexed reference type: %s not a array value.", ptr->Stringify().c_str());
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

Value *Allocator::GetGlobalVariableRef(size_t index)
{
    return &m_GlobalVariables[index];
}

Value *Allocator::GetLocalVariableSlot(int16_t index)
{
    return PeekCallFrame(1)->slot + index;
}

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
void Allocator::InsideJitExecutor()
{
    m_IsInsideJitExecutor=true;
}

void Allocator::OutsideJitExecutor()
{
    m_IsInsideJitExecutor=false;
}
#endif

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
