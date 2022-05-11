#include "Frame.h"
#include "Utils.h"

Frame::Frame()
	: m_UpFrame(nullptr)
{
}

Frame::Frame(Frame *upFrame)
	: m_UpFrame(upFrame)
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

void Frame::AddFunctionFrame(std::string_view name, Frame frame)
{
	if (m_FunctionFrames.find(name.data()) != m_FunctionFrames.end())
		Assert("Redefinition function:" + std::string(name));
	m_FunctionFrames[name.data()] = frame;
}

Frame Frame::GetFunctionFrame(std::string_view name)
{
	if (m_FunctionFrames.find(name.data()) != m_FunctionFrames.end())
		return m_FunctionFrames[name.data()];
	else if (m_UpFrame)
		return m_UpFrame->GetFunctionFrame(name);
	Assert("No function definition:" + std::string(name));
	return Frame(); //just avoid compiler warning
}

bool Frame::HasFunctionFrame(std::string_view name)
{
	for (auto [key, value] : m_FunctionFrames)
		if (key == name)
			return true;

	if (m_UpFrame)
		return m_UpFrame->HasFunctionFrame(name);

	return false;
}

void Frame::AddStructFrame(std::string_view name, Frame frame)
{
	if (m_StructFrames.find(name.data()) != m_StructFrames.end())
		Assert("Redefinition struct:" + std::string(name));
	m_StructFrames[name.data()] = frame;
}

Frame Frame::GetStructFrame(std::string_view name)
{
	if (m_StructFrames.find(name.data()) != m_StructFrames.end())
		return m_StructFrames[name.data()];
	else if (m_UpFrame)
		return m_UpFrame->GetStructFrame(name);
	Assert("No struct definition:" + std::string(name));
	return Frame(); //just avoid compiler warning
}

bool Frame::HasStructFrame(std::string_view name)
{
	for (auto [key, value] : m_StructFrames)
		if (key == name)
			return true;

	if (m_UpFrame)
		return m_UpFrame->HasStructFrame(name);

	return false;
}

uint64_t Frame::AddLambdaFrame(Frame frame)
{
	Frame *rootFrame = this;
	//lambda frame save to rootframe
	if (rootFrame->m_UpFrame)
	{
		while (rootFrame->m_UpFrame)
			rootFrame = rootFrame->m_UpFrame;
	}
	rootFrame->m_LambdaFrames.emplace_back(frame);
	return rootFrame->m_LambdaFrames.size() - 1;
}

Frame Frame::GetLambdaFrame(uint64_t idx)
{
	if (m_UpFrame)
	{
		Frame *rootFrame = this;
		while (rootFrame->m_UpFrame)
			rootFrame = rootFrame->m_UpFrame;
		return rootFrame->GetLambdaFrame(idx);
	}
	else if (idx >= 0 || idx < m_LambdaFrames.size())
		return m_LambdaFrames[idx];
	else
		return nullptr;
}

bool Frame::HasLambdaFrame(uint64_t idx)
{
	if (m_UpFrame)
	{
		Frame *rootFrame = this;
		while (rootFrame->m_UpFrame)
			rootFrame = rootFrame->m_UpFrame;
		return rootFrame->HasLambdaFrame(idx);
	}
	else if (idx >= 0 || idx < m_LambdaFrames.size())
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

	for (auto [key, value] : m_StructFrames)
	{
		result << interval << "Frame " << key << ":\n";
		result << value.Stringify(depth + 1);
	}

	for (auto [key, value] : m_FunctionFrames)
	{
		result << interval << "Frame " << key << ":\n";
		result << value.Stringify(depth + 1);
	}

	for (size_t i = 0; i < m_LambdaFrames.size(); ++i)
	{
		result << interval << "Frame " << i << ":\n";
		result << m_LambdaFrames[i].Stringify(depth + 1);
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
		case OP_NEW_STRUCT:
			CONSTANT_INSTR_STRINGIFY(OP_NEW_STRUCT, m_Strings);
			break;
		case OP_NEW_LAMBDA:
			CONSTANT_INSTR_STRINGIFY(OP_NEW_LAMBDA, m_Nums);
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
		case OP_GET_INDEX_VAR:
			SINGLE_INSTR_STRINGIFY(OP_GET_INDEX_VAR);
			break;
		case OP_SET_INDEX_VAR:
			SINGLE_INSTR_STRINGIFY(OP_SET_INDEX_VAR);
			break;
		case OP_GET_STRUCT_VAR:
			CONSTANT_INSTR_STRINGIFY(OP_GET_STRUCT_VAR, m_Strings);
			break;
		case OP_SET_STRUCT_VAR:
			CONSTANT_INSTR_STRINGIFY(OP_SET_STRUCT_VAR, m_Strings);
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
			CONSTANT_INSTR_STRINGIFY(OP_FUNCTION_CALL, m_Strings);
			break;
		case OP_STRUCT_LAMBDA_CALL:
			CONSTANT_INSTR_STRINGIFY(OP_STRUCT_LAMBDA_CALL, m_Strings);
			break;
		case OP_REF:
			CONSTANT_INSTR_STRINGIFY(OP_REF, m_Strings);
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

	std::unordered_map<std::string, Frame>().swap(m_FunctionFrames);
}