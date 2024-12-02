#ifndef __STRINGPOOL_H__
#define __STRINGPOOL_H__

#include "kiml.h"

#include <map>
#include <vector>
#include <string>

class StringPool
{
private:
	std::vector<KIMLCHAR> buffer;
	std::map<std::string, KIMLUINT> table;

public:
	void Init(void);
	void AddString(KIMLCSTRING str);
	KIMLUINT GetStringAddress(KIMLCSTRING str);

	const KIMLCHAR * GetPool(void) const { return &buffer[0]; }
	KIMLUINT GetPoolSize(void) const { return buffer.size(); }

	void CleanUp();
};

#endif//__STRINGPOOL_H__
