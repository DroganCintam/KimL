#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include "kiml.h"

#include <map>
#include <string>

class SymbolTable
{
private:
	typedef std::map<std::string, KIMLTYPES> tabletype;

private:
	tabletype table;

public:
	KIMLCOMPILEERRORCODES AddSymbol(KIMLCSTRING ident, KIMLTYPES type);
	KIMLCOMPILEERRORCODES RemoveSymbol(KIMLCSTRING ident);
	KIMLCOMPILEERRORCODES HasSymbol(KIMLCSTRING ident);
	KIMLCOMPILEERRORCODES GetSymbolType(KIMLCSTRING ident, KIMLTYPES &type) const;
	void CleanUp();
};

#endif//__SYMBOLS_H__
