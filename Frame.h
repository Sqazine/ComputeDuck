#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <deque>
#include <unordered_map>

enum OpCode
{
	OP_NEW_NUM,
	OP_NEW_STR,
	OP_NEW_TRUE,
	OP_NEW_FALSE,
	OP_NEW_NIL,
	OP_NEW_ARRAY,
	OP_NEW_FUNCTION,
	OP_GET_VAR,
	OP_SET_VAR,
	OP_DEFINE_VAR,
	OP_GET_INDEX_VAR,
	OP_SET_INDEX_VAR,
	OP_NEG,
	OP_RETURN,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_GREATER,
	OP_LESS,
	OP_GREATER_EQUAL,
	OP_LESS_EQUAL,
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_NOT,
	OP_OR,
	OP_AND,
	OP_ENTER_SCOPE,
	OP_EXIT_SCOPE,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_FUNCTION_CALL,
};


#define IS_NORMAL_FRAME(f) (f->Type() == FrameType::NORMAL)
#define IS_NATIVE_FUNCTION_FRAME(f) (f->Type() == FrameType::NATIVE_FUNCTION)
#define TO_NORMAL_FRAME(f) ((Frame *)f)
#define TO_NATIVE_FUNCTION_FRAME(f) ((NativeFunctionFrame *)f)

class Frame
{
public:
	Frame();
	virtual ~Frame();

	void AddOpCode(uint64_t code);
	uint64_t GetOpCodeSize() const;

	uint64_t AddNum(double value);
	uint64_t AddString(std::string_view value);

	std::vector<double> &GetNums();

	uint64_t AddFunctionFrame(Frame frame);
	Frame GetFunctionFrame(uint64_t idx);
	bool HasFunctionFrame(uint64_t idx);

	std::string Stringify(int depth = 0);

	void Clear();
private:
	friend class VM;

	std::vector<uint64_t> m_Codes;

	std::vector<double> m_Nums;
	std::vector<std::string> m_Strings;

	std::vector<Frame> m_FunctionFrames;

};