#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "functions.h"

KIMLCSTRING builtinFunctionNames[] =
{
	"abs", "sqrt",
	"sin", "cos", "tan", "asin", "acos", "atan",
	"chr", "asc", "len", "left", "mid", "right",
	"iif", "_tape", "_stack", "_pop", "_peek",
	"min", "max", "random",
	"print", "readln",

	"SENTINEL"
};
const KIMLUINT notBuiltinFunction = sizeof(builtinFunctionNames) / sizeof(KIMLCSTRING) - 1;

FunctionInfo builtinFunctionInfos[] =
{
	FunctionInfo("abs", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("sqrt", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("sin", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("cos", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("tan", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("asin", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("acos", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("atan", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("chr", KIML_STRING, KIML_INT, KIML_NOTYPE),
	FunctionInfo("asc", KIML_INT, KIML_STRING, KIML_NOTYPE),
	FunctionInfo("len", KIML_INT, KIML_STRING, KIML_NOTYPE),
	FunctionInfo("left", KIML_STRING, KIML_STRING, KIML_INT, KIML_NOTYPE),
	FunctionInfo("mid", KIML_STRING, KIML_STRING, KIML_INT, KIML_INT, KIML_NOTYPE),
	FunctionInfo("right", KIML_STRING, KIML_STRING, KIML_INT, KIML_NOTYPE),
	FunctionInfo("iif", KIML_ANYTYPE, KIML_BOOL, KIML_ANYTYPE, KIML_ANYTYPE, KIML_NOTYPE),
	FunctionInfo("_tape", KIML_ANYTYPE, KIML_INT, KIML_NOTYPE),
	FunctionInfo("_stack", KIML_ANYTYPE, KIML_INT, KIML_NOTYPE),
	FunctionInfo("_pop", KIML_ANYTYPE, KIML_NOTYPE),
	FunctionInfo("_peek", KIML_ANYTYPE, KIML_NOTYPE),
	FunctionInfo("min", KIML_REAL, KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("max", KIML_REAL, KIML_REAL, KIML_REAL, KIML_NOTYPE),
	FunctionInfo("random", KIML_REAL, KIML_NOTYPE),
	FunctionInfo("print", KIML_INT, KIML_ANYTYPE, KIML_NOTYPE),
	FunctionInfo("readln", KIML_STRING, KIML_NOTYPE)
};

KIMLCOMPILEERRORCODES FunctionTable::AddFunction(KIMLCSTRING name, const FunctionInfo &funcInfo)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);

	if (it != table.end())
		return KCE_DUPLICATE_FUNC;

	KIMLUINT fidx;
	if (GetBuiltinFunctionInfo(name, fidx))
		return KCE_DUPLICATE_FUNC;
	
	table[s] = funcInfo;
	return KCE_NONE;
}

KIMLCOMPILEERRORCODES FunctionTable::GetFunction(KIMLCSTRING name, FunctionInfo &funcInfo)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);

	if (it == table.end())
		return KCE_UNDECLARED_FUNC;

	funcInfo = it->second;
	return KCE_NONE;
}

void FunctionTable::CleanUp(void)
{
	table.clear();
}

const FunctionInfo * FunctionTable::GetBuiltinFunctionInfo(KIMLCSTRING name, KIMLUINT &index)
{
	builtinFunctionNames[notBuiltinFunction] = name;
	
	KIMLUINT i = 0;
	for (; ; ++i)
		if (!strcmp(builtinFunctionNames[i], name))
			break;

	if (i == notBuiltinFunction)
	{
		return nullptr;
	}
	else
	{
		index = i;
		return &builtinFunctionInfos[i];
	}
}
