from Token import Token, TokenType
from Utils import Assert

keywords: dict = {
    "if": TokenType.IF,
    "else": TokenType.ELSE,
    "true": TokenType.TRUE,
    "false": TokenType.FALSE,
    "nil": TokenType.NIL,
    "while": TokenType.WHILE,
    "function": TokenType.FUNCTION,
    "return": TokenType.RETURN,
    "and": TokenType.AND,
    "or": TokenType.OR,
    "not": TokenType.NOT,
    "struct": TokenType.STRUCT,
    "ref": TokenType.REF,
    "dllimport": TokenType.DLLIMPORT,
    "import": TokenType.IMPORT,
}


def IsNumber(c) -> bool:
    return c >= '0' and c <= '9'


def IsLetter(c) -> bool:
    return (c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z') or c == '_'


def IsLetterOrNumber(c) -> bool:
    return IsLetter(c) or IsNumber(c)


class Lexer:
    __startPos: int = 0
    __curPos: int = 0
    __line: int = 1
    __source: str = ""
    __tokens: list = []
    __filePath: str = ""

    def __init__(self) -> None:
        self.ResetStatus()

    def GenerateTokens(self, src: str, filePath="RootFile") -> list:
        self.ResetStatus()
        self.__source = src
        self.__filePath = filePath
        while (not self.IsAtEnd()):
            self.__startPos = self.__curPos
            self.GenerateToken()

        return self.__tokens

    def ResetStatus(self) -> None:
        self.__startPos = self.__curPos = 0
        self.__line = 1
        self.__tokens = []
        self.__source = ""

    def IsMatchCurChar(self, c) -> bool:
        return self.GetCurChar() == c

    def IsMatchCurCharAndStepOnce(self, c) -> bool:
        result = self.GetCurChar() == c
        if result:
            self.__curPos += 1
        return result

    def IsMatchNextChar(self, c) -> bool:
        return self.GetNextChar() == c

    def IsMatchNextCharAndStepOnce(self, c) -> bool:
        result = self.GetNextChar() == c
        if result:
            self.__curPos += 1
        return result

    def GetNextCharAndStepOnce(self):
        if self.__curPos+1 < len(self.__source):
            self.__curPos += 1
            return self.__source[self.__curPos]
        return '\0'

    def GetNextChar(self):
        if self.__curPos+1 < len(self.__source):
            return self.__source[self.__curPos+1]
        return '\0'

    def GetCurCharAndStepOnce(self):
        res = '\0'
        if not self.IsAtEnd():
            res = self.__source[self.__curPos]
            self.__curPos += 1
        return res

    def GetCurChar(self):
        res = '\0'
        if not self.IsAtEnd():
            res = self.__source[self.__curPos]
        return res

    def AddToken(self, type: TokenType, literal: str = None):
        if (literal == None):
            literal = self.__source[self.__startPos:self.__curPos]
        self.__tokens.append(
            Token(type, literal, self.__line, self.__filePath))

    def IsAtEnd(self) -> bool:
        return self.__curPos >= len(self.__source)

    def Number(self):
        while IsNumber(self.GetCurChar()):
            self.GetCurCharAndStepOnce()
        if self.IsMatchCurCharAndStepOnce('.'):
            if IsNumber(self.GetCurChar()):
                while IsNumber(self.GetCurChar()):
                    self.GetCurCharAndStepOnce()
            else:
                Assert("[line " + self.__line + "]:Number cannot end with '.'")
        self.AddToken(TokenType.NUMBER)

    def Identifier(self):
        while IsLetterOrNumber(self.GetCurChar()):
            self.GetCurCharAndStepOnce()

        literal = self.__source[self.__startPos:self.__curPos]

        isKeyWord = False
        for key, value in keywords.items():
            if key == literal:
                self.AddToken(value, literal)
                isKeyWord = True
                break
        if not isKeyWord:
            self.AddToken(TokenType.IDENTIFIER, literal)

    def String(self):
        while (not self.IsMatchCurChar('\"')) and (not self.IsAtEnd()):
            if self.IsMatchCurChar('\n'):
                self.__line += 1
            self.GetCurCharAndStepOnce()
        if self.IsAtEnd():
            Assert("[line " + self.__line + "]:Uniterminated string.")
        self.GetCurCharAndStepOnce()  # eat the second '\"'

        self.AddToken(TokenType.STRING,
                      self.__source[self.__startPos+1:self.__curPos-1])

    def GenerateToken(self):
        c = self.GetCurCharAndStepOnce()

        if c == '(':
            self.AddToken(TokenType.LPAREN)
        elif c == ')':
            self.AddToken(TokenType.RPAREN)
        elif c == '[':
            self.AddToken(TokenType.LBRACKET)
        elif c == ']':
            self.AddToken(TokenType.RBRACKET)
        elif c == '{':
            self.AddToken(TokenType.LBRACE)
        elif c == '}':
            self.AddToken(TokenType.RBRACE)
        elif c == ',':
            self.AddToken(TokenType.COMMA)
        elif c == '.':
            self.AddToken(TokenType.DOT)
        elif c == ':':
            self.AddToken(TokenType.COLON)
        elif c == ';':
            self.AddToken(TokenType.SEMICOLON)
        elif c == '\"':
            self.String()
        elif c == ' ' or c == ' \t' or c == '\r':
            pass
        elif c == '\n':
            self.__line += 1
        elif c == '+':
            self.AddToken(TokenType.PLUS)
        elif c == '-':
            self.AddToken(TokenType.MINUS)
        elif c == '*':
            self.AddToken(TokenType.ASTERISK)
        elif c == '/':
            self.AddToken(TokenType.SLASH)
        elif c == '#':
            while (not self.IsMatchCurChar('\n')) and (not self.IsAtEnd()):
                self.GetCurCharAndStepOnce()
            self.__line += 1
        elif c == '!':
            if self.IsMatchCurCharAndStepOnce('='):
                self.AddToken(TokenType.BANG_EQUAL)
        elif c == '<':
            if self.IsMatchCurCharAndStepOnce('='):
                self.AddToken(TokenType.LESS_EQUAL)
            else:
                self.AddToken(TokenType.LESS)
        elif c == '>':
            if self.IsMatchCurCharAndStepOnce('='):
                self.AddToken(TokenType.GREATER_EQUAL)
            else:
                self.AddToken(TokenType.GREATER)
        elif c == '=':
            if self.IsMatchCurCharAndStepOnce('='):
                self.AddToken(TokenType.EQUAL_EQUAL)
            else:
                self.AddToken(TokenType.EQUAL)
        elif IsNumber(c):
            self.Number()
        elif IsLetter(c):
            self.Identifier()
        else:
            self.AddToken(TokenType.UNKNOWN)
