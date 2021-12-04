import sys
from Lexer import Lexer
from Token import Token
from Parser import Parser
from Ast import AstNode
from Compiler import Compiler
from VM import VM
from Utils import ReadFile


def Repl():
    print("> ", end="")
    lexer = Lexer()
    parser = Parser()
    compiler = Compiler()
    vm = VM()
    line = input()
    while(True):
        if line == "clear":
            compiler.ResetStatus()
        elif line == "exit":
            break
        else:
            tokens = lexer.GenerateTokens(line)
            for token in tokens:
                token.Print()

            stmts = parser.Parse(tokens)

            for stmt in stmts:
                print(stmt.Stringify())

            frame = compiler.Compile(stmts)

            print(frame.Stringify())

            vm.ResetStatus()
            vm.Execute(frame)

        print("> ", end="")
        line = input()


def RunFile(filePath):
    content = ReadFile(filePath)
    lexer = Lexer()
    parser = Parser()
    compiler = Compiler()
    vm = VM()
    tokens = lexer.GenerateTokens(content)
    for token in tokens:
        token.Print()

    stmts = parser.Parse(tokens)

    for stmt in stmts:
        print(stmt.Stringify())

    frame = compiler.Compile(stmts)

    print(frame.Stringify())

    vm.Execute(frame)


if __name__ == "__main__":
    if len(sys.argv) == 2:
        RunFile(sys.argv[1])
    elif len(sys.argv) == 1:
        Repl()
    else:
        print("Usage: ComputeDuck [filepath]")
