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

    SymbolTable(SymbolTable *enclosing)
        : m_Enclosing(enclosing), m_ScopeDepth(m_Enclosing->m_ScopeDepth + 1)
    {
    }

    ~SymbolTable()
    {
        auto p = m_Enclosing;
        while (p)
        {
            auto q = p->m_Enclosing;
            SAFE_DELETE(p);
            p = q;
        }
    }

    Symbol Define(std::string_view name, bool isStructSymbol = false)
    {
        Symbol symbol;
        symbol.name = name;
        symbol.index = m_DefinitionCount;
        symbol.scopeDepth = m_ScopeDepth;

        if (!m_Enclosing)
            symbol.scope = SymbolScope::GLOBAL;
        else
            symbol.scope = SymbolScope::LOCAL;

        if (m_SymbolMaps.find(name) != m_SymbolMaps.end())
            ASSERT("Redefined variable:(%s) in current context.", name.data());

        m_SymbolMaps[name] = symbol;
        m_DefinitionCount++;
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
            symbol = iter->second;
            return true;
        }
        else if (m_Enclosing)
        {
            bool isFound = m_Enclosing->Resolve(name, symbol);
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

    uint8_t GetDefinitionCount() const
    {
        return m_DefinitionCount;
    }

    SymbolTable *GetEnclosing() const
    {
        return m_Enclosing;
    }

private:
    SymbolTable *m_Enclosing{ nullptr };
    std::unordered_map<std::string_view, Symbol> m_SymbolMaps;
    uint8_t m_DefinitionCount{ 0 };
    uint8_t m_ScopeDepth{ 0 };
};
