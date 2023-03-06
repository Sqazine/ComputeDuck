import sys
from Parser import Parser
from Compiler import Compiler
from VM import VM
from PreProcessor import PreProcessor
from Utils import ReadFile


def Repl():
    print("> ", end="")
    preProcessor = PreProcessor()
    parser = Parser()
    compiler = Compiler()
    vm = VM()
    line = input()
    while (True):
        if line == "clear":
            compiler.ResetStatus()
        else:
            tokens = preProcessor.PreProcess(line)
            for token in tokens:
                print(token)

            stmts = parser.Parse(tokens)

            for stmt in stmts:
                print(stmt)

            chunk = compiler.Compile(stmts)

            chunk.Stringify()

            vm.Run(chunk)

        print("> ", end="")
        line = input()


def RunFile(filePath):
    content = ReadFile(filePath)
    preProcessor = PreProcessor()
    parser = Parser()
    compiler = Compiler()
    vm = VM()
    tokens = preProcessor.PreProcess(content)
    for token in tokens:
        print(token)

    stmts = parser.Parse(tokens)
    for stmt in stmts:
        print(stmt)
    chunk = compiler.Compile(stmts)
    chunk.Stringify()
    vm.Run(chunk)


if __name__ == "__main__":
    if len(sys.argv) == 2:
        RunFile(sys.argv[1])
    elif len(sys.argv) == 1:
        Repl()
    else:
        print("Usage: ComputeDuck [filepath]")
