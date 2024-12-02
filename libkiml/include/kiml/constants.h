#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include "kiml.h"
#include "object.h"

#include <map>
#include <string>

class ConstTable
{
private:
	typedef std::map<std::string, Object> tabletype;

private:
	tabletype table;

public:
	KIMLCOMPILEERRORCODES AddConst(KIMLCSTRING name, const Object &val);
	bool HasConst(KIMLCSTRING name);
	bool GetConst(KIMLCSTRING name, Object &val);
	void CleanUp(void);
};

#endif//__CONSTANTS_H__
