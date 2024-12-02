#include "functions.h"

KIMLCSTRING simplBuiltinFunctionNames[] =
{
	"abs", "sqrt",
	"sin", "cos", "tan", "asin", "acos", "atan",
	"chr", "asc", "len", "left", "mid", "right",
	"iif", "_tape", "_stack", "_pop", "_peek",
	"min", "max", "random",
	"print", "readln",

	"SENTINEL"
};
const KIMLUINT notBuiltinFunction = sizeof(simplBuiltinFunctionNames) / sizeof(KIMLCSTRING) - 1;

SimpLFunctionInfo simplBuiltinFunctionInfos[] =
{
	SimpLFunctionInfo("abs", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("sqrt", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("sin", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("cos", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("tan", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("asin", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("acos", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("atan", KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("chr", KIML_STRING, KIML_INT, KIML_NOTYPE),
	SimpLFunctionInfo("asc", KIML_INT, KIML_STRING, KIML_NOTYPE),
	SimpLFunctionInfo("len", KIML_INT, KIML_STRING, KIML_NOTYPE),
	SimpLFunctionInfo("left", KIML_STRING, KIML_STRING, KIML_INT, KIML_NOTYPE),
	SimpLFunctionInfo("mid", KIML_STRING, KIML_STRING, KIML_INT, KIML_INT, KIML_NOTYPE),
	SimpLFunctionInfo("right", KIML_STRING, KIML_STRING, KIML_INT, KIML_NOTYPE),
	SimpLFunctionInfo("iif", KIML_ANYTYPE, KIML_BOOL, KIML_ANYTYPE, KIML_ANYTYPE, KIML_NOTYPE),
	SimpLFunctionInfo("_tape", KIML_ANYTYPE, KIML_INT, KIML_NOTYPE),
	SimpLFunctionInfo("_stack", KIML_ANYTYPE, KIML_INT, KIML_NOTYPE),
	SimpLFunctionInfo("_pop", KIML_ANYTYPE, KIML_NOTYPE),
	SimpLFunctionInfo("_peek", KIML_ANYTYPE, KIML_NOTYPE),
	SimpLFunctionInfo("min", KIML_REAL, KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("max", KIML_REAL, KIML_REAL, KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("random", KIML_REAL, KIML_NOTYPE),
	SimpLFunctionInfo("print", KIML_INT, KIML_ANYTYPE, KIML_NOTYPE),
	SimpLFunctionInfo("readln", KIML_STRING, KIML_NOTYPE)
};

SIMPLCOMPILEERRORCODES SimpLFunctionTable::AddFunction(KIMLCSTRING name, const SimpLFunctionInfo &funcInfo)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);

	if (it != table.end())
		return SCE_DUPLICATE_SYMBOL;

	KIMLUINT fidx;
	if (GetBuiltinFunctionInfo(name, fidx))
		return SCE_DUPLICATE_SYMBOL;
	
	table[s] = funcInfo;
	return SCE_NONE;
}

SIMPLCOMPILEERRORCODES SimpLFunctionTable::GetFunction(KIMLCSTRING name, const SimpLFunctionInfo *&funcInfo)
{
	std::string s(name);
	tabletype::iterator it = table.find(s);

	if (it == table.end())
		return SCE_UNDECLARED_SYMBOL;

	funcInfo = &it->second;
	return SCE_NONE;
}

void SimpLFunctionTable::CleanUp(void)
{
	table.clear();
}

const SimpLFunctionInfo * SimpLFunctionTable::GetBuiltinFunctionInfo(KIMLCSTRING name, KIMLUINT &index)
{
	simplBuiltinFunctionNames[notBuiltinFunction] = name;
	
	KIMLUINT i = 0;
	for (; ; ++i)
		if (!strcmp(simplBuiltinFunctionNames[i], name))
			break;

	if (i == notBuiltinFunction)
	{
		return nullptr;
	}
	else
	{
		index = i;
		return &simplBuiltinFunctionInfos[i];
	}
}
