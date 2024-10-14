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

    void ResetStack();

    void Push(const Value &value);
    Value Pop();

    Value *GetGlobalVariableRef(size_t index);

    Value *GetStackTop() { return m_StackTop; }

    void StackTopJump(size_t slotCount);

    template <IsChildOfObject T, typename... Args>
    T *CreateObject(Args &&...params)
    {
        T *object = new T(std::forward<Args>(params)...);
        return object;
    }

    // ++ 新增内容
    Value *GetLocalVariableSlot(int16_t index);
    // -- 新增内容
private:
    Allocator() = default;
    ~Allocator() = default;

    Value *m_StackTop{nullptr};
    Value m_ValueStack[STACK_MAX];

    Value m_GlobalVariables[STACK_MAX];
};

#define GET_GLOBAL_VARIABLE_REF(x) (Allocator::GetInstance()->GetGlobalVariableRef(x))

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())

#define STACK_TOP() (Allocator::GetInstance()->GetStackTop())
#define STACK_TOP_JUMP(x) (Allocator::GetInstance()->StackTopJump(x))

// ++ 新增内容
#define GET_LOCAL_VARIABLE_SLOT(idx) (Allocator::GetInstance()->GetLocalVariableSlot(idx))
// -- 新增内容
