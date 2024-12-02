#include "constants.h"

SIMPLCOMPILEERRORCODES ConstTable::AddConst(KIMLCSTRING name, const Object &val)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);

	if (it != table.end())
		return SCE_DUPLICATE_SYMBOL;

	table[s] = val;
	return SCE_NONE;
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
