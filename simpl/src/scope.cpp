#include "scope.h"

Scope::Scope(Scope *parent)
	: parent(parent), labelCounter(0), childCounter(0)
{
	int d = 1;
	for (auto s = parent; s; s = s->parent)
		++d;

	depth = d;
	if (parent)
		ordinal = parent->IncrementChildCounter();
	else
		ordinal = 0;
}

Scope::~Scope(void)
{
	symbols.clear();
}

SIMPLCOMPILEERRORCODES Scope::AddSymbol(KIMLCSTRING name, KIMLTYPES dataType)
{
	std::string key(name);
	auto it = symbols.find(key);
	if (it != symbols.end())
		return SCE_DUPLICATE_SYMBOL;

	symbols[key] = dataType;
	return SCE_NONE;
}

SIMPLCOMPILEERRORCODES Scope::GetSymbol(KIMLCSTRING name, KIMLTYPES &dataType, int &depth) const
{
	std::string key(name);
	auto it = symbols.find(key);
	if (it == symbols.end())
	{
		if (parent)
			return parent->GetSymbol(name, dataType, depth);
		else
			return SCE_UNDECLARED_SYMBOL;
	}

	dataType = it->second;
	depth = this->depth;
	return SCE_NONE;
}

bool Scope::HasSymbol(KIMLCSTRING name) const
{
	std::string key(name);
	auto it = symbols.find(key);
	if (it != symbols.end())
		return true;
	else if (parent)
		return parent->HasSymbol(name);
	else
		return false;
}
