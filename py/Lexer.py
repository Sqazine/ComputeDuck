from py.Token import TokenType


class Lexer:
    __startPos:int=0
    __curPos:int=0
    __line:int=1
    __source:str=""
    __tokens:list=[]
    
    def __init__(self) -> None:
        self.ResetStatus()
    
    def GenerateTokens(self, src:str)->list:
        self.ResetStatus()
        while(not self.IsAtEnd()):
            self.__startPos=self.__curPos
            self.GenerateToken()
        self.AddToken(TokenType.END,"EOF")
        
        return self.__tokens
        
    
    
    def ResetStatus(self)->None:
        self.__startPos=self.__curPos=0
        self.__line=1
        self.__tokens=[]
        self.__source=""