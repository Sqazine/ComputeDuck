#pragma once
#include <string>
#include <array>
#include "Utils.h"
#include "Value.h"

enum class SymbolScope
{
    GLOBAL,
    
    LOCAL,
    
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

    ~SymbolTable() = default;

    Symbol Define(std::string_view name)
    {
        for (int32_t i = m_VarCount - 1; i >= 0; --i)
        {
            Symbol *symbol = &m_VarList[i];
             
            if (symbol->scopeDepth != m_ScopeDepth) // not in same scope
                continue;
            
            if (symbol->name == name)
                ASSERT("Variable already defined in this scope:%s", name.data());
        }

        if (m_VarCount == UINT8_COUNT)
            ASSERT("Too many variable definitions, max is %d", UINT8_COUNT);

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

    bool Resolve(std::string_view name, Symbol &symbol)
    {
       if (Symbol *result = FindSymbolReference(name))
        {
            
            if (m_ScopeDepth < result->scopeDepth)
                return false;
            
            symbol = *result;
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

    std::array<Symbol, UINT8_COUNT> m_VarList;
    uint8_t m_VarCount{0};
    
    uint8_t m_LocalVarCount{0};
    uint8_t m_GlobalVarCount{0};
    uint8_t m_ScopeDepth{0};
    
};
