#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "symbols.h"

KIMLCOMPILEERRORCODES SymbolTable::AddSymbol(KIMLCSTRING ident, KIMLTYPES type)
{
	std::string s(ident);
	tabletype::iterator it = this->table.find(s);

	if (it != this->table.end())
		return KCE_DUPLICATE_VAR;

	this->table[s] = type;
	return KCE_NONE;
}

KIMLCOMPILEERRORCODES SymbolTable::RemoveSymbol(KIMLCSTRING ident)
{
	std::string s(ident);
	tabletype::iterator it = this->table.find(s);

	if (it == this->table.end())
		return KCE_UNDECLARED_VAR;

	this->table.erase(it);
	return KCE_NONE;
}

KIMLCOMPILEERRORCODES SymbolTable::HasSymbol(KIMLCSTRING ident)
{
	std::string s(ident);
	tabletype::iterator it = this->table.find(s);

	if (it == this->table.end())
		return KCE_UNDECLARED_VAR;

	return KCE_NONE;
}

KIMLCOMPILEERRORCODES SymbolTable::GetSymbolType(KIMLCSTRING ident, KIMLTYPES &type) const
{
	std::string s(ident);
	tabletype::const_iterator it = this->table.find(s);

	if (it == this->table.end())
		return KCE_UNDECLARED_VAR;

	type = it->second;
	return KCE_NONE;
}

void SymbolTable::CleanUp()
{
	this->table.clear();
}
