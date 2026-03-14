#pragma once
#include "Utils.h"
#include "Value.h"
#include "Object.h"

struct CallFrame
{
    CallFrame() = default;
    ~CallFrame() = default;

    CallFrame(ClosureObject *closure, Value *slot)
        : closure(closure), slot(slot)
    {
        ip = closure->function->chunk.opCodeList.data();
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        closure->callCount++;
#endif
    }

    bool IsEnd()
    {
        if ((ip - closure->function->chunk.opCodeList.data()) < closure->function->chunk.opCodeList.size())
            return false;
        return true;
    }

    ClosureObject *closure{nullptr};
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

    template <IsChildOfObject T, typename... Args>
    T *AllocateObject(Args &&...params)
    {
        if (m_CurObjCount >= m_MaxObjCount && m_IsGCEnabled)
            Gc();

        T *object = new T(std::forward<Args>(params)...);

        object->marked = false;
        object->next = m_FirstObject;
        m_FirstObject = object;
        m_CurObjCount++;
        return object;
    }

    RefObject *AllocateIndexRefObject(Value *ptr, const Value &idxValue);

    void Push(const Value &value);
    Value Pop();

    void PushCallFrame(const CallFrame &callFrame);
    CallFrame *PopCallFrame();
    CallFrame *PeekCallFrame(int32_t distance);

    bool IsCallFrameStackEmpty();

    Value *GetStackTop() const;
    void SetStackTop(Value *slot);

    void StackTopJump(size_t slotCount);

    UpvalueObject *CaptureUpvalue(int16_t index, int16_t scopeDepth);
	void ClosedUpvalues(Value *last);

    Value *GetGlobalVariableSlot(size_t index);

    Value *GetLocalVariableSlot(int16_t index);

    void DisableGC();
    void EnableGC();

private:
    Allocator() = default;
    ~Allocator() = default;

    void Gc(bool deleteAll = false);

    bool m_IsGCEnabled{true};

    Value m_GlobalVariables[STACK_COUNT];

    Value *m_StackTop{nullptr};
    Value m_ValueStack[STACK_COUNT]{};

    UpvalueObject *m_OpenUpvalues{nullptr};

    CallFrame *m_CallFrameTop{nullptr};
    CallFrame m_CallFrameStack[STACK_COUNT]{};

    Object *m_FirstObject{nullptr};
    size_t m_CurObjCount{0};
    size_t m_MaxObjCount{0};
};

#define ALLOCATE_OBJECT(type, ...) (Allocator::GetInstance()->AllocateObject<type>(__VA_ARGS__))
#define ALLOCATE_INDEX_REF_OBJECT(ptr, idxValue) (Allocator::GetInstance()->AllocateIndexRefObject(ptr, idxValue))

#define GET_GLOBAL_VARIABLE_SLOT(x) (Allocator::GetInstance()->GetGlobalVariableSlot(x))

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())

#define PUSH_CALL_FRAME(x) (Allocator::GetInstance()->PushCallFrame(x))
#define POP_CALL_FRAME() (Allocator::GetInstance()->PopCallFrame())
#define PEEK_CALL_FRAME(x) (Allocator::GetInstance()->PeekCallFrame(x))

#define GET_LOCAL_VARIABLE_SLOT(idx) (Allocator::GetInstance()->GetLocalVariableSlot(idx))

#define GET_STACK_TOP() (Allocator::GetInstance()->GetStackTop())
#define STACK_TOP_JUMP(x) (Allocator::GetInstance()->StackTopJump(x))
#define SET_STACK_TOP(x) (Allocator::GetInstance()->SetStackTop(x))