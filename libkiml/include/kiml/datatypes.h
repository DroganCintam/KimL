#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include "kiml.h"

enum KIMLTYPEMATCH
{
	K_TYPE_MATCHED,
	K_TYPE_MISMATCHED,
	K_TYPE_CAST
};

static KIMLTYPEMATCH TypeMatchTable[][7] =
{	//           none               bool               int             real               string             any             object
	/*none*/   { K_TYPE_MATCHED,    K_TYPE_MATCHED,    K_TYPE_MATCHED, K_TYPE_MATCHED,    K_TYPE_MATCHED,    K_TYPE_MATCHED, K_TYPE_MATCHED },
	/*bool*/   { K_TYPE_MISMATCHED, K_TYPE_MATCHED,    K_TYPE_CAST,    K_TYPE_CAST,       K_TYPE_CAST,       K_TYPE_CAST,    K_TYPE_CAST },
	/*int*/    { K_TYPE_MISMATCHED, K_TYPE_CAST,       K_TYPE_MATCHED, K_TYPE_CAST,       K_TYPE_MISMATCHED, K_TYPE_CAST,    K_TYPE_CAST },
	/*real*/   { K_TYPE_MISMATCHED, K_TYPE_CAST,       K_TYPE_CAST,    K_TYPE_MATCHED,    K_TYPE_MISMATCHED, K_TYPE_CAST,    K_TYPE_MISMATCHED },
	/*string*/ { K_TYPE_MISMATCHED, K_TYPE_CAST,       K_TYPE_CAST,    K_TYPE_CAST,       K_TYPE_MATCHED,    K_TYPE_CAST,    K_TYPE_CAST },
	/*any*/    { K_TYPE_MATCHED,    K_TYPE_MATCHED,    K_TYPE_MATCHED, K_TYPE_MATCHED,    K_TYPE_MATCHED,    K_TYPE_MATCHED, K_TYPE_MATCHED },
	/*object*/ { K_TYPE_MATCHED,    K_TYPE_MISMATCHED, K_TYPE_CAST,    K_TYPE_MISMATCHED, K_TYPE_CAST,       K_TYPE_CAST,    K_TYPE_MATCHED }
};

inline static KIMLTYPES HigherType(KIMLTYPES t1, KIMLTYPES t2)
{
	return t1 > t2 ? t1 : t2;
}

// Returns true if the given type is valid for arithmetic operations.
inline static bool IsValidArithmetic(KIMLTYPES t)
{
	switch (t)
	{
	case KIML_INT:
	case KIML_REAL:
	case KIML_ANYTYPE: //yes, because it can be int or real at runtime
		return true;
	default:
		return false;
	}
}

#endif//__DATATYPES_H__
