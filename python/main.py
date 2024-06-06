import sys
import os
from Parser import Parser
from Compiler import Compiler
from VM import VM
from PreProcessor import PreProcessor
from Utils import read_file
from BuiltinManager import gBuiltinManager

preProcessor = PreProcessor()
parser = Parser()
compiler = Compiler()
vm = VM()

def set_base_path(path:str):
    gBuiltinManager.set_execute_file_path(os.path.dirname(path)+"/")

def run(content:str):
    tokens = preProcessor.pre_process(content)
    for token in tokens:
        print(token)
    stmts = parser.parse(tokens)
    for stmt in stmts:
        print(stmt)
    chunk = compiler.compile(stmts)
    chunk.stringify()
    vm.run(chunk)

def repl(path:str):
    set_base_path(path)

    print("> ", end="")
    line = input()
    while (True):
        if line == "clear":
            compiler.reset_status()
        else:
            run(line)
        print("> ", end="")
        line = input()


def run_file(filePath):
    set_base_path(filePath)

    content = read_file(filePath)
    run(content)

if __name__ == "__main__":
    run_file("D:\\.sc\\ComputeDuck\\examples\\sdl2.cd")
    # if len(sys.argv) == 2:
    #     run_file(sys.argv[1])
    # elif len(sys.argv) == 1:
    #     repl(sys.argv[0])
    # else:
    #     print("Usage: ComputeDuck [filepath]")
