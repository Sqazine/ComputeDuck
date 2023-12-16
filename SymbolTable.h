#pragma once
#include <string>
#include <unordered_map>
#include "Utils.h"
enum class SymbolScope
{
    GLOBAL,
    LOCAL,
    BUILTIN,
};

struct Symbol
{
    Symbol()
        : scope(SymbolScope::GLOBAL), index(0), scopeDepth(0), isInUpScope(0), isStructSymbol(false)
    {
    }

    Symbol(const std::string &name, const SymbolScope &scope, int32_t index, int32_t scopeDepth = 0, bool isStructSymbol = false)
        : name(name), scope(scope), index(index), isStructSymbol(isStructSymbol), scopeDepth(scopeDepth), isInUpScope(0)
    {
    }

    std::string name;
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

    Symbol Define(const std::string &name, bool isStructSymbol = false)
    {
        auto symbol = Symbol(name, SymbolScope::GLOBAL, definitionCount, scopeDepth, isStructSymbol);

        if (!enclosing)
            symbol.scope = SymbolScope::GLOBAL;
        else
            symbol.scope = SymbolScope::LOCAL;

        if (symbolMaps.find(name) != symbolMaps.end())
            ASSERT("Redefined variable:(%s) in current context.", name.data());

        symbolMaps[name] = symbol;
        definitionCount++;
        return symbol;
    }

    Symbol DefineBuiltin(const std::string &name, int32_t index)
    {
        auto symbol = Symbol(name, SymbolScope::BUILTIN, index, scopeDepth);
        symbolMaps[name] = symbol;
        return symbol;
    }

    bool Resolve(const std::string &name, Symbol &symbol)
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

            symbol.isInUpScope = 1;

            symbolMaps[symbol.name] = symbol;
            return true;
        }

        return false;
    }

    SymbolTable *enclosing;

    std::unordered_map<std::string, Symbol> symbolMaps;
    int32_t definitionCount;

    int32_t scopeDepth;
};