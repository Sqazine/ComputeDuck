using System.Collections.Generic;

namespace ComputeDuck
{
    public enum SymbolScope
    {
        GLOBAL,
        LOCAL,
        BUILTIN
    }

    public class Symbol
    {
        public Symbol()
        {
            this.scope = SymbolScope.GLOBAL;
            this.index = 0;
            this.scopeDepth = 0;
            this.name = "";
            this.isUpValue = false;
        }

        public Symbol(string name,
                      SymbolScope scope,
                      int index,
                      int scopeDepth,
                      bool isStructSymbol = false)
        {
            this.name = name;
            this.isStructSymbol = isStructSymbol;
            this.scope = scope;
            this.index = index;
            this.scopeDepth = scopeDepth;
            this.isUpValue = false;
        }

        public string name;
        public bool isStructSymbol;
        public SymbolScope scope;
        public int index;
        public int scopeDepth;
        public bool isUpValue;
    }

    public class SymbolTable
    {
        public SymbolTable()
        {
            this.upper = null;
            this.symbolList = new Symbol[Utils.UINT8_COUNT];
            this.varCount = 0;
            this.scopeDepth = 0;
        }

        public SymbolTable(SymbolTable upper)
        {
            this.upper = upper;
            this.symbolList = new Symbol[Utils.UINT8_COUNT];
            this.varCount = 0;
            this.scopeDepth = upper.scopeDepth + 1;
        }

        public Symbol Define(string name, bool isStructSymbol = false)
        {
            if (varCount >= Utils.UINT8_COUNT)
                Utils.Assert("Too many variable definitions, max is " + Utils.UINT8_COUNT.ToString());

            if (FindSymbol(name) != null)
                Utils.Assert("Variable already defined in this scope:" + name);

            Symbol symbol = new Symbol(name, SymbolScope.GLOBAL, varCount, scopeDepth, isStructSymbol);
            if (upper == null)
                symbol.scope = SymbolScope.GLOBAL;
            else
                symbol.scope = SymbolScope.LOCAL;

            symbolList[varCount++] = symbol;
            return symbol;
        }

        public Symbol DefineBuiltin(string name)
        {
            Symbol result = FindSymbol(name);
            if (result != null)
                return result;

            if (varCount == Utils.UINT8_COUNT)
                Utils.Assert("Too many variable definitions, max is " + Utils.UINT8_COUNT.ToString());

            var symbol = new Symbol(name, SymbolScope.BUILTIN, -1, scopeDepth);
            symbolList[varCount++] = symbol;
            return symbol;
        }

        public (bool, Symbol?) Resolve(string name)
        {
            Symbol result = FindSymbol(name);
            if (result != null)
            {
                if (scopeDepth < result.scopeDepth)
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

                symbol.isUpValue = true;
                symbolList[varCount++] = symbol;
                return (true, symbol);
            }

            return (false, null);
        }

        public void EnterScope()
        {

            scopeDepth++;
        }

        public void ExitScope()
        {
            scopeDepth--;
        }

        public int GetVarCount()
        {
            return varCount;
        }

        public SymbolTable GetUpper()
        {
            return upper;
        }

        private Symbol FindSymbol(string name)
        {
            for (int i = 0; i <= varCount; ++i)
            {
                if (symbolList[i] !=null && symbolList[i].name == name)
                    return symbolList[i];
            }

            return null;
        }

        private SymbolTable? upper;
        private Symbol[] symbolList;
        private int varCount;
        private int scopeDepth;
    }

}