#pragma once
#include "Utils.h"
#include "OpCodeCompilerImpl.h"
#ifdef BUILD_WITH_LLVM
#include "LLVMCompilerImpl.h"
#endif

enum class CompileFlag
{
	OPCODE,
	LLVM,
};

class COMPUTE_DUCK_API Compiler
{
public:
	Compiler(CompileFlag flag)
		: m_CompileFlag(flag)
	{
		m_OpcodeCompilerImpl = new OpCodeCompilerImpl();
#ifdef BUILD_WITH_LLVM
		m_LLVMCompilerImpl = new LLVMCompilerImpl();
#else
#error "Cannot run with llvm,not build yet.";
#endif
	}
	~Compiler()
	{
		SAFE_DELETE(m_OpcodeCompilerImpl);
#ifdef BUILD_WITH_LLVM
		SAFE_DELETE(m_LLVMCompilerImpl);
#else
#error "Cannot run with llvm,not build yet.";
#endif
	}

	Chunk *Compile(const std::vector<Stmt *> &stmts)
	{
		switch (m_CompileFlag)
		{
		case CompileFlag::OPCODE:
			return m_OpcodeCompilerImpl->Compile(stmts);
			break;
		case CompileFlag::LLVM:
		{
#ifdef BUILD_WITH_LLVM
			auto fn = m_LLVMCompilerImpl->Compile(stmts);
			m_LLVMCompilerImpl->Run(fn);
#else
#error "Cannot run with llvm,not build yet.";
#endif
			break;
		}
		default:
			break;
		}
		return nullptr;
	}

private:
	CompileFlag m_CompileFlag;

	OpCodeCompilerImpl *m_OpcodeCompilerImpl;
#ifdef BUILD_WITH_LLVM
	LLVMCompilerImpl *m_LLVMCompilerImpl;
#else
#error "Cannot run with llvm,not build yet.";
#endif
};