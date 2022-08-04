#pragma once
#include <string_view>
#include <unordered_map>
enum class SymbolScope
{
    GLOBAL,
    LOCAL,
    BUILTIN,
    UPVALUE,
    FUNCTION,
};

struct Symbol
{
    Symbol()
        : scope(SymbolScope::GLOBAL), index(0), scopeDepth(0)
    {
    }

    Symbol(std::string_view name, const SymbolScope &scope, int32_t index, int32_t scopeDepth = 0, bool isStructSymbol = false)
        : name(name), scope(scope), index(index), isStructSymbol(isStructSymbol), scopeDepth(scopeDepth)
    {
    }

    std::string_view name;
    bool isStructSymbol;
    SymbolScope scope;
    int32_t index;
    int32_t scopeDepth;
    int32_t upScopeLocation;
};

struct SymbolTable
{
    SymbolTable()
        : enclosing(nullptr), definitionCount(0), scopeDepth(0)
    {
    }

    SymbolTable(SymbolTable *enclosing)
        : enclosing(enclosing), definitionCount(0)
    {
        scopeDepth = enclosing->scopeDepth + 1;
    }

    ~SymbolTable()
    {
        auto p = enclosing;
        while (p)
        {
            auto q = p->enclosing;
            delete p;
            p = q;
        }
    }

    Symbol Define(std::string_view name, bool isStructSymbol = false)
    {
        auto symbol = Symbol(name, SymbolScope::GLOBAL, definitionCount, scopeDepth, isStructSymbol);

        if (!enclosing)
            symbol.scope = SymbolScope::GLOBAL;
        else
            symbol.scope = SymbolScope::LOCAL;

        if (symbolMaps.find(name) != symbolMaps.end())
            Assert("Redefined variable:(" + std::string(name) + ") in current context.");

        symbolMaps[name] = symbol;
        definitionCount++;
        return symbol;
    }

    Symbol DefineBuiltin(std::string_view name, int32_t index)
    {
        auto symbol = Symbol(name, SymbolScope::BUILTIN, index, scopeDepth);
        symbolMaps[name] = symbol;
        return symbol;
    }

    Symbol DefineUpValue(const Symbol &origin)
    {
        auto symbol = Symbol(origin.name, SymbolScope::UPVALUE, origin.index, origin.scopeDepth);
        symbol.upScopeLocation = origin.index;
        symbolMaps[symbol.name] = symbol;
        return symbol;
    }

    Symbol DefineFunction(std::string_view name)
    {
        auto symbol = Symbol(name, SymbolScope::FUNCTION, 0, scopeDepth);
        symbolMaps[name] = symbol;
        return symbol;
    }

    bool Resolve(std::string_view name, Symbol &symbol)
    {
        auto iter = symbolMaps.find(name);
        if (iter != symbolMaps.end())
        {
            symbol = iter->second;
            return true;
        }
        else if (enclosing)
        {
            bool isFound = enclosing->Resolve(name, symbol);
            if (!isFound)
                return false;
            if (symbol.scope == SymbolScope::GLOBAL || symbol.scope == SymbolScope::BUILTIN)
                return true;
            symbol = DefineUpValue(symbol);
            return true;
        }

        return false;
    }

    SymbolTable *enclosing;

    std::unordered_map<std::string_view, Symbol> symbolMaps;
    int32_t definitionCount;

    int32_t scopeDepth;
};