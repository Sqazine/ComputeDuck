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
#include "Jit.h"
class COMPUTE_DUCK_API VM
{
public:
    VM();
    ~VM();

    void Run(const Value& fnValue);

private:
    struct CallFrame
    {
        CallFrame() = default;
        ~CallFrame() = default;

        CallFrame(const Value& f,Value* slot)
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

        FunctionObject* GetFnObject() const
        {
            return TO_FUNCTION_VALUE(fn);
        }

        Value fn;
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
    CallFrame *PeekCallFrameFromFront(int32_t distance);
    CallFrame *PeekCallFrameFromBack(int32_t distance);

    Value FindActualValue(const Value &v);
    Value *GetEndOfRefValue(Value *v);
    Value GetEndOfRefValue(const Value &v);

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    void RunJit(FunctionObject* curFn,size_t argCount);
#endif

    friend class Jit;

    Value m_GlobalVariables[STACK_MAX];

    Value *m_StackTop;
    Value m_ValueStack[STACK_MAX];

    CallFrame *m_CallFrameTop;
    CallFrame m_CallFrameStack[STACK_MAX];

    Object *m_FirstObject;
    int m_CurObjCount;
    int m_MaxObjCount;

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    Jit* m_Jit{nullptr};
#endif
};

template <class T, typename... Args>
inline T *VM::CreateObject(Args &&...params)
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