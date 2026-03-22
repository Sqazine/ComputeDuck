from Utils import error, UINT8_COUNT, UPVALUE_COUNT
from enum import IntEnum


class SymbolScope(IntEnum):
    GLOBAL = 0,
    LOCAL = 1,
    UPVALUE = 2,
    BUILTIN = 3,


class Symbol:
    name: str
    isStructSymbol: bool
    scope: SymbolScope = SymbolScope.GLOBAL
    index: int
    upvalueIndex : int
    scopeDepth: int
    
    def __init__(self) -> None:
        self.name = ""
        self.isStructSymbol = False
        self.scope = SymbolScope.GLOBAL
        self.index = 0
        self.upvalueIndex = 0
        self.scopeDepth = 0

class SymbolTable:
    __upper = None
    __varList: list[Symbol] = [None]*UINT8_COUNT
    __varCount: int
    __localVarCount : int
    __globalVarCount: int
    __upvalueList: list[Symbol] = [None]*UPVALUE_COUNT
    __upvalueCount: int
    __scopeDepth: int

    def __init__(self, upper=None) -> None:
        self.__upper = upper
        self.__varList = [None]*UINT8_COUNT
        self.__varCount = 0
        self.__localVarCount = 0
        self.__globalVarCount = 0
        self.__upvalueList = [None]*UPVALUE_COUNT
        self.__upvalueCount = 0
        
        if self.__upper != None:
            self.__scopeDepth = self.__upper.__scopeDepth+1
        else:
            self.__scopeDepth = 0

    def define(self, name: str, isStructSymbol: bool = False) -> Symbol:

        if self.__varCount == UINT8_COUNT:
            error("Too many variables, max is {}".format(UINT8_COUNT))

        if self.__find_symbol(name) != None:
            error("Variable already defined in this scope:{}".format(name))

        symbol = Symbol()
        if self.__upper == None:
            symbol.scope = SymbolScope.GLOBAL
            symbol.index = self.__globalVarCount
            self.__globalVarCount += 1
        else:
            symbol.scope = SymbolScope.LOCAL
            symbol.index = self.__localVarCount
            self.__localVarCount +=1
            
        symbol.name = name
        symbol.scopeDepth = self.__scopeDepth
        symbol.isStructSymbol = isStructSymbol
        

        self.__varList[self.__varCount] = symbol
        self.__varCount += 1
        return symbol

    def define_builtin(self, name: str) -> Symbol:
        result = self.__find_symbol(name)
        if result != None:
            return result
        
        symbol = Symbol()
        symbol.name = name
        symbol.scope = SymbolScope.BUILTIN
        symbol.scopeDepth = self.__scopeDepth
        
        self.__varList[self.__varCount] = symbol
        self.__varCount += 1
        return symbol

    def resolve(self, name: str) -> tuple[bool, Symbol]:
        result = self.__find_symbol(name)
        if result != None:
            if self.__scopeDepth < result.scopeDepth:
                return False, None
            return True, result
        elif self.get_upper() != None:
            isFound, symbol = self.__upper.resolve(name)
            if isFound == False:
                return False, None
            if symbol.scope == SymbolScope.GLOBAL or symbol.scope == SymbolScope.BUILTIN:
                return True, symbol
            
            if self.__upvalueCount == UPVALUE_COUNT:
                error("Too many upvalues, max is {}".format(UPVALUE_COUNT))

            symbol.scope = SymbolScope.UPVALUE
            symbol.upvalueIndex = self.__upvalueCount
            
            self.__upvalueList[self.__upvalueCount] = symbol
            self.__upvalueCount += 1
            return True, symbol

        return False, None

    def enter_scope(self):
        if self.__scopeDepth >= UINT8_COUNT:
            error("Too many scope depth, max is {}".format(UINT8_COUNT))
        self.__scopeDepth += 1

    def exit_scope(self):
        if self.__scopeDepth == 0:
            error("Exit scope when scope depth is 0")
        self.__scopeDepth -= 1
    
    def get_local_var_count(self):
        return self.__localVarCount
    
    def get_upvalue_count(self):
        return self.__upvalueCount
    
    def get_upvalue_list(self):
        return self.__upvalueList

    def get_upper(self):
        return self.__upper

    def __find_symbol(self, name: str) -> Symbol:
        for i in range(0, self.__varCount):
            symbol = self.__varList[i]
            if symbol != None and symbol.name == name:
               return symbol
           
        for i in range(0, self.__upvalueCount):
            symbol = self.__upvalueList[i]
            if symbol != None and symbol.name == name:
               return symbol
        return None
