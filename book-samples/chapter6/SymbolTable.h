#pragma once
#include <string>
#include <array>
#include "Utils.h"
#include "Value.h"

enum class SymbolScope
{
    GLOBAL,
};

struct Symbol
{
     std::string_view name;
    SymbolScope scope{SymbolScope::GLOBAL};
    int32_t index{-1};
};

class SymbolTable
{
public:
    SymbolTable() = default;

    ~SymbolTable() = default;

    Symbol Define(std::string_view name)
    {
        for (int32_t i = m_DefinitionCount - 1; i >= 0; --i)
        {
            Symbol *symbol = &m_SymbolList[i];
            if (symbol->name == name)
                ASSERT("Variable already defined in this scope:%s", name.data());
        }

        if (m_DefinitionCount == UINT8_COUNT)
            ASSERT("Too many variable definitions, max is %d", UINT8_COUNT);

        Symbol symbol;
        symbol.name = name;
        symbol.index = m_DefinitionCount;
        symbol.scope = SymbolScope::GLOBAL;

        m_SymbolList[m_DefinitionCount++] = symbol;
        return symbol;
    }

    bool Resolve(std::string_view name, Symbol &symbol)
    {
        for (int32_t i = m_DefinitionCount - 1; i >= 0; --i)
        {
            if (m_SymbolList[i].name == name)
            {
                symbol = m_SymbolList[i];
                return true;
            }
        }

        return false;
    }

    uint8_t GetDefinitionCount() const
    {
        return m_DefinitionCount;
    }

private:
    std::array<Symbol, UINT8_COUNT> m_SymbolList;
    uint8_t m_DefinitionCount{0};
};
