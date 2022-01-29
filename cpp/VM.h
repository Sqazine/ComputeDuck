#pragma once
#include <array>
#include <cstdint>
#include <stack>
#include <functional>
#include <unordered_map>
#include <string>
#include <string_view>
#include "Frame.h"
#include "Object.h"
#include "Utils.h"
#include "Context.h"

#define STACK_MAX 4096
#define INIT_OBJ_NUM_MAX 4096

class VM
{
public:
	VM();
	~VM();

	void ResetStatus();
	Object *Execute(Frame frame);

	NumObject *CreateNumObject(double value = 0.0);
	StrObject *CreateStrObject(std::string_view value = "");
	BoolObject *CreateBoolObject(bool value = false);
	NilObject *CreateNilObject();
	ArrayObject *CreateArrayObject(const std::vector<Object *> &elements = {});
	StructObject *CreateStructObject(std::string_view name,
									 const std::unordered_map<std::string, Object *> &members={});

	void Gc();

private:
	std::function<Object *(std::vector<Object *>)> GetNativeFunction(std::string_view fnName);
	bool HasNativeFunction(std::string_view name);

	void PushObject(Object *object);
	Object *PopObject();

	uint8_t sp;
	std::array<Object *, STACK_MAX> m_ObjectStack;

	Object *firstObject;
	int curObjCount;
	int maxObjCount;

	Context *m_Context;

	std::unordered_map<std::string, std::function<Object *(std::vector<Object *>)>> m_NativeFunctions;
};