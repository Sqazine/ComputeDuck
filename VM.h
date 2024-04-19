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


#define STACK_MAX 512

struct CallFrame
{
	CallFrame() = default;
	~CallFrame() = default;

	CallFrame(FunctionObject *fn, Value *slot)
		: fn(fn), ip(fn->opCodes.data()), slot(slot)
	{
	}

	FunctionObject* fn{ nullptr };
	int16_t* ip{ nullptr };
	Value* slot{ nullptr };
};

class COMPUTE_DUCK_API VM
{
public:
	VM();
	~VM();

	void ResetStatus();

	void Run(Chunk *chunk);

private:
	void Execute();

	template <class T, typename... Args>
	T *CreateObject(Args &&...params);

	void RegisterToGCRecordChain(const Value &value);

	void Gc(bool isExitingVM = false);

	void Push(const Value& value);
	Value Pop();

	void PushCallFrame(const CallFrame& callFrame);
	CallFrame* PopCallFrame();
	CallFrame* PeekCallFrame(int32_t distance);

	const Chunk* m_Chunk{nullptr};

	Value m_GlobalVariables[STACK_MAX];

	Value *m_StackTop;
	Value m_ValueStack[STACK_MAX];

	CallFrame* m_CallFrameTop;
	CallFrame m_CallFrameStack[STACK_MAX];

	Object *m_FirstObject;
	int m_CurObjCount;
	int m_MaxObjCount;
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