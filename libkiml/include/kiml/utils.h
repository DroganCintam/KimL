#ifndef __UTILS_H__
#define __UTILS_H__

#include "kiml.h"

KIMLSTRING kimlstrdup(KIMLCSTRING source);

KIMLBOOL kimlstrequ(KIMLCSTRING s1, KIMLCSTRING s2);

KIMLUINT kimlstrhash(KIMLCSTRING s);

KIMLVOID kimlSerialize(KIMLBYTE *buf, KIMLINT value);
KIMLINT kimlDeserializeInt(const KIMLBYTE *buf);

KIMLVOID kimlSerialize(KIMLBYTE *buf, KIMLUINT value);
KIMLUINT kimlDeserializeUInt(const KIMLBYTE *buf);

KIMLVOID kimlSerialize(KIMLBYTE *buf, KIMLFLOAT value);
KIMLFLOAT kimlDeserializeFloat(const KIMLBYTE *buf);

#endif//__UTILS_H__
