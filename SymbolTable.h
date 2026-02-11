#pragma once
#include <string>
#include <unordered_map>
#include "Utils.h"
#include "Value.h"

enum class SymbolScope
{
    GLOBAL,
    LOCAL,
    BUILTIN,
};

struct Symbol
{
    std::string_view name;
    bool isStructSymbol{ false };
    SymbolScope scope{ SymbolScope::GLOBAL };
    int32_t index{ -1 };
    int32_t scopeDepth{ 0 };
    int32_t isUpValue{ 0 };
};

class SymbolTable
{
public:
    SymbolTable() = default;

    SymbolTable(SymbolTable *upper)
        : m_Upper(upper), m_ScopeDepth(upper->m_ScopeDepth + 1)
    {
    }

    ~SymbolTable()
    {
    }

    Symbol Define(std::string_view name, bool isStructSymbol = false)
    {
        Symbol symbol;
        symbol.name = name;
        symbol.index = m_VarCount;
        symbol.scopeDepth = m_ScopeDepth;
        symbol.isStructSymbol=isStructSymbol;

        if (symbol.scopeDepth == 0)
            symbol.scope = SymbolScope::GLOBAL;
        else
            symbol.scope = SymbolScope::LOCAL;

        if (m_SymbolMaps.find(name) != m_SymbolMaps.end())
            ASSERT("Redefined variable:(%s) in current context.", name.data());

        m_SymbolMaps[name] = symbol;
        m_VarCount++;
        return symbol;
    }

    Symbol DefineBuiltin(std::string_view name)
    {
        auto iter = m_SymbolMaps.find(name);
        if (iter != m_SymbolMaps.end())
            return iter->second;

        Symbol symbol;
        symbol.name = name;
        symbol.scopeDepth = m_ScopeDepth;
        symbol.scope = SymbolScope::BUILTIN;

        m_SymbolMaps[name] = symbol;
        return symbol;
    }

    bool Resolve(std::string_view name, Symbol &symbol)
    {
        auto iter = m_SymbolMaps.find(name);
        if (iter != m_SymbolMaps.end())
        {
			if (m_ScopeDepth < iter->second.scopeDepth)
               return false;
            symbol = iter->second;
            return true;
        }
        else if (GetUpper())
        {
            bool isFound = GetUpper()->Resolve(name, symbol);
            if (!isFound)
                return false;
            if (symbol.scope == SymbolScope::GLOBAL || symbol.scope == SymbolScope::BUILTIN)
                return true;

            symbol.isUpValue = 1;

            m_SymbolMaps[symbol.name] = symbol;
            return true;
        }

        return false;
    }

    uint8_t GetVarCount() const
    {
        return m_VarCount;
    }

    SymbolTable *GetUpper() const
    {
        return m_Upper;
    }

    void EnterScope()
    {
        m_ScopeDepth++;
    }

    void ExitScope()
    {
        m_ScopeDepth--;
    }

private:
    SymbolTable *m_Upper{ nullptr };
    std::unordered_map<std::string_view, Symbol> m_SymbolMaps;
    uint8_t m_VarCount{ 0 };
    uint8_t m_ScopeDepth{ 0 };
};
