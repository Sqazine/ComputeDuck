#pragma once
#include <string_view>
#include <unordered_map>
enum class SymbolScope
{
    GLOBAL,
    LOCAL,
    BUILTIN_FUNCTION,
    BUILTIN_VARIABLE,
};

struct Symbol
{
    Symbol()
        : scope(SymbolScope::GLOBAL), index(0), scopeDepth(0), isInUpScope(0)
    {
    }

    Symbol(std::string_view name, const SymbolScope &scope, int32_t index, int32_t scopeDepth = 0, bool isStructSymbol = false)
        : name(name), scope(scope), index(index), isStructSymbol(isStructSymbol), scopeDepth(scopeDepth), isInUpScope(0)
    {
    }

    std::string_view name;
    bool isStructSymbol;
    SymbolScope scope;
    int32_t index;
    int32_t scopeDepth;
    int32_t isInUpScope;
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

    Symbol DefineBuiltinFunction(std::string_view name)
    {
        auto symbol = Symbol(name, SymbolScope::BUILTIN_FUNCTION, definitionCount, scopeDepth);
        symbolMaps[name] = symbol;
        definitionCount++;
        return symbol;
    }

    Symbol DefineBuiltinVariable(std::string_view name)
    {
        auto symbol = Symbol(name, SymbolScope::BUILTIN_VARIABLE, definitionCount, scopeDepth);
        symbolMaps[name] = symbol;
        definitionCount++;
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
            if (symbol.scope == SymbolScope::GLOBAL || symbol.scope == SymbolScope::BUILTIN_FUNCTION || symbol.scope == SymbolScope::BUILTIN_VARIABLE)
                return true;

            symbol.isInUpScope = 1;

            symbolMaps[symbol.name] = symbol;
            return true;
        }

        return false;
    }

    SymbolTable *enclosing;

    std::unordered_map<std::string_view, Symbol> symbolMaps;
    int32_t definitionCount;

    int32_t scopeDepth;
};