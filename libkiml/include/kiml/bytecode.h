#ifndef __BYTECODE_H__
#define __BYTECODE_H__

#include "kiml.h"
#include "opcodes.h"

#include <vector>

struct KCHeader
{
	static char magic[4];
	static char ver[4];
	static char pad[4];
	static KIMLUINT size() { return sizeof(magic) + sizeof(ver) + sizeof(pad); }
};

class ByteCodeStream
{
private:
	std::vector<KIMLBYTE> buffer;
	static KIMLBYTE tmpBuf[16];

public:
	void Init(void);
	void CleanUp(void);

	KIMLBYTE * GetStream(void);
	KIMLUINT GetStreamSize(void) const;
	
	ByteCodeStream& operator <<(const OpCodes &op);
	ByteCodeStream& operator <<(const KIMLINT &iv);
	ByteCodeStream& operator <<(const KIMLUINT &iv);
	ByteCodeStream& operator <<(const KIMLFLOAT &rv);
};

#endif//__BYTECODE_H__
