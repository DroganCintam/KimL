#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "states.h"

#include "utils.h"

#include <string.h>

bool States::loadByteCode(const KIMLBYTE *byteCodeContent, KIMLUINT byteCodeSize)
{
	if (byteCodeSize < 24)
		return false;

	if (this->code)
	{
		delete []this->code;
		this->code = nullptr;
		this->codesize = 0;
	}

	KIMLCHAR magic[4];
	KIMLCHAR ver[4];
	KIMLBYTE pad[8];

	memcpy(magic, byteCodeContent, 4);
	memcpy(ver, byteCodeContent + 4, 4);
	memcpy(pad, byteCodeContent + 8, 8);

	if (strncmp(magic, "kiml", 4))
		return false;

	// has to be version 13.08, 14.11 or 17.11
	if (strncmp(ver, "1308", 4)
		&& strncmp(ver, "1411", 4)
		&& strncmp(ver, "1711", 4))
		return false;

	if (*(uint64_t *)pad)
		return false;

	KIMLUINT codeSize = kimlDeserializeUInt(byteCodeContent + 16);
	if (codeSize + 16 + sizeof(KIMLUINT) * 2 > byteCodeSize)
		return false;

	KIMLBYTE *code = new KIMLBYTE[codeSize];
	memcpy(code, byteCodeContent + 16 + sizeof(KIMLUINT), codeSize);

	KIMLUINT poolSize = kimlDeserializeUInt(byteCodeContent + 16 + sizeof(KIMLUINT) + codeSize);
	if (poolSize + codeSize + 16 + sizeof(KIMLUINT) * 2 > byteCodeSize)
	{
		delete []code;
		return false;
	}

	KIMLCHAR *pool = new KIMLCHAR[poolSize];
	memcpy(pool, byteCodeContent + 16 + sizeof(KIMLUINT) * 2 + codeSize, poolSize);

	this->code = code;
	this->codesize = codeSize;
	this->stringpool = pool;
	this->stringpoolsize = poolSize;

	return true;
}
