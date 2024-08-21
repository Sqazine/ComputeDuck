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
        fnObj->callCount++;
        ip = fnObj->chunk.opCodes.data();
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

    FunctionObject *fn;
    int16_t *ip{ nullptr };
    Value *slot{ nullptr };
};

class COMPUTE_DUCK_API Allocator
{
public:
    static Allocator *GetInstance();

    void ResetStack();
    void ResetFrame();

    template <class T, typename... Args>
    T *CreateObject(Args &&...params) 
    {
        if (m_CurObjCount >= m_MaxObjCount)
            Gc();

        T *object = new T(std::forward<Args>(params)...);

        object->marked = false;
        object->next = m_FirstObject;
        m_FirstObject = object;
        m_CurObjCount++;
        return object;
    }

    //void RegisterToGCRecordChain(const Value &value);

    void Push(const Value &value);
    Value Pop();

    void PushCallFrame(const CallFrame &callFrame);
    CallFrame *PopCallFrame();
    CallFrame *PeekCallFrameFromFront(int32_t distance);
    CallFrame *PeekCallFrameFromBack(int32_t distance);

    bool IsCallFrameStackEmpty();

    Value* GetStackTop() const;
    void SetStackTop(Value *slot);

    void StackTopJumpBack(size_t slotCount);
    void StackTopJump(size_t slotCount);

    Value* GetGlobalVariableRef(size_t index);

    void FreeAllObjects();
private:
    Allocator();
    ~Allocator();

    void DeleteObject(Object *object);
    void Gc(bool isExitingVM = false);

    Value m_GlobalVariables[STACK_MAX];

    Value *m_StackTop;
    Value m_ValueStack[STACK_MAX];

    CallFrame *m_CallFrameTop;
    CallFrame m_CallFrameStack[STACK_MAX];

    Object *m_FirstObject;
    int m_CurObjCount;
    int m_MaxObjCount;
};

#define GET_GLOBAL_VARIABLE_REF(x) (Allocator::GetInstance()->GetGlobalVariableRef(x)) 

#define PUSH(x) (Allocator::GetInstance()->Push(x))
#define POP() (Allocator::GetInstance()->Pop())

//#define REGISTER_GC_RECORD_CHAIN(x) (Allocator::GetInstance()->RegisterToGCRecordChain(x))

#define PUSH_CALL_FRAME(x) (Allocator::GetInstance()->PushCallFrame(x))
#define POP_CALL_FRAME(x) (Allocator::GetInstance()->PopCallFrame())
#define PEEK_CALL_FRAME_FROM_FRONT(x) (Allocator::GetInstance()->PeekCallFrameFromFront(x))
#define PEEK_CALL_FRAME_FROM_BACK(x) (Allocator::GetInstance()->PeekCallFrameFromBack(x))

#define STACK_TOP() (Allocator::GetInstance()->GetStackTop())
#define STACK_TOP_JUMP_BACK(x) (Allocator::GetInstance()->StackTopJumpBack(x))
#define STACK_TOP_JUMP(x) (Allocator::GetInstance()->StackTopJump(x))
#define SET_STACK_TOP(x) (Allocator::GetInstance()->SetStackTop(x))