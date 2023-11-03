#include <string>
#include <string_view>
#include <filesystem>
#include "PreProcessor.h"
#include "Parser.h"
#include "Compiler.h"
#include "VM.h"
#include "BuiltinManager.h"
void Repl()
{
	std::string line;

	BuiltinManager::GetInstance()->Init();

	PreProcessor preProcessor;
	Parser parser;
	Compiler compiler;
	VM vm;

	std::cout << "> ";
	while (getline(std::cin, line))
	{
		if (line == "clear")
			compiler.ResetStatus();
		else
		{
			auto tokens = preProcessor.PreProcess(line);

			for (const auto& token : tokens)
				std::cout << token << std::endl;

			auto stmts = parser.Parse(tokens);

			for (const auto& stmt : stmts)
				std::cout << stmt->Stringify() << std::endl;

			auto chunk = compiler.Compile(stmts);

			chunk.Stringify();

			vm.Run(chunk);
		}
		std::cout << "> ";
	}

	BuiltinManager::GetInstance()->Release();
}

void RunFile(std::string path)
{
	std::string content = ReadFile(path);

	BuiltinManager::GetInstance()->Init();

	PreProcessor preProcessor;
	Parser parser;
	Compiler compiler;
	VM vm;

	auto tokens = preProcessor.PreProcess(content);

	for (const auto& token : tokens)
		std::cout << token << std::endl;

	auto stmt = parser.Parse(tokens);

	auto stmts = parser.Parse(tokens);

	for (const auto& stmt : stmts)
		std::cout << stmt->Stringify() << std::endl;

	auto chunk = compiler.Compile(stmts);

	chunk.Stringify();

	vm.Run(chunk);

	BuiltinManager::GetInstance()->Release();
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

	return 0;
}