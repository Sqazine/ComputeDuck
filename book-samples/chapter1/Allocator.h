#pragma once
#include "Utils.h"
#include "Value.h"
class COMPUTEDUCK_API Allocator
{
public:
    static Allocator *GetInstance();

    void Init();
    void Destroy();
    void ResetStatus();

    void Push(const Value &value);
    Value Pop();
private:
    Allocator() = default;
    ~Allocator() = default;

    Value *m_StackTop{nullptr};
    Value m_ValueStack[STACK_COUNT]{};
};

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())