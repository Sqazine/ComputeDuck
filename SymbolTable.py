from Utils import Assert
from enum import IntEnum


class SymbolScope(IntEnum):
    GLOBAL = 0,
    LOCAL = 1,
    BUILTIN_FUNCTION = 2,
    BUILTIN_VARIABLE = 3,


class Symbol:
    name: str
    isStructSymbol: bool
    scope: SymbolScope = SymbolScope.GLOBAL
    index: int
    scopeDepth: int
    isInUpScope: int

    def __init__(self, name="", scope: SymbolScope = SymbolScope.GLOBAL, index: int = 0, scopeDepth: int = 0, isStructSymbol: bool = False) -> None:
        self.name = name
        self.isStructSymbol = isStructSymbol
        self.scope = scope
        self.index = index
        self.scopeDepth = scopeDepth
        self.isInUpScope = 0


class SymbolTable:
    enclosing = None
    symbolMaps: dict[str, Symbol] = {}
    definitionCount: int
    scopeDepth: int

    def __init__(self, enclosing=None) -> None:
        self.enclosing = enclosing
        self.symbolMaps = {}
        self.definitionCount = 0
        self.scopeDepth = 0
        if self.enclosing != None:
            self.scopeDepth = self.enclosing.scopeDepth+1

    def Define(self, name: str, isStructSymbol: bool = False) -> Symbol:
        symbol = Symbol(name, SymbolScope.GLOBAL,
                        self.definitionCount, self.scopeDepth, isStructSymbol)
        if self.enclosing == None:
            symbol.scope = SymbolScope.GLOBAL
        else:
            symbol.scope = SymbolScope.LOCAL

        if self.symbolMaps.get(name, None) != None:
            Assert("Redefined variable:("+name+") in current context.")

        self.symbolMaps[name] = symbol
        self.definitionCount += 1
        return symbol

    def DefineBuiltinFunction(self, name: str, index: int) -> Symbol:
        symbol = Symbol(name, SymbolScope.BUILTIN_FUNCTION,
                        index, self.scopeDepth)
        self.symbolMaps[name] = symbol
        return symbol

    def DefineBuiltinVariable(self, name: str, index: int) -> Symbol:
        symbol = Symbol(name, SymbolScope.BUILTIN_VARIABLE,
                        index, self.scopeDepth)
        self.symbolMaps[name] = symbol
        return symbol

    def Resolve(self, name: str):
        symbol = self.symbolMaps.get(name)
        if symbol != None:
            return True, symbol
        elif self.enclosing != None:
            isFound, symbol = self.enclosing.Resolve(name)
            if isFound == False:
                return False, None
            if symbol.scope == SymbolScope.GLOBAL or symbol.scope == SymbolScope.BUILTIN_FUNCTION:
                return True, symbol

            symbol.isInUpScope = 1
            self.symbolMaps[symbol.name] = symbol
            return True, symbol

        return False, None
