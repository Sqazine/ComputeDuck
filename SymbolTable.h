#pragma once
#include <string>
#include <unordered_map>

#include "Utils.h"
#ifdef BUILD_WITH_LLVM
#include "llvm/IR/Value.h"
#endif
enum class SymbolScope
{
	GLOBAL,
	LOCAL,
	BUILTIN,
};

struct Symbol
{
	Symbol()
		: scope(SymbolScope::GLOBAL), index(0), scopeDepth(0), isInUpScope(0), isStructSymbol(false)
	{
	}

	Symbol(const std::string &name, const SymbolScope &scope, int32_t index, int32_t scopeDepth = 0, bool isStructSymbol = false)
		: name(name), scope(scope), index(index), isStructSymbol(isStructSymbol), scopeDepth(scopeDepth), isInUpScope(0)
	{
	}

#ifdef BUILD_WITH_LLVM
	Symbol(const std::string &name, const SymbolScope &scope, llvm::Value *allocationGEP, int32_t scopeDepth = 0)
		: name(name), scope(scope), allocationGEP(allocationGEP), scopeDepth(scopeDepth), isInUpScope(0)
	{
	}

	llvm::Value *allocationGEP;
#endif

	std::string name;
	bool isStructSymbol;
	SymbolScope scope;
	int32_t index;
	int32_t scopeDepth;
	int32_t isInUpScope;
};

struct SymbolTable
{
	SymbolTable()
		: enclosing(nullptr), definitionCount(0), scopeDepth(0)
	{
	}

	SymbolTable(SymbolTable *enclosing)
		: enclosing(enclosing), definitionCount(0)
	{
		scopeDepth = enclosing->scopeDepth + 1;
	}

	~SymbolTable()
	{
		auto p = enclosing;
		while (p)
		{
			auto q = p->enclosing;
			SAFE_DELETE(p);
			p = q;
		}
	}

#ifdef BUILD_WITH_LLVM
	void Set(const std::string &name, llvm::Value *a)
	{
		symbolMaps[name].allocationGEP = a;
	}
#endif

	Symbol Define(const std::string &name, bool isStructSymbol = false)
	{
		auto symbol = Symbol(name, SymbolScope::GLOBAL, definitionCount, scopeDepth, isStructSymbol);

		if (!enclosing)
			symbol.scope = SymbolScope::GLOBAL;
		else
			symbol.scope = SymbolScope::LOCAL;

		if (symbolMaps.find(name) != symbolMaps.end())
			ASSERT("Redefined variable:(%s) in current context.", name.data());

		symbolMaps[name] = symbol;
		definitionCount++;
		return symbol;
	}

	Symbol DefineBuiltin(const std::string &name, int32_t index = -1)
	{
		auto symbol = Symbol(name, SymbolScope::BUILTIN, index, scopeDepth);
		symbolMaps[name] = symbol;
		return symbol;
	}

	bool Resolve(const std::string &name, Symbol &symbol)
	{
		auto iter = symbolMaps.find(name);
		if (iter != symbolMaps.end())
		{
			symbol = iter->second;
			return true;
		}
		else if (enclosing)
		{
			bool isFound = enclosing->Resolve(name, symbol);
			if (!isFound)
				return false;
			if (symbol.scope == SymbolScope::GLOBAL || symbol.scope == SymbolScope::BUILTIN)
				return true;

			symbol.isInUpScope = 1;

			symbolMaps[symbol.name] = symbol;
			return true;
		}

		return false;
	}

	SymbolTable *enclosing;
	std::unordered_map<std::string, Symbol> symbolMaps;
	uint8_t definitionCount;
	uint8_t scopeDepth;
};
