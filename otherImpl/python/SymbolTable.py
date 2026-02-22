from Utils import error, UINT8_COUNT
from enum import IntEnum


class SymbolScope(IntEnum):
    GLOBAL = 0,
    LOCAL = 1,
    BUILTIN = 2,


class Symbol:
    name: str
    isStructSymbol: bool
    scope: SymbolScope = SymbolScope.GLOBAL
    index: int
    scopeDepth: int
    isUpValue: bool

    def __init__(self, name="", scope: SymbolScope = SymbolScope.GLOBAL, index: int = 0, scopeDepth: int = 0, isStructSymbol: bool = False) -> None:
        self.name = name
        self.isStructSymbol = isStructSymbol
        self.scope = scope
        self.index = index
        self.scopeDepth = scopeDepth
        self.isUpValue = False


class SymbolTable:
    upper = None
    symbolList: list[Symbol] = [None]*UINT8_COUNT
    varCount: int
    scopeDepth: int

    def __init__(self, upper=None) -> None:
        self.upper = upper
        self.symbolList = [None]*UINT8_COUNT
        self.varCount = 0
        if self.upper != None:
            self.scopeDepth = self.upper.scopeDepth+1
        else:
            self.scopeDepth = 0

    def define(self, name: str, isStructSymbol: bool = False) -> Symbol:

        if self.varCount == UINT8_COUNT:
            error("Too many variables, max is {}".format(UINT8_COUNT))

        if self.__find_symbol(name) != None:
            error("Variable already defined in this scope:{}".format(name))

        symbol = Symbol(name, SymbolScope.GLOBAL, self.varCount,
                        self.scopeDepth, isStructSymbol)
        if self.upper == None:
            symbol.scope = SymbolScope.GLOBAL
        else:
            symbol.scope = SymbolScope.LOCAL

        self.symbolList[self.varCount] = symbol
        self.varCount += 1
        return symbol

    def define_builtin(self, name: str) -> Symbol:
        result = self.__find_symbol(name)
        if result != None:
            return result

        symbol = Symbol(name, SymbolScope.BUILTIN, -1, self.scopeDepth)
        self.symbolList[self.varCount] = symbol
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

            symbol.isUpValue = True
            self.symbolList[self.varCount] = symbol
            self.varCount += 1
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

    def get_upper(self):
        return self.upper

    def __find_symbol(self, name: str) -> Symbol:
        for i in range(0, self.varCount+1):
            symbol = self.symbolList[i]
            if symbol != None and symbol.name == name:
               return symbol
        return None
