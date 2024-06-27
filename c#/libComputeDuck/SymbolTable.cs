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
            this.isUpValue = 0;
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
            this.isUpValue = 0;
        }

        public string name;
        public bool isStructSymbol;
        public SymbolScope scope;
        public int index;
        public int scopeDepth;
        public int isUpValue;
    }

    public class SymbolTable
    {
        public SymbolTable()
        {
            this.enclosing = null;
            this.symbolMaps = new Dictionary<string, Symbol>();
            this.definitionCount = 0;
            this.scopeDepth = 0;
        }

        public SymbolTable(SymbolTable enclosing)
        {
            this.enclosing = enclosing;
            this.symbolMaps = new Dictionary<string, Symbol>();
            this.definitionCount = 0;
            this.scopeDepth = enclosing.scopeDepth + 1;
        }

        public Symbol Define(string name, bool isStructSymbol = false)
        {
            Symbol symbol = new Symbol(name, SymbolScope.GLOBAL, definitionCount, scopeDepth, isStructSymbol);
            if (enclosing == null)
                symbol.scope = SymbolScope.GLOBAL;
            else
                symbol.scope = SymbolScope.LOCAL;

            if (symbolMaps.ContainsKey(name))
                Utils.Assert("Redefined variable:(" + name + ") in current context");

            symbolMaps[name] = symbol;
            definitionCount++;
            return symbol;
        }

        public Symbol DefineBuiltin(string name)
        {
            var symbol = new Symbol(name, SymbolScope.BUILTIN, -1, scopeDepth);
            symbolMaps[name] = symbol;
            return symbol;
        }

        public (bool,Symbol?) Resolve(string name)
        {
            Symbol symbol;
            var isFound = symbolMaps.ContainsKey(name);
            if (isFound)
            {
                symbol = symbolMaps[name];
                return (true,symbol);
            }
            else if (enclosing != null)
            {
               (isFound,symbol) = enclosing.Resolve(name);
                if (!isFound)
                    return (false,null);
                if (symbol.scope == SymbolScope.GLOBAL || symbol.scope == SymbolScope.BUILTIN)
                    return (true,symbol);

                symbol.isUpValue = 1;

                symbolMaps[symbol.name] = symbol;
                return (true,symbol);
            }

            return (false,null);
        }

        public SymbolTable enclosing;
        public Dictionary<string, Symbol> symbolMaps;
        public int definitionCount;
        public int scopeDepth;
    }

}