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
   EQUAL_EQUAL = 20,   # ==
   LESS_EQUAL = 21,	   # <=
   GREATER_EQUAL = 22,  # >=
   BANG_EQUAL = 23,	   # !=
   VAR = 24,		   # var
   IF = 25,			   # if
   ELSE = 26,		   # else
   TRUE = 27,		   # true
   FALSE = 28,		   # false
   NIL = 29,		   # nil
   WHILE = 30,		   # while
   FUNCTION = 31,	   # function
   RETURN = 32,		   # return
   AND = 33,		   # and
   OR = 34,			   # or
   NOT = 35,		   # not
   STRUCT = 36,		   # struct
   UNKNOWN = 37,
   END = 38


class Token:
    type: TokenType
    literal: str
    line: int

    def __init__(self, type: TokenType, literal: str, line: int) -> None:
        self.type = type
        self.literal = literal
        self.line = line

    def Print(self):
        print("{},{}", self.literal, self.line)
