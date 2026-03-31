#include "Allocator.h"
#include <format>
void Allocator::Init()
{
    memset(m_ValueStack, 0, sizeof(Value) * STACK_COUNT);

    ResetStatus();
}

void Allocator::Destroy()
{
}

Allocator *Allocator::GetInstance()
{
    static Allocator instance;
    return &instance;
}

void Allocator::ResetStatus()
{
    m_StackTop = m_ValueStack;
    // ++ 新增内容
    m_CallFrameTop = m_CallFrameStack;
    // -- 新增内容
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

// ++ 新增内容
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

void Allocator::SetStackTop(Value *slot)
{
    m_StackTop = slot;
}
// -- 新增内容

Value *Allocator::GetGlobalVariableSlot(size_t index)
{
    return &m_GlobalVariables[index];
}

void Allocator::StackTopJump(size_t slotCount)
{
    m_StackTop += slotCount;
}

Value *Allocator::GetLocalVariableSlot(int16_t index)
{
    // ++ 修改内容
    // Value *slot = nullptr;
    // slot = m_ValueStack + index;
    // return slot;
    return PeekCallFrame(1)->slot + index;
    // -- 修改内容
}
