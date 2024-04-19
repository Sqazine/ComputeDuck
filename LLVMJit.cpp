#include "LLVMJit.h"

LLVMJit::LLVMJit(std::unique_ptr<llvm::orc::ExecutionSession> es, llvm::orc::JITTargetMachineBuilder jtmb, llvm::DataLayout dl)
	: m_Es(std::move(es)), m_DataLayout(std::move(dl)), m_Mangle(*m_Es, m_DataLayout),
	  m_ObjectLayer(*m_Es,
					[]()
					{ return std::make_unique<llvm::SectionMemoryManager>(); }),
	  m_CompileLayer(*m_Es, m_ObjectLayer,
					 std::make_unique<llvm::orc::ConcurrentIRCompiler>(std::move(jtmb))),
	  m_MainJD(m_Es->createBareJITDylib("<main>"))
{
	m_MainJD.addGenerator(cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(m_DataLayout.getGlobalPrefix())));

	if (jtmb.getTargetTriple().isOSBinFormatCOFF())
	{
		m_ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
		m_ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
	}
}

LLVMJit::~LLVMJit()
{
	if (auto Err = m_Es->endSession())
		m_Es->reportError(std::move(Err));
}

std::unique_ptr<LLVMJit> LLVMJit::Create()
{
	auto epc = llvm::orc::SelfExecutorProcessControl::Create();
	if (!epc)
		return nullptr;

	auto es = std::make_unique<llvm::orc::ExecutionSession>(std::move(*epc));

	llvm::orc::JITTargetMachineBuilder JTMB(es->getExecutorProcessControl().getTargetTriple());

	auto dataLayout = JTMB.getDefaultDataLayoutForTarget();
	if (!dataLayout)
		return nullptr;

	return std::make_unique<LLVMJit>(std::move(es), std::move(JTMB), std::move(*dataLayout));
}

const llvm::DataLayout &LLVMJit::GetDataLayout() const
{
	return m_DataLayout;
}

llvm::orc::JITDylib &LLVMJit::GetMainJITDylib()
{
	return m_MainJD;
}

llvm::Error LLVMJit::AddModule(llvm::orc::ThreadSafeModule tsm, llvm::orc::ResourceTrackerSP rt)
{
	if (!rt)
		rt = m_MainJD.getDefaultResourceTracker();
	return m_CompileLayer.add(rt, std::move(tsm));
}

llvm::Expected<llvm::orc::ExecutorSymbolDef> LLVMJit::LookUp(llvm::StringRef name)
{
	return m_Es->lookup({&m_MainJD}, m_Mangle(name.str()));
}
