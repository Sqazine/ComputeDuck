#pragma once
#include <string>
#include <unordered_map>
#include "Utils.h"
#include "Value.h"

enum class SymbolScope
{
    GLOBAL,
};

struct Symbol
{
    SymbolScope scope{ SymbolScope::GLOBAL };
    int32_t index{ -1 };
};

class SymbolTable
{
public:
    SymbolTable() = default;

    ~SymbolTable() = default;

    Symbol Define(std::string_view name)
    {
        if (m_SymbolMaps.find(name) != m_SymbolMaps.end())
            ASSERT("Redefined variable:(%s) in current context.", name.data());

        Symbol symbol;
        symbol.index = m_DefinitionCount;
        symbol.scope = SymbolScope::GLOBAL;

        m_SymbolMaps[name] = symbol;
        m_DefinitionCount++;
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


        return false;
    }

    uint8_t GetDefinitionCount() const
    {
        return m_DefinitionCount;
    }

private:
    std::unordered_map<std::string_view, Symbol> m_SymbolMaps;
    uint8_t m_DefinitionCount{ 0 };
};
