#include <string>
#include <string_view>
#include <filesystem>
#include <cstring>
#include "PreProcessor.h"
#include "Parser.h"
#include "Compiler.h"
#include "BuiltinManager.h"
#include "VM.h"
#include "VM.h"

PreProcessor *g_PreProcessor = nullptr;
Parser *g_Parser = nullptr;
Compiler *g_Compiler = nullptr;
VM *g_Vm = nullptr;

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

void Run(std::string_view content)
{
	auto tokens = g_PreProcessor->PreProcess(content);
#ifndef NDEBUG
	for (const auto &token : tokens)
		std::cout << token << std::endl;
#endif

	auto stmts = g_Parser->Parse(tokens);
#ifndef NDEBUG
	for (const auto &stmt : stmts)
		std::cout << stmt->Stringify() << std::endl;
#endif

	auto fn = g_Compiler->Compile(stmts);

#ifndef NDEBUG
	std::cout << Stringify(fn,true) << std::endl;
#endif

	for (auto stmt : stmts)
		SAFE_DELETE(stmt);

	g_Vm->Run(fn);
}

void Repl(std::string_view exePath)
{
	SetBasePath(exePath);

	std::string allLines;
	std::string line;

	std::cout << "> ";
	while (getline(std::cin, line))
	{
		if (line == "exit")
			return;
		else if (line == "clear")
			allLines.clear();
		else
		{
			allLines += line;
			Run(allLines);
		}

		std::cout << "> ";
	}
}

void RunFile(std::string_view path)
{
	SetBasePath(path);
	std::string content = ReadFile(path);
	Run(content);
}

int32_t PrintUsage()
{
	std::cout << "Usage: ComputeDuck [option]:" << std::endl;
	std::cout << "-h or --help:show usage info." << std::endl;
	std::cout << "-f or --file:run source file with a valid file path,like : ComputeDuck -f examples/array.cd." << std::endl;
	return EXIT_FAILURE;
}

#undef main
int32_t main(int argc, const char **argv)
{
	std::string_view sourceFilePath;
	for (size_t i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0)
		{
			if (i + 1 < argc)
				sourceFilePath = argv[++i];
			else
				return PrintUsage();
		}

		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return PrintUsage();
	}

	g_PreProcessor = new PreProcessor();
	g_Parser = new Parser();
	g_Compiler = new Compiler();
	g_Vm = new VM();

	if (!sourceFilePath.empty())
		RunFile(sourceFilePath);
	else
		Repl(argv[0]);

	SAFE_DELETE(g_Vm);
	SAFE_DELETE(g_Compiler);
	SAFE_DELETE(g_Parser);
	SAFE_DELETE(g_PreProcessor);

	return 0;
}