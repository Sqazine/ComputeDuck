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
    bool isStructSymbol{false};
    SymbolScope scope{SymbolScope::GLOBAL};
    int32_t index{-1};
    int32_t scopeDepth{0};
    bool isUpValue{false};
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
        if (m_VarCount == UINT8_COUNT)
            ASSERT("Too many variable definitions, max is %d", UINT8_COUNT);

        if (FindSymbolReference(name))
            ASSERT("Variable already defined in this scope:%s", name.data());

        Symbol symbol;
        symbol.name = name;
        symbol.index = m_VarCount;
        symbol.scopeDepth = m_ScopeDepth;
        symbol.isStructSymbol = isStructSymbol;

        if (symbol.scopeDepth == 0)
            symbol.scope = SymbolScope::GLOBAL;
        else
            symbol.scope = SymbolScope::LOCAL;

        m_SymbolList[m_VarCount++] = symbol;
        return symbol;
    }

    Symbol DefineBuiltin(std::string_view name)
    {
        if (Symbol *result = FindSymbolReference(name))
            return *result;

        if (m_VarCount == UINT8_COUNT)
            ASSERT("Too many variable definitions, max is %d", UINT8_COUNT);

        Symbol symbol;
        symbol.name = name;
        symbol.scopeDepth = m_ScopeDepth;
        symbol.scope = SymbolScope::BUILTIN;

        m_SymbolList[m_VarCount++] = symbol;
        return symbol;
    }

    bool Resolve(std::string_view name, Symbol &symbol)
    {
        if (Symbol *result = FindSymbolReference(name))
        {
            if (m_ScopeDepth < result->scopeDepth)
                return false;
            symbol = *result;
            return true;
        }
        else if (GetUpper())
        {
            bool isFound = GetUpper()->Resolve(name, symbol);
            if (!isFound)
                return false;
            if (symbol.scope == SymbolScope::GLOBAL || symbol.scope == SymbolScope::BUILTIN)
                return true;

            symbol.isUpValue = true;

            m_SymbolList[m_VarCount++] = symbol;
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
        if(m_ScopeDepth == UINT8_COUNT)
            ASSERT("Too many scope depth, max is %d", UINT8_COUNT);

        m_ScopeDepth++;
    }

    void ExitScope()
    {
        if(m_ScopeDepth == 0)
            ASSERT("Exit scope when scope depth is 0");

        m_ScopeDepth--;
    }

private:
    Symbol *FindSymbolReference(std::string_view name)
    {
        for (uint8_t i = 0; i <= m_VarCount; ++i)
        {
            if (m_SymbolList[i].name == name)
                return &m_SymbolList[i];
        }
        return nullptr;
    }

    SymbolTable *m_Upper{nullptr};
    std::array<Symbol, UINT8_COUNT> m_SymbolList;
    uint8_t m_VarCount{0};
    uint8_t m_ScopeDepth{0};
};
