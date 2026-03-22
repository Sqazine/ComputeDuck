#pragma once
#include <string>
#include <array>
#include "Utils.h"
#include "Value.h"
// ++ 新增内容
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
        for (int32_t i = m_VarCount - 1; i >= 0; --i)
        {
            Symbol *symbol = &m_VarList[i];
            if (symbol->name == name)
                ASSERT("Variable already defined in this scope:%s", name.data());
        }

        if (m_VarCount == UINT8_COUNT)
            ASSERT("Too many variable definitions, max is %d", UINT8_COUNT);

        Symbol symbol;
        symbol.name = name;
        symbol.index = m_VarCount;
        symbol.scope = SymbolScope::GLOBAL;

        m_VarList[m_VarCount++] = symbol;
        return symbol;
    }

    bool Resolve(std::string_view name, Symbol &symbol)
    {
        if (Symbol *result = FindSymbolReference(name))
        {
            symbol = *result;
            return true;
        }

        return false;
    }

    uint8_t GetVarCount() const
    {
        return m_VarCount;
    }

private:
    Symbol *FindSymbolReference(std::string_view name)
    {
        for (uint8_t i = 0; i <= m_VarCount; ++i)
        {
            if (m_VarList[i].name == name)
                return &m_VarList[i];
        }
        return nullptr;
    }

    std::array<Symbol, UINT8_COUNT> m_VarList;
    uint8_t m_VarCount{0};
};

// -- 新增内容