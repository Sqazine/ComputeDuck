#pragma once
#include <string>
#include <unordered_map>

#include "Utils.h"
#ifdef BUILD_WITH_LLVM
#include "llvm/IR/Instructions.h"
#else
#error "Cannot run with llvm,not build yet.";
#endif
enum class SymbolScope
{
    GLOBAL,
    LOCAL,
    BUILTIN,
};

struct Symbol
{
    Symbol() {}

    Symbol(std::string_view name, const SymbolScope& scope, int32_t index, int32_t scopeDepth = 0, bool isStructSymbol = false)
        : name(name), scope(scope), index(index), isStructSymbol(isStructSymbol), scopeDepth(scopeDepth)
    {
    }

#ifdef BUILD_WITH_LLVM
    Symbol(std::string_view name, const SymbolScope& scope, llvm::AllocaInst* allocation, int32_t scopeDepth = 0)
        : name(name), scope(scope), allocation(allocation), scopeDepth(scopeDepth)
    {
    }

    llvm::AllocaInst* allocation{nullptr};
#else
#error "Cannot run with llvm,not build yet.";
#endif

    std::string_view name;
    bool isStructSymbol{false};
    SymbolScope scope{ SymbolScope::GLOBAL };
    int32_t index{0};
    int32_t scopeDepth{0};
};

struct SymbolTable
{
    SymbolTable(){}

    SymbolTable(SymbolTable* enclosing)
        : enclosing(enclosing), scopeDepth(enclosing->scopeDepth + 1)
    {
    }

    ~SymbolTable()
    {
        auto p = enclosing;
        while (p)
        {
            auto q = p->enclosing;
            SAFE_DELETE(p);
            p = q;
        }
    }

#ifdef BUILD_WITH_LLVM
    void Set(std::string_view name, llvm::AllocaInst* a)
    {
        symbolMaps[name].allocation = a;
    }
#else
#error "Cannot run with llvm,not build yet.";
#endif

    Symbol Define(const std::string& name, bool isStructSymbol = false)
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

    Symbol DefineBuiltin(std::string_view name)
    {
        auto iter = symbolMaps.find(name);
        if (iter != symbolMaps.end())
            return iter->second;

        auto symbol = Symbol(name, SymbolScope::BUILTIN, -1, scopeDepth);
        symbolMaps[name] = symbol;
        return symbol;
    }

    bool Resolve(const std::string& name, Symbol& symbol)
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

            symbolMaps[symbol.name] = symbol;
            return true;
        }

        return false;
    }

    SymbolTable* enclosing{nullptr};
    std::unordered_map<std::string_view, Symbol> symbolMaps;
    uint8_t definitionCount{0};
    uint8_t scopeDepth{0};
};
