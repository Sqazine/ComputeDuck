#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>
#include "Frame.h"
#include "Value.h"
#include "Object.h"
#include "Utils.h"
#include "Context.h"

#define STACK_MAX 512
#define INITIAL_GC_THRESHOLD 128

class VM
{
public:
	VM();
	~VM();

	void ResetStatus();
	Value Execute(Frame frame);

	StrObject *CreateStrObject(std::string_view value = "");
	ArrayObject *CreateArrayObject(const std::vector<Value> &elements = {});
	StructObject *CreateStructObject(std::string_view name,const std::unordered_map<std::string,Value> &members={});
	RefObject *CreateRefObject(std::string_view name);
	LambdaObject *CreateLambdaObject(int64_t idx);


private:
	void Gc();
	std::function<Value(std::vector<Value>)> GetNativeFunction(std::string_view fnName);
	bool HasNativeFunction(std::string_view name);

	void Push(Value value);
	Value Pop();

	uint8_t sp;
	std::array<Value, STACK_MAX> m_ValueStack;

	Object *firstObject;
	int curObjCount;
	int maxObjCount;

	Context *m_Context;

	std::unordered_map<std::string, std::function<Value(std::vector<Value>)>> m_NativeFunctions;
};