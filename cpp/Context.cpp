#include "Context.h"
#include "Utils.h"
#include "VM.h"
#include "Object.h"

Context::Context() : m_UpContext(nullptr) {}
Context::Context(Context *upContext) : m_UpContext(upContext) {}
Context::~Context() {}

void Context::DefineVariableByName(std::string_view name, Object *value)
{
	auto iter = m_Values.find(name.data());
	if (iter != m_Values.end())
		Assert("Redefined variable:" + std::string(name) + " in current context.");
	else
		m_Values[name.data()] = value;
}

void Context::AssignVariableByName(std::string_view name, Object *value)
{
	auto iter = m_Values.find(name.data());
	if (iter != m_Values.end())
		m_Values[name.data()] = value;
	else if (m_UpContext != nullptr)
		m_UpContext->AssignVariableByName(name, value);
	else
		Assert("Undefine variable:" + std::string(name) + " in current context");
}

Object *Context::GetVariableByName(std::string_view name)
{
	auto iter = m_Values.find(name.data());
	if (iter != m_Values.end())
		return iter->second;
	if (m_UpContext != nullptr)
		return m_UpContext->GetVariableByName(name);
	return nullptr;
}