#include "Allocator.h"
#include <format>
void Allocator::Init()
{
    memset(m_ValueStack, 0, sizeof(Value) * STACK_MAX);

    ResetStack();
}

void Allocator::Destroy()
{
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

Value *Allocator::GetGlobalVariableRef(size_t index)
{
    return &m_GlobalVariables[index];
}

void Allocator::StackTopJump(size_t slotCount)
{
    m_StackTop += slotCount;
}
// ++ 新增内容
Value *Allocator::GetLocalVariableSlot(int16_t index)
{
    Value *slot = nullptr;
    slot = m_ValueStack + index;
    return slot;
}
// -- 新增内容