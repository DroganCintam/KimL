#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "utils.h"
#include "endian.hpp"

#include <string.h>

KIMLSTRING kimlstrdup(KIMLCSTRING source)
{
	KIMLUINT len = strlen(source);
	KIMLCHAR *s = new KIMLCHAR[len + 1];
	if (s)
	{
#ifdef _MSC_VER
		strncpy_s(s, len+1, source, len);
#else
		strncpy(s, source, len);
#endif
		s[len] = (KIMLCHAR)0;
	}

	return s;
}

KIMLBOOL kimlstrequ(KIMLCSTRING s1, KIMLCSTRING s2)
{
	return *s1 != *s2 ? FALSE : !strcmp(s1, s2);
}

KIMLUINT kimlstrhash(KIMLCSTRING s)
{
	KIMLUINT h = 0;
	KIMLCSTRING p;

	for (p = s; *p; ++p)
		h = ((h << 4) | (h >> 28)) ^ *p; // _rotl(h, 4) ^ *s

	return h;
}

#ifndef KIML_LITTLE_ENDIAN
#define KIML_LITTLE_ENDIAN
#endif

KIMLVOID kimlSerialize(KIMLBYTE *buf, KIMLINT value)
{
#if defined(KIML_LITTLE_ENDIAN) && defined(BOOST_LITTLE_ENDIAN)
	*reinterpret_cast<KIMLINT *>(buf) = value;
#else
	KIMLBYTE bytes[sizeof(KIMLINT)]; reinterpret_cast<KIMLINT *>(bytes) = &value;
	for (register int i = sizeof(KIMLINT) - 1; i >= 0; --i)
		buf[sizeof(KIMLINT) - 1 - i] = bytes[i];
#endif
}

KIMLINT kimlDeserializeInt(const KIMLBYTE *buf)
{
	KIMLINT value;
#if defined(KIML_LITTLE_ENDIAN) && defined(BOOST_LITTLE_ENDIAN)
	value = *reinterpret_cast<const KIMLINT *>(buf);
#else
	KIMLBYTE bytes[sizeof(KIMLINT)];
	for (register int i = sizeof(KIMLINT) - 1; i >= 0; --i)
		bytes[sizeof(KIMLINT) - 1 - i] = buf[i];
	value = *reinterpret_cast<KIMLINT *>(bytes);
#endif
	return value;
}


KIMLVOID kimlSerialize(KIMLBYTE *buf, KIMLUINT value)
{
#if defined(KIML_LITTLE_ENDIAN) && defined(BOOST_LITTLE_ENDIAN)
	*reinterpret_cast<KIMLUINT *>(buf) = value;
#else
	KIMLBYTE bytes[sizeof(KIMLUINT)]; reinterpret_cast<KIMLUINT *>(bytes) = &value;
	for (register int i = sizeof(KIMLUINT) - 1; i >= 0; --i)
		buf[sizeof(KIMLUINT) - 1 - i] = bytes[i];
#endif
}

KIMLUINT kimlDeserializeUInt(const KIMLBYTE *buf)
{
	KIMLUINT value;
#if defined(KIML_LITTLE_ENDIAN) && defined(BOOST_LITTLE_ENDIAN)
	value = *reinterpret_cast<const KIMLUINT *>(buf);
#else
	KIMLBYTE bytes[sizeof(KIMLUINT)];
	for (register int i = sizeof(KIMLUINT) - 1; i >= 0; --i)
		bytes[sizeof(KIMLUINT) - 1 - i] = buf[i];
	value = *reinterpret_cast<KIMLUINT *>(bytes);
#endif
	return value;
}


KIMLVOID kimlSerialize(KIMLBYTE *buf, KIMLFLOAT value)
{
#if defined(KIML_LITTLE_ENDIAN) && defined(BOOST_LITTLE_ENDIAN)
	*reinterpret_cast<KIMLFLOAT *>(buf) = value;
#else
	KIMLBYTE bytes[sizeof(KIMLFLOAT)]; reinterpret_cast<KIMLFLOAT *>(bytes) = &value;
	for (register int i = sizeof(KIMLFLOAT) - 1; i >= 0; --i)
		buf[sizeof(KIMLFLOAT) - 1 - i] = bytes[i];
#endif
}

KIMLFLOAT kimlDeserializeFloat(const KIMLBYTE *buf)
{
	KIMLFLOAT value;
#if defined(KIML_LITTLE_ENDIAN) && defined(BOOST_LITTLE_ENDIAN)
	value = *reinterpret_cast<const KIMLFLOAT *>(buf);
#else
	KIMLBYTE bytes[sizeof(KIMLFLOAT)];
	for (register int i = sizeof(KIMLFLOAT) - 1; i >= 0; --i)
		bytes[sizeof(KIMLFLOAT) - 1 - i] = buf[i];
	value = *reinterpret_cast<KIMLFLOAT *>(bytes);
#endif
	return value;
}

