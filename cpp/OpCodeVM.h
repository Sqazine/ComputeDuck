#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>
#include "Value.h"
#include "Object.h"
#include "Utils.h"
#include "Chunk.h"

class COMPUTE_DUCK_API OpCodeVM
{
public:
    OpCodeVM();
    ~OpCodeVM();

    void Run(FunctionObject* fnObj);

private:
    struct CallFrame
    {
        CallFrame() = default;
        ~CallFrame() = default;

        CallFrame(FunctionObject* fn,Value* slot)
            : fn(fn), ip(fn->chunk.opCodes.data()), slot(slot)
        {
        }

        bool IsEnd()
        {
            if ((ip - fn->chunk.opCodes.data()) < fn->chunk.opCodes.size())
                return false;
            return true;
        }

        FunctionObject* fn{ nullptr };
        int16_t* ip{ nullptr };
        Value* slot{ nullptr };
    };


    void ResetStatus();
    void Execute();

    template <class T, typename... Args>
    T *CreateObject(Args &&...params);

    void RegisterToGCRecordChain(const Value &value);

    void Gc(bool isExitingVM = false);

    void Push(const Value &value);
    Value Pop();

    void PushCallFrame(const CallFrame &callFrame);
    CallFrame *PopCallFrame();
    CallFrame *PeekCallFrame(int32_t distance);
    CallFrame* GetCurCallFrame();

    Value FindActualValue(const Value &v);
    Value *GetEndOfRefValue(Value *v);
    Value GetEndOfRefValue(const Value &v);

    Value m_GlobalVariables[STACK_MAX];

    Value *m_StackTop;
    Value m_ValueStack[STACK_MAX];

    CallFrame *m_CallFrameTop;
    CallFrame m_CallFrameStack[STACK_MAX];

    Object *m_FirstObject;
    int m_CurObjCount;
    int m_MaxObjCount;
};

template <class T, typename... Args>
inline T *OpCodeVM::CreateObject(Args &&...params)
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