#pragma once
#include "Utils.h"
#include "Value.h"
#include "Object.h"

struct CallFrame
{
    CallFrame() = default;
    ~CallFrame() = default;

    CallFrame(FunctionObject *function, Value *slot)
        : function(function), slot(slot)
    {
        ip = function->chunk.opCodeList.data();
    }

    bool IsEnd()
    {
        if ((ip - function->chunk.opCodeList.data()) < function->chunk.opCodeList.size())
            return false;
        return true;
    }

    FunctionObject *function{nullptr};
    int16_t *ip{nullptr};
    Value *slot{nullptr};
};

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

    bool IsCallFrameStackEmpty();
    void PushCallFrame(const CallFrame &callFrame);
    CallFrame *PopCallFrame();
    CallFrame *PeekCallFrame(int32_t distance);

    Value *GetGlobalVariableSlot(size_t index);

    Value *GetStackTop() { return m_StackTop; }
    void SetStackTop(Value *slot);
    void StackTopJump(size_t slotCount);

    template <IsChildOfObject T, typename... Args>
    T *AllocateObject(Args &&...params)
    {
        T *object = new T(std::forward<Args>(params)...);
        return object;
    }

    Value *GetLocalVariableSlot(int16_t index);

private:
    Allocator() = default;
    ~Allocator() = default;

    Value *m_StackTop{nullptr};
    Value m_ValueStack[STACK_COUNT];

    Value m_GlobalVariables[STACK_COUNT];

    CallFrame *m_CallFrameTop{nullptr};
    CallFrame m_CallFrameStack[STACK_COUNT]{};
};

#define ALLOCATE_OBJECT(type, ...) (Allocator::GetInstance()->AllocateObject<type>(__VA_ARGS__))

#define GET_GLOBAL_VARIABLE_SLOT(x) (Allocator::GetInstance()->GetGlobalVariableSlot(x))

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())

#define GET_STACK_TOP() (Allocator::GetInstance()->GetStackTop())
#define SET_STACK_TOP(x) (Allocator::GetInstance()->SetStackTop(x))
#define STACK_TOP_JUMP(x) (Allocator::GetInstance()->StackTopJump(x))

#define GET_LOCAL_VARIABLE_SLOT(idx) (Allocator::GetInstance()->GetLocalVariableSlot(idx))

#define PUSH_CALL_FRAME(x) (Allocator::GetInstance()->PushCallFrame(x))
#define POP_CALL_FRAME() (Allocator::GetInstance()->PopCallFrame())
#define PEEK_CALL_FRAME(x) (Allocator::GetInstance()->PeekCallFrame(x))