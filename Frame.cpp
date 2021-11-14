#include "Frame.h"
#include "Utils.h"

Frame::Frame()
{
}

Frame::~Frame()
{
	Clear();
}
void Frame::AddOpCode(uint64_t code)
{
	m_Codes.emplace_back(code);
}

uint64_t Frame::GetOpCodeSize() const
{
	return m_Codes.size();
}

uint64_t Frame::AddNum(double value)
{
	m_Nums.emplace_back(value);
	return m_Nums.size() - 1;
}

uint64_t Frame::AddString(std::string_view value)
{
	m_Strings.emplace_back(value);
	return m_Strings.size() - 1;
}

std::vector<double> &Frame::GetNums()
{
	return m_Nums;
}

uint64_t Frame::AddFunctionFrame(Frame frame)
{
	m_FunctionFrames.emplace_back(frame);
	return m_FunctionFrames.size() - 1;
}

Frame Frame::GetFunctionFrame(uint64_t idx)
{
		return m_FunctionFrames[idx];
}

bool Frame::HasFunctionFrame(uint64_t idx)
{
	if (idx >= 0 || idx < m_FunctionFrames.size())
		return true;
	else
		return false;
}

std::string Frame::Stringify(int depth)
{
	std::string interval;
	for (size_t i = 0; i < depth; ++i)
		interval += "\t";

#define SINGLE_INSTR_STRINGIFY(op) \
	result << interval << "\t" << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "\n"

#define CONSTANT_INSTR_STRINGIFY(op, vec) \
	result << interval << "\t" << std::setfill('0') << std::setw(8) << i << "     " << (#op) << "     " << vec[m_Codes[++i]] << "\n"

	std::stringstream result;

	for (size_t i = 0; i < m_FunctionFrames.size(); ++i)
	{
		result << interval << "Frame " << i << ":\n";
		result << m_FunctionFrames[i].Stringify(depth + 1);
	}

	result << interval << "OpCodes:\n";

	for (size_t i = 0; i < m_Codes.size(); ++i)
	{
		switch (m_Codes[i])
		{
		case OP_RETURN:
			SINGLE_INSTR_STRINGIFY(OP_RETURN);
			break;
		case OP_NEW_NUM:
			CONSTANT_INSTR_STRINGIFY(OP_NEW_NUM, m_Nums);
			break;
		case OP_NEW_STR:
			CONSTANT_INSTR_STRINGIFY(OP_NEW_STR, m_Strings);
			break;
		case OP_NEW_TRUE:
			SINGLE_INSTR_STRINGIFY(OP_NEW_TRUE);
			break;
		case OP_NEW_FALSE:
			SINGLE_INSTR_STRINGIFY(OP_NEW_FALSE);
			break;
		case OP_NEW_NIL:
			SINGLE_INSTR_STRINGIFY(OP_NEW_NIL);
			break;
		case OP_NEG:
			SINGLE_INSTR_STRINGIFY(OP_NEG);
			break;
		case OP_ADD:
			SINGLE_INSTR_STRINGIFY(OP_ADD);
			break;
		case OP_SUB:
			SINGLE_INSTR_STRINGIFY(OP_SUB);
			break;
		case OP_MUL:
			SINGLE_INSTR_STRINGIFY(OP_MUL);
			break;
		case OP_DIV:
			SINGLE_INSTR_STRINGIFY(OP_DIV);
			break;
		case OP_GREATER:
			SINGLE_INSTR_STRINGIFY(OP_GREATER);
			break;
		case OP_LESS:
			SINGLE_INSTR_STRINGIFY(OP_LESS);
			break;
		case OP_GREATER_EQUAL:
			SINGLE_INSTR_STRINGIFY(OP_GREATER_EQUAL);
			break;
		case OP_LESS_EQUAL:
			SINGLE_INSTR_STRINGIFY(OP_LESS_EQUAL);
			break;
		case OP_EQUAL:
			SINGLE_INSTR_STRINGIFY(OP_EQUAL);
			break;
		case OP_NOT:
			SINGLE_INSTR_STRINGIFY(OP_NOT);
			break;
		case OP_NOT_EQUAL:
			SINGLE_INSTR_STRINGIFY(OP_NOT_EQUAL);
			break;
		case OP_AND:
			SINGLE_INSTR_STRINGIFY(OP_AND);
			break;
		case OP_OR:
			SINGLE_INSTR_STRINGIFY(OP_OR);
			break;
		case OP_GET_VAR:
			CONSTANT_INSTR_STRINGIFY(OP_GET_VAR, m_Strings);
			break;
		case OP_DEFINE_VAR:
			CONSTANT_INSTR_STRINGIFY(OP_DEFINE_VAR, m_Strings);
			break;
		case OP_SET_VAR:
			CONSTANT_INSTR_STRINGIFY(OP_SET_VAR, m_Strings);
			break;
		case OP_NEW_ARRAY:
			CONSTANT_INSTR_STRINGIFY(OP_NEW_ARRAY, m_Nums);
			break;
		case OP_NEW_FUNCTION:
			CONSTANT_INSTR_STRINGIFY(OP_NEW_FUNCTION, m_Nums);
			break;
		case OP_GET_INDEX_VAR:
			SINGLE_INSTR_STRINGIFY(OP_GET_INDEX_VAR);
			break;
		case OP_SET_INDEX_VAR:
			SINGLE_INSTR_STRINGIFY(OP_SET_INDEX_VAR);
			break;

		case OP_ENTER_SCOPE:
			SINGLE_INSTR_STRINGIFY(OP_ENTER_SCOPE);
			break;
		case OP_EXIT_SCOPE:
			SINGLE_INSTR_STRINGIFY(OP_EXIT_SCOPE);
			break;
		case OP_JUMP:
			CONSTANT_INSTR_STRINGIFY(OP_JUMP, m_Nums);
			break;
		case OP_JUMP_IF_FALSE:
			CONSTANT_INSTR_STRINGIFY(OP_JUMP_IF_FALSE, m_Nums);
			break;
		case OP_FUNCTION_CALL:
			CONSTANT_INSTR_STRINGIFY(OP_FUNCTION_CALL,m_Strings);
			break;
		case OP_REF:
			SINGLE_INSTR_STRINGIFY(OP_REF);
			break;
		default:
			SINGLE_INSTR_STRINGIFY(UNKNOWN);
			break;
		}
	}

	return result.str();
}
void Frame::Clear()
{
	std::vector<uint64_t>().swap(m_Codes);
	std::vector<double>().swap(m_Nums);
	std::vector<std::string>().swap(m_Strings);

	for (auto funcFrame : m_FunctionFrames)
		funcFrame.Clear();

	std::vector<Frame>().swap(m_FunctionFrames);
}