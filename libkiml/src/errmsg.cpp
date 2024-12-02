#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "errmsg.h"

KIMLCSTRING compileErrorMessages[] =
{
	"No error.",
	"Duplicate variable.",
	"Undeclared variable.",
	"Duplicate label.",
	"Undeclared label.",
	"Duplicate function.",
	"Undeclared function.",
	"Duplicate constant.",
	"Expression is not constant.",
	"Expression is not int constant.",
	"Type mismatched.",
	"Inconsistent argument count.",
	"Cannot use initial value for variable in include file.",
	"Cannot use label for function in include file.",
	"Only const, var.decl and func.decl are allowed in include file."
};

KIMLCSTRING runtimeErrorMessages[] =
{
	"No error.",
	"Illegal opcode.",
	"Cannot convert object to int.",
	"Cannot convert object to real.",
	"Stack empty.",
	"Tape index out of range.",
	"Code offset out of range.",
	"Type mismatched.",
	"Stack pointer out of range.",
	"Division by zero.",
	"Unknown variable.",
	"Unknown function."
};
