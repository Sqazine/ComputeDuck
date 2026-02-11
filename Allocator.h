#pragma once
#include "Utils.h"
#include "Value.h"
#include "Object.h"

struct CallFrame
{
    CallFrame() = default;
    ~CallFrame() = default;

    CallFrame(FunctionObject *f, Value *slot)
        : fn(f), slot(slot)
    {
        auto fnObj = GetFnObject();
        ip = fnObj->chunk.opCodes.data();
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        fnObj->callCount++;
#endif
    }

    bool IsEnd()
    {
        auto fnObj = GetFnObject();
        if ((ip - fnObj->chunk.opCodes.data()) < fnObj->chunk.opCodes.size())
            return false;
        return true;
    }

    FunctionObject *GetFnObject() const
    {
        return fn;
    }

    FunctionObject *fn{ nullptr };
    int16_t *ip{ nullptr };
    Value *slot{ nullptr };
};

class COMPUTE_DUCK_API Allocator
{
public:
    static Allocator *GetInstance();

    void Init();
    void Destroy();

    void ResetStack();
    void ResetFrame();

    template <class T, typename... Args>
    T *CreateObject(Args &&...params)
    {
        if (m_CurObjCount >= m_MaxObjCount
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            && m_IsInsideJitExecutor == false
#endif
            )
            Gc();

        T *object = new T(std::forward<Args>(params)...);

        object->marked = false;
        object->next = m_FirstObject;
        m_FirstObject = object;
        m_CurObjCount++;
        return object;
    }

    RefObject *CreateIndexRefObject(Value *ptr, const Value &idxValue);

    void Push(const Value &value);
    Value Pop();

    void PushCallFrame(const CallFrame &callFrame);
    CallFrame *PopCallFrame();
    CallFrame *PeekCallFrame(int32_t distance);

    bool IsCallFrameStackEmpty();

    Value *GetStackTop() const;
    void SetStackTop(Value *slot);

    void StackTopJump(size_t slotCount);

    Value *GetGlobalVariableRef(size_t index);

    Value *GetLocalVariableSlot(int16_t index);

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    void InsideJitExecutor();
    void OutsideJitExecutor();
private:
    bool m_IsInsideJitExecutor{ false };
#endif

private:
    Allocator() = default;
    ~Allocator() = default;

    void DeleteObject(Object *object);
    void Gc(bool isExitingVM = false);

    Value m_GlobalVariables[STACK_MAX];

    Value *m_StackTop{ nullptr };
    Value m_ValueStack[STACK_MAX]{};

    CallFrame *m_CallFrameTop{ nullptr };
    CallFrame m_CallFrameStack[STACK_MAX]{};

    Object *m_FirstObject{ nullptr };
    size_t m_CurObjCount{ 0 };
    size_t m_MaxObjCount{ 0 };

};

#define GET_GLOBAL_VARIABLE_REF(x) (Allocator::GetInstance()->GetGlobalVariableRef(x)) 

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())

#define PUSH_CALL_FRAME(x) (Allocator::GetInstance()->PushCallFrame(x))
#define POP_CALL_FRAME() (Allocator::GetInstance()->PopCallFrame())
#define PEEK_CALL_FRAME(x) (Allocator::GetInstance()->PeekCallFrame(x))

#define GET_LOCAL_VARIABLE_SLOT(idx) (Allocator::GetInstance()->GetLocalVariableSlot(idx))

#define STACK_TOP() (Allocator::GetInstance()->GetStackTop())
#define STACK_TOP_JUMP(x) (Allocator::GetInstance()->StackTopJump(x))
#define SET_STACK_TOP(x) (Allocator::GetInstance()->SetStackTop(x))