#include <stdio.h>
#include <stdarg.h>
#include <kiml/kiml.h>

#include "progpath.h"

enum RETURNVALUES
{
	RET_OK = 0,
	RET_RUNTIME_ERROR,
	RET_NO_EMBED,
	RET_FAIL_ENGINE,
	RET_FAIL_CODE,
	RET_FAIL_READ_SELF,
};

static int Run(KIMLBYTE *code, KIMLUINT codeSize);
static bool HasEmbeddedCode(KIMLBYTE *&code, KIMLUINT &codeSize);
static void showError(const char *s, ...);

int main(void)
{
	KIMLBYTE *code;
	KIMLUINT codeSize;

	if (!HasEmbeddedCode(code, codeSize))
	{
		printf("This executable does not contain embedded KimL bytecode.\n");
		return RET_NO_EMBED;
	}
	else
	{
		int result = Run(code, codeSize);
		
		delete []code;
		return result;
	}
}

int Run(KIMLBYTE *code, KIMLUINT codeSize)
{
	HKIMLENGINE engine;
	HKIMLSTATES states;

	engine = kimlCreateEngine();
	if (!engine)
	{
		showError("Cannot initialize virtual machine.\n");
		return RET_FAIL_ENGINE;
	}

	states = kimlCreateStates(engine);
	if (!states)
	{
		kimlFreeEngine(&engine);
		showError("Cannot initialize virtual machine.\n");
		return RET_FAIL_ENGINE;
	}

	if (kimlLoadByteCode(states, code, codeSize) == FALSE)
	{
		kimlFreeStates(&states);
		kimlFreeEngine(&engine);
		showError("Bytecode is invalid or corrupted.\n");
		return RET_FAIL_CODE;
	}

	kimlInitEngine(engine);
	kimlInitEngineWithStates(engine, states);

	if (kimlRun(engine) == FALSE)
	{
		KIMLUINT line = kimlGetLastLine(states);
		KIMLRUNTIMEERRORCODES err = kimlGetLastRuntimeError(states);
		KIMLCSTRING errmsg = kimlGetRuntimeErrorMessage(err);

		if (line)
			showError("Line %u:\n\tRuntime error %d: %s\n", line, err, errmsg);
		else
			showError("Runtime error %d: %s\n", err, errmsg);

		kimlFreeStates(&states);
		kimlFreeEngine(&engine);
		return RET_RUNTIME_ERROR;
	}
	
	kimlFreeStates(&states);
	kimlFreeEngine(&engine);

	return RET_OK;
}

bool HasEmbeddedCode(KIMLBYTE *&code, KIMLUINT &codeSize)
{
	char *progPath = GetProgramPath();

	FILE *fp = fopen(progPath, "rb");
	if (!fp)
	{
		showError("This program cannot read itself. Path: %s\n", progPath);

		delete [] progPath;
		return false;
	}

	delete [] progPath;

	fseek(fp, -4, SEEK_END);

	char embedMagic[4] = { 0 };
	fread(embedMagic, sizeof(char), sizeof(embedMagic), fp);

	if (embedMagic[0] != '\x0A'
		|| embedMagic[1] != '\xCE'
		|| embedMagic[2] != '\xCA'
		|| embedMagic[3] != '\xDE')
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);
	KIMLUINT fileSize = (KIMLUINT)ftell(fp);

	fseek(fp, -4 - (signed)sizeof(KIMLUINT), SEEK_END);

	KIMLUINT offset;
	fread(&offset, sizeof(KIMLUINT), 1, fp);
	
	KIMLUINT embedSize = fileSize - offset - sizeof(embedMagic) - sizeof(KIMLUINT);
	if (embedSize <= 0)
	{
		showError("Embedded bytecode corrupted.\n");
		fclose(fp);
		return false;
	}

	code = new KIMLBYTE[embedSize];
	codeSize = embedSize;
	
	fseek(fp, offset, SEEK_SET);
	fread(code, sizeof(KIMLBYTE), embedSize, fp);

	fclose(fp);

	return true;
}

void showError(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);

	vfprintf(stderr, s, ap);

	va_end(ap);
}
