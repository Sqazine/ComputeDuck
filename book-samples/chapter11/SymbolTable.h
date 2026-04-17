#pragma once
#include <string>
#include <array>
#include "Utils.h"
#include "Value.h"

enum class SymbolScope
{
    GLOBAL,
    LOCAL,
    // ++ 新增内容
    BUILTIN,
    // -- 新增内容
};

struct Symbol
{
    std::string_view name;
    SymbolScope scope{SymbolScope::GLOBAL};
    int32_t scopeDepth{0};
    int32_t index{-1};
};

class SymbolTable
{
public:
    SymbolTable() = default;

    SymbolTable(SymbolTable *upper)
        : m_Upper(upper), m_ScopeDepth(upper->m_ScopeDepth + 1)
    {
        if (m_ScopeDepth == UINT8_COUNT)
            ASSERT("Too many scope depth, max is %d", UINT8_COUNT);
    }

    ~SymbolTable()
    {
        m_Upper = nullptr;
    }

    Symbol Define(std::string_view name)
    {
        if (m_VarCount == UINT8_COUNT)
            ASSERT("Too many variable definitions, max is %d", UINT8_COUNT);

        if (auto symbol = FindSymbolReference(name))
            if (symbol && symbol->scopeDepth == m_ScopeDepth)
                ASSERT("Variable already defined in this scope:%s", name.data());

        Symbol symbol;
        symbol.name = name;

        symbol.scopeDepth = m_ScopeDepth;
        if (m_ScopeDepth == 0)
        {
            symbol.scope = SymbolScope::GLOBAL;
            symbol.index = m_GlobalVarCount++;
        }
        else
        {
            symbol.scope = SymbolScope::LOCAL;
            symbol.index = m_LocalVarCount++;
        }

        m_VarList[m_VarCount++] = symbol;
        return symbol;
    }

    // ++ 新增内容
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

        m_VarList[m_VarCount++] = symbol;
        return symbol;
    }
    // -- 新增内容

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
            if (!GetUpper()->Resolve(name, symbol))
                return false;
            if (symbol.scope == SymbolScope::GLOBAL)
                return true;
        }

        return false;
    }

    uint8_t GetVarCount() const
    {
        return m_VarCount;
    }

    uint8_t GetLocalVarCount() const
    {
        return m_LocalVarCount;
    }

    SymbolTable *GetUpper() const
    {
        return m_Upper;
    }

    void EnterScope()
    {
        if (m_ScopeDepth == UINT8_COUNT)
            ASSERT("Too many scope depth, max is %d", UINT8_COUNT);

        m_ScopeDepth++;
    }

    void ExitScope()
    {
        if (m_ScopeDepth == 0)
            ASSERT("Exit scope when scope depth is 0");
        m_ScopeDepth--;
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

    SymbolTable *m_Upper{nullptr};

    std::array<Symbol, UINT8_COUNT> m_VarList;
    uint8_t m_VarCount{0};

    uint8_t m_LocalVarCount{0};
    uint8_t m_GlobalVarCount{0};
    uint8_t m_ScopeDepth{0};
};
