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
	OP_NEW_STRUCT,
	OP_GET_VAR,
	OP_SET_VAR,
	OP_DEFINE_VAR,
	OP_GET_INDEX_VAR,
	OP_SET_INDEX_VAR,
	OP_GET_STRUCT_VAR,
	OP_SET_STRUCT_VAR,
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

class Frame
{
public:
	Frame();
	Frame(Frame* upFrame);
	virtual ~Frame();

	void AddOpCode(uint64_t code);
	uint64_t GetOpCodeSize() const;

	uint64_t AddNum(double value);
	uint64_t AddString(std::string_view value);

	std::vector<double> &GetNums();

	void AddFunctionFrame(std::string_view name, Frame frame);
	Frame GetFunctionFrame(std::string_view name);
	bool HasFunctionFrame(std::string_view name);

	void AddStructFrame(std::string_view name, Frame frame);
	Frame GetStructFrame(std::string_view name);
	bool HasStructFrame(std::string_view name);

	std::string Stringify(int depth = 0);

	void Clear();

private:
	friend class VM;

	std::vector<uint64_t> m_Codes;

	std::vector<double> m_Nums;
	std::vector<std::string> m_Strings;

	std::unordered_map<std::string, Frame> m_FunctionFrames;
	std::unordered_map<std::string, Frame> m_StructFrames;

 	Frame* m_UpFrame;
};