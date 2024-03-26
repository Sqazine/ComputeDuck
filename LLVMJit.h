#pragma once


#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"

class LLVMJit
{
public:
	LLVMJit(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl);
	~LLVMJit();

	static std::unique_ptr<LLVMJit> Create();

	const llvm::DataLayout& GetDataLayout() const;

	llvm::orc::JITDylib& GetMainJITDylib();

	llvm::Error AddModule(llvm::orc::ThreadSafeModule tsm, llvm::orc::ResourceTrackerSP rt = nullptr);

	llvm::Expected<llvm::orc::ExecutorSymbolDef> LookUp(llvm::StringRef name);
private:
	std::unique_ptr<llvm::orc::ExecutionSession> m_Es;
	llvm::DataLayout m_DataLayout;
	llvm::orc::MangleAndInterner m_Mangle;
	llvm::orc::RTDyldObjectLinkingLayer m_ObjectLayer;
	llvm::orc::IRCompileLayer m_CompileLayer;
	llvm::orc::JITDylib& m_MainJD;
};