import sys
from Lexer import Lexer
from Token import Token
from Parser import Parser
from Ast import AstNode
from Compiler import Compiler


def Repl():
    print("> ", end="")
    lexer = Lexer()
    parser = Parser()
    compiler=Compiler()
    line = input()
    while(True):
        tokens = lexer.GenerateTokens(line)
        for token in tokens:
            token.Print()

        stmts = parser.Parse(tokens)

        for stmt in stmts:
            print(stmt.Stringify())
            
        frame=compiler.Compile(stmts)
        
        print(frame.Stringify())

        print("> ", end="")
        line = input()


def RunFile(filePath):
    print(filePath)


if __name__ == "__main__":
    if len(sys.argv) == 2:
        RunFile(sys.argv[0])
    elif len(sys.argv) == 1:
        Repl()
    else:
        print("Usage: ComputeDuck [filepath]")
