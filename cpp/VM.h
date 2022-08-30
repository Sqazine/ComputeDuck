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
#define INITIAL_GC_THRESHOLD 256

#define GLOBAL_VARIABLE_MAX 1024

struct CallFrame
{
	CallFrame()
		: fn(nullptr), ip(nullptr), slot(nullptr)
	{
	}

	CallFrame(FunctionObject *fn, Value *slot)
		: fn(fn), ip(fn->opCodes.data()), slot(slot)
	{
	}

	FunctionObject *fn;
	int32_t* ip;
	Value *slot;
};

class VM
{
public:
	VM();
	~VM();

	void ResetStatus();

	void Run(const Chunk &chunk);

private:
	void Execute();

	template <class T, typename... Args>
	T *CreateObject(Args &&...params);

	void RegisterToGCRecordChain(const Value &value);

	void Gc(bool isExitingVM = false);

	void Push(const Value &value);
	const Value &Pop();

	void PushCallFrame(const CallFrame& callFrame);
	CallFrame* PopCallFrame();
	CallFrame* PeekCallFrame(int32_t distance);

	Value m_Constants[CONSTANT_MAX];

	Value m_GlobalVariables[GLOBAL_VARIABLE_MAX];

	Value *m_StackTop;
	Value m_ValueStack[STACK_MAX];

	CallFrame* m_CallFrameTop;
	CallFrame m_CallFrames[STACK_MAX];

	Object *firstObject;
	int curObjCount;
	int maxObjCount;

	std::vector<BuiltinObject *> m_Builtins;
};

template <class T, typename... Args>
inline T *VM::CreateObject(Args &&...params)
{
	if (curObjCount >= maxObjCount)
		Gc();

	T *object = new T(std::forward<Args>(params)...);
	object->marked = false;
	object->next = firstObject;
	firstObject = object;
	curObjCount++;
	return object;
}