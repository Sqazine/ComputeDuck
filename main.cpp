#include <string>
#include <string_view>
#include <filesystem>
#include "PreProcessor.h"
#include "Parser.h"
#include "Compiler.h"
#include "VM.h"
#include "BuiltinManager.h"
#ifdef BUILD_WITH_LLVM
#include "LLVMCompiler.h"
#endif

namespace
{
	bool g_UseLlvmFlag = false;
	std::string g_SourceFilePath;
}

PreProcessor* g_PreProcessor = new PreProcessor();
Parser* g_Parser = new Parser();
Compiler* g_Compiler = new Compiler();
VM* g_Vm = new VM();

#ifdef BUILD_WITH_LLVM
LLVMCompiler* g_LLVMCompiler = new LLVMCompiler();
#endif

void Run(std::string_view content, bool isLineInterpret)
{
	auto tokens = g_PreProcessor->PreProcess(content);
#ifdef _DEBUG
	for (const auto& token : tokens)
		std::cout << token << std::endl;
#endif

	auto stmts = g_Parser->Parse(tokens);
#ifdef _DEBUG
	for (const auto& stmt : stmts)
		std::cout << stmt->Stringify() << std::endl;
#endif

	if (g_UseLlvmFlag)
	{
#ifdef BUILD_WITH_LLVM
		auto fn = g_LLVMCompiler->Compile(stmts);
		g_LLVMCompiler->Run(fn);
#else
		ASSERT("Cannot run with llvm,not build yet.");
#endif
	}
	else
	{
		auto chunk = g_Compiler->Compile(stmts,isLineInterpret);
#ifdef _DEBUG
		chunk->Stringify();
#endif

		g_Vm->Run(chunk);
	}

	for (auto stmt : stmts)
		SAFE_DELETE(stmt);
}

void Repl()
{
	std::string line;

	std::cout << "> ";
	while (getline(std::cin, line))
	{
		if(line=="exit")
			return;
		if (line == "clear")
		{

			if (g_UseLlvmFlag)
			{
#ifdef BUILD_WITH_LLVM
				g_LLVMCompiler->ResetStatus();
#else
				ASSERT("Cannot run with llvm,not build yet.");
#endif
			}
			else
				g_Compiler->ResetStatus();
		}
		else
			Run(line,true);

		std::cout << "> ";
	}
}

void RunFile(std::string_view path)
{
	std::string content = ReadFile(path);
	Run(content,false);
}

void PrintUsage()
{
	std::cout << "Usage: ComputeDuck [option]:" << std::endl;
	std::cout << "-h or --help:show usage info." << std::endl;
	std::cout << "-l or --llvm:run source with llvm jit,like : ComputeDuck -l." << std::endl;
	std::cout << "-f or --file:run source file with a valid file path,like : ComputeDuck -f examples/array.cd." << std::endl;
}

void SetBasePath(std::string_view path)
{
	std::string curPath = std::filesystem::absolute(std::filesystem::path(path)).string();
#ifdef _WIN32
	curPath = curPath.substr(0, curPath.find_last_of('\\') + 1);
#else
	curPath = curPath.substr(0, curPath.find_last_of('/') + 1);
#endif
	BuiltinManager::GetInstance()->SetExecuteFilePath(curPath);
}

#undef main
int main(int argc, char** argv)
{
	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--llvm") == 0)
			g_UseLlvmFlag = true;

		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
				g_SourceFilePath = argv[++i];
			else
			{
				PrintUsage();
				return EXIT_FAILURE;
			}
		}

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			PrintUsage();
			return EXIT_FAILURE;
		}
	}

	if (g_SourceFilePath != "")
	{
		SetBasePath(g_SourceFilePath);
		RunFile(g_SourceFilePath);
	}
	else
	{
		SetBasePath(argv[0]);
		Repl();
	}

#ifdef BUILD_WITH_LLVM
	SAFE_DELETE(g_LLVMCompiler);
#endif

	SAFE_DELETE(g_Vm);
	SAFE_DELETE(g_Compiler);
	SAFE_DELETE(g_Parser);
	SAFE_DELETE(g_PreProcessor);

	return 0;
}