#ifndef ___SIMPL_CONSTANTS_H__
#define ___SIMPL_CONSTANTS_H__

#include <kiml/kiml.h>
#include <kiml/object.h>

#include <map>
#include <string>

#include "simpl.h"

class ConstTable
{
private:
	typedef std::map<std::string, Object> tabletype;

private:
	tabletype table;

public:
	SIMPLCOMPILEERRORCODES AddConst(KIMLCSTRING name, const Object &val);
	bool HasConst(KIMLCSTRING name);
	bool GetConst(KIMLCSTRING name, Object &val);
	void CleanUp(void);
};

#endif//___SIMPL_CONSTANTS_H__
