#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "bytecode.h"
#include "utils.h"

#include <string.h>

KIMLBYTE ByteCodeStream::tmpBuf[16];

void ByteCodeStream::Init(void)
{
	this->buffer.clear();
	this->buffer.reserve(5192); //5KB
}

void ByteCodeStream::CleanUp(void)
{
	this->buffer.clear();
}

KIMLBYTE * ByteCodeStream::GetStream(void)
{
	return &this->buffer[0];
}

KIMLUINT ByteCodeStream::GetStreamSize(void) const
{
	return this->buffer.size();
}

ByteCodeStream& ByteCodeStream::operator <<(const OpCodes &op)
{
	this->buffer.push_back((KIMLBYTE)op);

	return *this;
}

ByteCodeStream& ByteCodeStream::operator <<(const KIMLINT &iv)
{
	kimlSerialize(this->tmpBuf, iv);
	this->buffer.insert(this->buffer.end(), this->tmpBuf, this->tmpBuf + sizeof(KIMLINT));

	return *this;
}

ByteCodeStream& ByteCodeStream::operator <<(const KIMLUINT &iv)
{
	kimlSerialize(this->tmpBuf, iv);
	this->buffer.insert(this->buffer.end(), this->tmpBuf, this->tmpBuf + sizeof(KIMLUINT));

	return *this;
}

ByteCodeStream& ByteCodeStream::operator <<(const KIMLFLOAT &rv)
{
	kimlSerialize(this->tmpBuf, rv);
	this->buffer.insert(this->buffer.end(), this->tmpBuf, this->tmpBuf + sizeof(KIMLFLOAT));

	return *this;
}
