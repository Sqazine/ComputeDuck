#include <string>
#include <string_view>
#include <filesystem>
#include "PreProcessor.h"
#include "Parser.h"
#include "Compiler.h"
#include "VM.h"
#include "BuiltinManager.h"

PreProcessor* gPreProcessor = new PreProcessor();
Parser* gParser = new Parser();
Compiler * gCompiler = new Compiler();
VM* gVm = new VM();

void Run(std::string_view content)
{
	auto tokens = gPreProcessor->PreProcess(content);

#ifdef DEBUG
	for (const auto& token : tokens)
		std::cout << token << std::endl;
#endif

	auto stmts = gParser->Parse(tokens);

#ifdef DEBUG
	for (const auto& stmt : stmts)
		std::cout << stmt->Stringify() << std::endl;
#endif

	auto chunk = gCompiler->Compile(stmts);

	for (auto stmt : stmts)
		SAFE_DELETE(stmt);
		
#ifdef DEBUG
	chunk->Stringify();
#endif

	gVm->Run(chunk);
}

void Repl()
{
	std::string line;

	std::cout << "> ";
	while (getline(std::cin, line))
	{
		if (line == "clear")
			gCompiler->ResetStatus();
		else
			Run(line);
		std::cout << "> ";
	}
}

void RunFile(std::string_view path)
{
	std::string content = ReadFile(path);
	Run(content);
}

#undef main
int main(int argc, char** argv)
{
	if (argc == 2)
	{
		std::string curPath = std::filesystem::absolute(std::filesystem::path(argv[1])).string();
#ifdef _WIN32
		curPath = curPath.substr(0, curPath.find_last_of('\\') + 1);
#else
		curPath = curPath.substr(0, curPath.find_last_of('/') + 1);
#endif
		BuiltinManager::GetInstance()->SetExecuteFilePath(curPath);

		RunFile(argv[1]);
	}
	else if (argc == 1)
	{
		std::string curPath = std::filesystem::absolute(std::filesystem::path(argv[0])).string();
#ifdef _WIN32
		curPath = curPath.substr(0, curPath.find_last_of('\\') + 1);
#else
		curPath = curPath.substr(0, curPath.find_last_of('/') + 1);
#endif
		BuiltinManager::GetInstance()->SetExecuteFilePath(curPath);
		Repl();
	}
	else
		std::cout << "Usage: ComputeDuck [filepath]" << std::endl;
		
	SAFE_DELETE(gVm);
	SAFE_DELETE(gCompiler);
	SAFE_DELETE(gParser);
	SAFE_DELETE(gPreProcessor);

	return 0;
}