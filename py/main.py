import sys
import Lexer
def Repl():
    print("> ",end="")
    line=input()
    while(True):
        print("> ",end="")
        line=input()
    
def RunFile(filePath):
    print(filePath)

if __name__ == "__main__":
    if len(sys.argv) == 2:
        RunFile(sys.argv[0])
    elif len(sys.argv) == 1:
        Repl()
    else:
        print("Usage: ComputeDuck [filepath]")
