#include <string>
#include <string_view>
#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"
#include "VM.h"
void Repl()
{
	std::string line;
	Lexer lexer;
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
			auto tokens = lexer.GenerateTokens(line);

			for (const auto &token : tokens)
				std::cout << token << std::endl;

			auto stmt = parser.Parse(tokens);

			std::cout << stmt->Stringify() << std::endl;

			Frame frame = compiler.Compile(stmt);

			std::cout << frame.Stringify() << std::endl;

			vm.ResetStatus();
			vm.Execute(frame);
		}
		std::cout << "> ";
	}
}

void RunFile(std::string path)
{
	std::string content = ReadFile(path);
	Lexer lexer;
	Parser parser;
	Compiler compiler;
	VM vm;

	auto tokens = lexer.GenerateTokens(content);

	for (const auto &token : tokens)
		std::cout << token << std::endl;

	auto stmt = parser.Parse(tokens);

	std::cout << stmt->Stringify() << std::endl;

	Frame frame = compiler.Compile(stmt);

	std::cout << frame.Stringify() << std::endl;

	vm.Execute(frame);
}

int main(int argc, char **argv)
{
	if (argc == 2)
		RunFile(argv[1]);
	else if (argc == 1)
		Repl();
	else
		std::cout << "Usage: ComputeDuck [filepath]" << std::endl;

	return 0;
}