#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "constants.h"

KIMLCOMPILEERRORCODES ConstTable::AddConst(KIMLCSTRING name, const Object &val)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);

	if (it != table.end())
		return KCE_DUPLICATE_VAR;

	table[s] = val;
	return KCE_NONE;
}

bool ConstTable::HasConst(KIMLCSTRING name)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);
	
	if (it == table.end())
		return false;

	return true;
}

bool ConstTable::GetConst(KIMLCSTRING name, Object &val)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);
	
	if (it == table.end())
		return false;

	val = it->second;
	return true;
}

void ConstTable::CleanUp(void)
{
	table.clear();
}
