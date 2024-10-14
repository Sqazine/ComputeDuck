#pragma once
#include <string>
#include <array>
#include "Utils.h"
#include "Value.h"

enum class SymbolScope
{
    GLOBAL,
    // ++ 新增内容
    LOCAL,
    // -- 新增内容
};

struct Symbol
{
    std::string_view name;
    SymbolScope scope{SymbolScope::GLOBAL};
    // ++ 新增内容
    int32_t scopeDepth{0};
    // -- 新增内容
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
             // ++ 新增内容
            if (symbol->scopeDepth != m_ScopeDepth) // not in same scope
                continue;
            // -- 新增内容
            if (symbol->name == name)
                ASSERT("Variable already defined in this scope:%s", name.data());
        }

        if (m_DefinitionCount == UINT8_COUNT)
            ASSERT("Too many variable definitions, max is %d", UINT8_COUNT);

        Symbol symbol;
        symbol.name = name;
        symbol.index = m_DefinitionCount;
        // ++ 删除内容
        //symbol.scope = SymbolScope::GLOBAL;
        // -- 删除内容
        
        // ++ 新增内容
        symbol.scopeDepth = m_ScopeDepth;
        if (m_ScopeDepth == 0)
            symbol.scope = SymbolScope::GLOBAL;
        else
            symbol.scope = SymbolScope::LOCAL;
        // -- 新增内容

        m_SymbolList[m_DefinitionCount++] = symbol;
        return symbol;
    }

    bool Resolve(std::string_view name, Symbol &symbol)
    {
        for (int32_t i = m_DefinitionCount - 1; i >= 0; --i)
        {
            if (m_SymbolList[i].name == name)
            {
                // ++ 新增内容
                if(m_SymbolList[i].scopeDepth <= m_ScopeDepth)
                {
                    symbol = m_SymbolList[i];
                    return true;
                }
                 // -- 新增内容
            }
        }

        return false;
    }

    uint8_t GetDefinitionCount() const
    {
        return m_DefinitionCount;
    }

    // ++ 新增内容
    void BeginScope()
    {
        m_ScopeDepth++;
    }

    void EndScope()
    {
        m_ScopeDepth--;
    }
    // -- 新增内容

private:
    std::array<Symbol, UINT8_COUNT> m_SymbolList;
    uint8_t m_DefinitionCount{0};
    // ++ 新增内容
    uint8_t m_ScopeDepth{0};
    // -- 新增内容
};
