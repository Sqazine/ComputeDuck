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
    fn = compiler.compile(stmts)

    print(fn.str_with_chunk())
    
    vm.run(fn)

def repl(path:str):
    set_base_path(path)

    allLines=""

    print("> ", end="")
    line = input()
    while (True):
        if line == "clear":
            allLines=""
        elif line=="exit":
            return
        else:
            allLines+=line
            run(allLines)
        print("> ", end="")
        line = input()


def run_file(filePath):
    set_base_path(filePath)

    content = read_file(filePath)
    run(content)

def print_usage():
    print("Usage: ComputeDuck [option]:")
    print("-h or --help:show usage info.")
    print("-f or --file:run source file with a valid file path,like : python3 main.py -f examples/array.cd.")
    exit(1)

if __name__ == "__main__":
    sourceFilePath=""
    for i in range(0,len(sys.argv)):
        if sys.argv[i]=="-f" or sys.argv[i]== "--file":
            if i+1<len(sys.argv):
                sourceFilePath=sys.argv[i+1]
            else:
                print_usage()
        if sys.argv[i]=="-h" or sys.argv[i]=="--help":
            print_usage()

    if sourceFilePath!="":
        run_file(sourceFilePath)
    else:
        repl(sys.argv[0])
