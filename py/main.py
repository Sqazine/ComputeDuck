import sys
from Lexer import Lexer
def Repl():
    print("> ",end="")
    line=input()
    while(True):
        lexer=Lexer()
        tokens=lexer.GenerateTokens(line)
        for token in tokens:
            token.Print()
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
