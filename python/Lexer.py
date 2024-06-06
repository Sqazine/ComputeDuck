from Token import Token, TokenType
from Utils import error

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

class Lexer:
    __startPos: int = 0
    __curPos: int = 0
    __line: int = 1
    __source: str = ""
    __tokens: list[Token] = []
    __filePath: str = ""

    def __init__(self) -> None:
        self.__reset_status()

    def generate_tokens(self, src: str, filePath="RootFile") -> list:
        self.__reset_status()
        self.__source = src
        self.__filePath = filePath
        while (not self.__is_at_end()):
            self.__startPos = self.__curPos
            self.__generate_token()

        return self.__tokens

    def __reset_status(self) -> None:
        self.__startPos = self.__curPos = 0
        self.__line = 1
        self.__tokens = []
        self.__source = ""

    def __is_match_cur_char(self, c) -> bool:
        return self.__get_cur_char() == c

    def __is_match_cur_char_and_step_once(self, c) -> bool:
        result = self.__get_cur_char() == c
        if result:
            self.__curPos += 1
        return result

    def __get_cur_char_and_step_once(self):
        res = '\0'
        if not self.__is_at_end():
            res = self.__source[self.__curPos]
            self.__curPos += 1
        return res

    def __get_cur_char(self):
        res = '\0'
        if not self.__is_at_end():
            res = self.__source[self.__curPos]
        return res

    def __add_token(self, type: TokenType, literal: str = None):
        if (literal == None):
            literal = self.__source[self.__startPos:self.__curPos]
        self.__tokens.append(
            Token(type, literal, self.__line, self.__filePath))

    def __is_at_end(self) -> bool:
        return self.__curPos >= len(self.__source)

    def __number(self):
        while self.__is_number(self.__get_cur_char()):
            self.__get_cur_char_and_step_once()
        if self.__is_match_cur_char_and_step_once('.'):
            if self.__is_number(self.__get_cur_char()):
                while self.__is_number(self.__get_cur_char()):
                    self.__get_cur_char_and_step_once()
            else:
                error("[line " + self.__line + "]:Number cannot end with '.'")
        self.__add_token(TokenType.NUMBER)

    def __identifier(self):
        while self.__is_letter_or_number(self.__get_cur_char()):
            self.__get_cur_char_and_step_once()

        literal = self.__source[self.__startPos:self.__curPos]

        isKeyWord = False
        for key, value in keywords.items():
            if key == literal:
                self.__add_token(value, literal)
                isKeyWord = True
                break
        if not isKeyWord:
            self.__add_token(TokenType.IDENTIFIER, literal)

    def __string(self):
        while (not self.__is_match_cur_char('\"')) and (not self.__is_at_end()):
            if self.__is_match_cur_char('\n'):
                self.__line += 1
            self.__get_cur_char_and_step_once()
        if self.__is_at_end():
            error("[line " + self.__line + "]:Uniterminated string.")
        self.__get_cur_char_and_step_once()  # eat the second '\"'

        self.__add_token(TokenType.STRING,
                      self.__source[self.__startPos+1:self.__curPos-1])

    def __generate_token(self):
        c = self.__get_cur_char_and_step_once()

        if c == '(':
            self.__add_token(TokenType.LPAREN)
        elif c == ')':
            self.__add_token(TokenType.RPAREN)
        elif c == '[':
            self.__add_token(TokenType.LBRACKET)
        elif c == ']':
            self.__add_token(TokenType.RBRACKET)
        elif c == '{':
            self.__add_token(TokenType.LBRACE)
        elif c == '}':
            self.__add_token(TokenType.RBRACE)
        elif c == ',':
            self.__add_token(TokenType.COMMA)
        elif c == '.':
            self.__add_token(TokenType.DOT)
        elif c == ':':
            self.__add_token(TokenType.COLON)
        elif c == ';':
            self.__add_token(TokenType.SEMICOLON)
        elif c == '\"':
            self.__string()
        elif c == ' ' or c == ' \t' or c == '\r':
            pass
        elif c == '\n':
            self.__line += 1
        elif c == '+':
            self.__add_token(TokenType.PLUS)
        elif c == '-':
            self.__add_token(TokenType.MINUS)
        elif c == '*':
            self.__add_token(TokenType.ASTERISK)
        elif c == '/':
            self.__add_token(TokenType.SLASH)
        elif c == '&':
            self.__add_token(TokenType.AMPERSAND)
        elif c == '|':
            self.__add_token(TokenType.VBAR)
        elif c == '~':
            self.__add_token(TokenType.TILDE)
        elif c == '^':
            self.__add_token(TokenType.CARET)
        elif c == '#':
            while (not self.__is_match_cur_char('\n')) and (not self.__is_at_end()):
                self.__get_cur_char_and_step_once()
            self.__line += 1
        elif c == '!':
            if self.__is_match_cur_char_and_step_once('='):
                self.__add_token(TokenType.BANG_EQUAL)
        elif c == '<':
            if self.__is_match_cur_char_and_step_once('='):
                self.__add_token(TokenType.LESS_EQUAL)
            else:
                self.__add_token(TokenType.LESS)
        elif c == '>':
            if self.__is_match_cur_char_and_step_once('='):
                self.__add_token(TokenType.GREATER_EQUAL)
            else:
                self.__add_token(TokenType.GREATER)
        elif c == '=':
            if self.__is_match_cur_char_and_step_once('='):
                self.__add_token(TokenType.EQUAL_EQUAL)
            else:
                self.__add_token(TokenType.EQUAL)
        elif self.__is_number(c):
            self.__number()
        elif self.__is_letter(c):
            self.__identifier()
        else:
            self.__add_token(TokenType.END)

    def __is_number(self,c) -> bool:
        return c >= '0' and c <= '9'

    def __is_letter(self,c) -> bool:
        return (c >= 'A' and c <= 'Z') or (c >= 'a' and c <= 'z') or c == '_'

    def __is_letter_or_number(self,c) -> bool:
        return self.__is_letter(c) or self.__is_number(c)
