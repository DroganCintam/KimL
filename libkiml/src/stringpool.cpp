#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "stringpool.h"

void StringPool::Init(void)
{
	this->buffer.reserve(5120); //5KB
}

void StringPool::AddString(KIMLCSTRING str)
{
	std::string s(str);
	std::map<std::string, KIMLUINT>::iterator it = this->table.find(s);

	if (it == this->table.end())
	{
		KIMLUINT ptr = this->buffer.size();
		this->buffer.insert(this->buffer.end(), str, str + s.length() + 1);
		this->table[s] = ptr;
	}
}

KIMLUINT StringPool::GetStringAddress(KIMLCSTRING str)
{
	return this->table[str];
}

void StringPool::CleanUp()
{
	this->buffer.clear();
	this->table.clear();
}
