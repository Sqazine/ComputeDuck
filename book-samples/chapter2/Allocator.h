#pragma once
#include "Utils.h"
#include "Value.h"
class COMPUTEDUCK_API Allocator
{
public:
    static Allocator *GetInstance();

    void Init();
    void Destroy();

    void ResetStack();

    void Push(const Value &value);
    Value Pop();

    Value *GetStackTop() { return m_StackTop; }
    const Value *GetValueStack() { return m_ValueStack; }

private:
    Allocator() = default;
    ~Allocator() = default;

    Value *m_StackTop{nullptr};
    Value m_ValueStack[STACK_MAX]{};
};

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())