from enum import Enum


class TokenType(Enum):
    NUMBER = 0,
    STRING = 1,
    IDENTIFIER = 2,
    COMMA = 3,		   # ,
    DOT = 4,		   # .
    COLON = 5,		   # :
    SEMICOLON = 6,	   # ;
    LBRACKET = 7,	   # [
    RBRACKET = 8,	   # ]
    LBRACE = 9,		   # {
    RBRACE = 10,		   # }
    LPAREN = 11,		   # (
    RPAREN = 12,		   # )
    PLUS = 13,		   # +
    MINUS = 14,		   # -
    ASTERISK = 15,	   # *
    SLASH = 16,		   # /
    EQUAL = 17,		   # =
    LESS = 18,		   # <
    GREATER = 19,	   # >
    VBAR = 20,		  # |
    CARET = 21,		  # ^
    AMPERSAND = 22,	  # &
    TILDE = 23,		  # ~
    EQUAL_EQUAL = 24,   # ==
    LESS_EQUAL = 25,	   # <=
    GREATER_EQUAL = 26,  # >=
    BANG_EQUAL = 27,	   # !=
    IF = 28,			   # if
    ELSE = 29,		   # else
    TRUE = 30,		   # true
    FALSE = 31,		   # false
    NIL = 32,		   # nil
    WHILE = 33,		   # while
    FUNCTION = 34,	   # function
    RETURN = 35,		   # return
    AND = 36,		   # and
    OR = 37,			   # or
    NOT = 38,		   # not
    STRUCT = 39,		   # struct
    REF = 40,           # ref
    DLLIMPORT = 41,       # dllimport
    IMPORT = 42,         # import
    END = 43


class Token:
    type: TokenType
    literal: str
    line: int
    column: int
    filePath: str

    def __init__(self, type: TokenType, literal: str, line: int, column: int, filePath="") -> None:
        self.type = type
        self.literal = literal
        self.line = line
        self.filePath = filePath
        self.column = column

    def __str__(self):
        return self.literal + "," + str(self.line)
