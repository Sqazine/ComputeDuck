#pragma once
#include "Utils.h"
#include "Value.h"

template <typename T>
concept IsChildOfObject = !std::is_same_v<T, void> &&
                          !std::is_abstract_v<T> &&
                          std::is_base_of_v<Object, T>;

class COMPUTEDUCK_API Allocator
{
public:
    static Allocator *GetInstance();

    void Init();
    void Destroy();
    void ResetStatus();

    void Push(const Value &value);
    Value Pop();

    Value *GetGlobalVariableSlot(size_t index);

    Value *GetStackTop() { return m_StackTop; }
    const Value *GetValueStack() { return m_ValueStack; }

    template <IsChildOfObject T, typename... Args>
    T *AllocateObject(Args &&...params)
    {
        T *object = new T(std::forward<Args>(params)...);
        return object;
    }

private:
    Allocator() = default;
    ~Allocator() = default;

    Value *m_StackTop{nullptr};
    Value m_ValueStack[STACK_COUNT];

    Value m_GlobalVariables[STACK_COUNT];
};

#define GET_GLOBAL_VARIABLE_SLOT(x) (Allocator::GetInstance()->GetGlobalVariableSlot(x))

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())