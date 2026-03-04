from Utils import error, UINT8_COUNT
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
    upper = None
    varList: list[Symbol] = [None]*UINT8_COUNT
    varCount: int
    upvalueList: list[Symbol] = [None]*UINT8_COUNT
    upvalueCount: int
    scopeDepth: int

    def __init__(self, upper=None) -> None:
        self.upper = upper
        self.varList = [None]*UINT8_COUNT
        self.varCount = 0
        self.upvalueList = [None]*UINT8_COUNT
        self.upvalueCount = 0
        
        if self.upper != None:
            self.scopeDepth = self.upper.scopeDepth+1
        else:
            self.scopeDepth = 0

    def define(self, name: str, isStructSymbol: bool = False) -> Symbol:

        if self.varCount == UINT8_COUNT:
            error("Too many variables, max is {}".format(UINT8_COUNT))

        if self.__find_symbol(name) != None:
            error("Variable already defined in this scope:{}".format(name))

        symbol = Symbol()
        symbol.name = name
        symbol.scope = SymbolScope.GLOBAL
        symbol.index = self.varCount
        symbol.scopeDepth = self.scopeDepth
        symbol.isStructSymbol = isStructSymbol
        
        if self.upper == None:
            symbol.scope = SymbolScope.GLOBAL
        else:
            symbol.scope = SymbolScope.LOCAL

        self.varList[self.varCount] = symbol
        self.varCount += 1
        return symbol

    def define_builtin(self, name: str) -> Symbol:
        result = self.__find_symbol(name)
        if result != None:
            return result
        
        symbol = Symbol()
        symbol.name = name
        symbol.scope = SymbolScope.BUILTIN
        symbol.scopeDepth = self.scopeDepth
        
        self.varList[self.varCount] = symbol
        self.varCount += 1
        return symbol

    def resolve(self, name: str) -> tuple[bool, Symbol]:
        result = self.__find_symbol(name)
        if result != None:
            if self.scopeDepth < result.scopeDepth:
                return False, None
            return True, result
        elif self.get_upper() != None:
            isFound, symbol = self.upper.resolve(name)
            if isFound == False:
                return False, None
            if symbol.scope == SymbolScope.GLOBAL or symbol.scope == SymbolScope.BUILTIN:
                return True, symbol

            symbol.scope = SymbolScope.UPVALUE
            symbol.upvalueIndex = self.upvalueCount
            self.upvalueList[self.upvalueCount] = symbol
            self.upvalueCount += 1
            return True, symbol

        return False, None

    def enter_scope(self):
        if self.scopeDepth >= UINT8_COUNT:
            error("Too many scope depth, max is {}".format(UINT8_COUNT))
        self.scopeDepth += 1

    def exit_scope(self):
        if self.scopeDepth == 0:
            error("Exit scope when scope depth is 0")
        self.scopeDepth -= 1

    def get_var_count(self):
        return self.varCount
    
    def get_upvalue_count(self):
        return self.upvalueCount
    
    def get_upvalue_list(self):
        return self.upvalueList

    def get_upper(self):
        return self.upper

    def __find_symbol(self, name: str) -> Symbol:
        for i in range(0, self.varCount):
            symbol = self.varList[i]
            if symbol != None and symbol.name == name:
               return symbol
           
        for i in range(0, self.upvalueCount):
            symbol = self.upvalueList[i]
            if symbol != None and symbol.name == name:
               return symbol
        return None
