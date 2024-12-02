#ifndef __SIMPL_SCOPE_H__
#define __SIMPL_SCOPE_H__

#include <kiml/kiml.h>
#include <map>
#include <string>

#include "simpl.h"

class Scope
{
private:
	std::map<std::string, KIMLTYPES> symbols;
	Scope *parent;
	int ordinal;
	int depth;

	int labelCounter;
	int childCounter;

public:
	Scope(Scope *parent);
	~Scope(void);

public:
	SIMPLCOMPILEERRORCODES AddSymbol(KIMLCSTRING name, KIMLTYPES dataType);
	SIMPLCOMPILEERRORCODES GetSymbol(KIMLCSTRING name, KIMLTYPES &dataType, int &depth) const;
	bool HasSymbol(KIMLCSTRING name) const;

	inline Scope * GetParent(void) const
	{
		return parent;
	}

	inline int GetDepth(void) const
	{
		return depth;
	}

	inline const std::map<std::string, KIMLTYPES> & GetAllSymbols(void) const
	{
		return symbols;
	}

	inline int GetOrdinal(void) const
	{
		return ordinal;
	}

	inline int IncrementLabelCounter(void)
	{
		return ++labelCounter;
	}

	inline int IncrementChildCounter(void)
	{
		return ++childCounter;
	}
};

#endif//__SIMPL_SCOPE_H__
