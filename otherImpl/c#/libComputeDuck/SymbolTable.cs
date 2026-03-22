using System.Collections.Generic;

namespace ComputeDuck
{
    public enum SymbolScope
    {
        GLOBAL,
        LOCAL,
        UPVALUE,
        BUILTIN
    }

    public class Symbol
    {
        public Symbol()
        {
            this.scope = SymbolScope.GLOBAL;
            this.index = 0;
            this.upvalueIndex = 0;
            this.scopeDepth = 0;
            this.name = "";
        }

        public string name;
        public bool isStructSymbol;
        public SymbolScope scope;
        public int index;
        public int upvalueIndex;
        public int scopeDepth;
    }

    public class SymbolTable
    {

        public SymbolTable(SymbolTable m_Upper = null)
        {
            this.m_Upper = m_Upper;
            this.m_VarList = new Symbol[Utils.UINT8_COUNT];
            this.m_VarCount = 0;
            this.m_LocalVarCount = 0;
            this.m_UpvalueList = new Symbol[Utils.UPVALUE_COUNT];
            this.m_UpvalueCount = 0;

            if(m_Upper == null)
                this.m_ScopeDepth = 0;
            else
                this.m_ScopeDepth = m_Upper.m_ScopeDepth + 1;
        }

        public Symbol Define(string name, bool isStructSymbol = false)
        {
            if (m_VarCount >= Utils.UINT8_COUNT)
                Utils.Assert("Too many variable definitions, max is " + Utils.UINT8_COUNT.ToString());

            if (FindSymbol(name) != null)
                Utils.Assert("Variable already defined in this scope:" + name);

            Symbol symbol =new Symbol();

            if (m_Upper == null)
            {
                symbol.scope = SymbolScope.GLOBAL;
                symbol.index = m_GlobalVarCount++;
            }
            else
            {
                symbol.scope = SymbolScope.LOCAL;
                symbol.index = m_LocalVarCount++;
            }

            symbol.name = name;
            symbol.scopeDepth = m_ScopeDepth;
            symbol.isStructSymbol = isStructSymbol;

            m_VarList[m_VarCount++] = symbol;
            return symbol;
        }

        public Symbol DefineBuiltin(string name)
        {
            Symbol result = FindSymbol(name);
            if (result != null)
                return result;

            if (m_VarCount == Utils.UINT8_COUNT)
                Utils.Assert("Too many variable definitions, max is " + Utils.UINT8_COUNT.ToString());

            Symbol symbol = new Symbol();
            symbol.name = name;
            symbol.scope = SymbolScope.BUILTIN;
            symbol.scopeDepth = m_ScopeDepth;

            m_VarList[m_VarCount++] = symbol;
            return symbol;
        }

        public (bool, Symbol?) Resolve(string name)
        {
            Symbol result = FindSymbol(name);
            if (result != null)
            {
                if (m_ScopeDepth < result.scopeDepth)
                    return (false, null);
                return (true, result);
            }
            else if (GetUpper() != null)
            {
                var (isFound, symbol) = GetUpper().Resolve(name);
                if (!isFound)
                    return (false, null);
                if (symbol.scope == SymbolScope.GLOBAL || symbol.scope == SymbolScope.BUILTIN)
                    return (true, symbol);

                if(m_UpvalueCount == Utils.UPVALUE_COUNT)
                    Utils.Assert("Too many upvalue definitions, max is " + Utils.UPVALUE_COUNT.ToString());

                symbol.scope = SymbolScope.UPVALUE;
                symbol.upvalueIndex = m_UpvalueCount;
                m_UpvalueList[m_UpvalueCount++] = symbol;
                return (true, symbol);
            }

            return (false, null);
        }

        public void EnterScope()
        {
            m_ScopeDepth++;
        }

        public void ExitScope()
        {
            m_ScopeDepth--;
        }

        public int GetLocalVarCount()
        {

            return m_LocalVarCount;
        }

        public int GetUpvalueCount()
        {
            return m_UpvalueCount;
        }

        public Symbol[] GetUpvalueList()
        {
            return m_UpvalueList;
        }

        public SymbolTable GetUpper()
        {
            return m_Upper;
        }

        private Symbol FindSymbol(string name)
        {
            for (int i = 0; i <= m_VarCount; ++i)
            {
                if (m_VarList[i] !=null && m_VarList[i].name == name)
                    return m_VarList[i];
            }

            for (int i = 0; i <= m_UpvalueCount; ++i)
            {
                if (m_UpvalueList[i] != null && m_UpvalueList[i].name == name)
                    return m_UpvalueList[i];
            }

            return null;
        }

        private SymbolTable? m_Upper;
        private Symbol[] m_VarList;
        private int m_VarCount;
        private int m_LocalVarCount;
        private int m_GlobalVarCount;
        private Symbol[] m_UpvalueList;
        private int m_UpvalueCount;
        private int m_ScopeDepth;
    }

}