#include <stdio.h>
#include <kiml/kiml.h>

#include "config.h"
#include "compiler.h"

enum RETURNVALUES
{
	RET_OK,
	RET_FAIL_ARGS,
	RET_RUNTIME_ERROR,
	RET_FAIL_ENGINE,
	RET_FAIL_COMPILER,
	RET_FAIL_CODE,
	RET_FAIL_IO,
	RET_FAIL_MEMORY,
};

enum class CommandTypes
{
	None,
	Invalid,
	Run,
	CompileAndRun,
	Compile,
	CompileNative,
};

static std::string inputfile;
static std::string inputfileNoExt;
static std::string outputfile;
static std::vector<std::string> includeFiles;
static bool hasDebugInfo;
static bool toNative;

static int Run(KIMLBYTE *code, KIMLUINT codeSize);
static int Compile(KIMLBYTE *&code, KIMLUINT &codeSize);

static int Run(void);
static int Compile(void);
static int CompileAndRun(void);

static CommandTypes parseArguments(int argc, char **argv);
static void showHelp(bool withTitle = false);
static void showError(const char *err, ...);

int main(int argc, char **argv)
{
	switch (parseArguments(argc, argv))
	{
		case CommandTypes::None:
			return RET_OK;

		case CommandTypes::Invalid:
			return RET_FAIL_ARGS;

		case CommandTypes::Run:
			return Run();

		case CommandTypes::Compile:
		case CommandTypes::CompileNative:
			return Compile();

		case CommandTypes::CompileAndRun:
			return CompileAndRun();
	}

	return RET_OK;
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

int Compile(KIMLBYTE *&code, KIMLUINT &codeSize)
{
	auto compiler = new SimpLCompiler();
	if (!compiler)
	{
		showError("Cannot initialize compiler service.\n");
		return RET_FAIL_COMPILER;
	}

	std::vector<std::string> files;
	for (auto it = includeFiles.begin(); it != includeFiles.end(); ++it)
	{
		files.push_back(*it);
	}
	files.push_back(inputfile);

	bool failed = false;

	FILE *fp;
	for (auto it = files.begin(); it != files.end(); ++it)
	{
		fp = fopen(it->c_str(), "r");
		if (!fp)
		{
			showError("Cannot open: %s.\n", it->c_str());
			delete compiler;
			return RET_FAIL_IO;
		}

		if (!compiler->Parse(it->c_str(), fp))
		{
			failed = true;
		}

		fclose(fp);
	}

	if (failed)
	{
		showError("*** Parsing failed.\n");
		delete compiler;
		return RET_FAIL_COMPILER;
	}

	if (!compiler->Compile(hasDebugInfo))
	{
		auto &errors = compiler->GetErrors();
		for (unsigned int i = 0, c = errors.size(); i < c; ++i)
		{
			auto &error = errors[i];
			KIMLCSTRING msg = simplCompileErrorMessages[error.code];
			showError("%s[%u]: %s\n", files[error.compileUnitIndex].c_str(), error.line, msg);
		}
		delete compiler;
		return RET_FAIL_COMPILER;
	}

	if (!compiler->Finish())
	{
		showError("*** Failed to finish compilation.\n");
		delete compiler;
		return RET_FAIL_COMPILER;
	}

	if (!compiler->GetByteCode(&code, &codeSize))
	{
		showError("*** Failed to export bytecode.\n");
		delete compiler;
		return RET_FAIL_COMPILER;
	}

	delete compiler;
	return RET_OK;
}

int Run(void)
{
	FILE *fp = fopen(inputfile.c_str(), "rb");
	if (!fp)
	{
		showError("Cannot open: %s.\n", inputfile.c_str());
		return RET_FAIL_IO;
	}

	KIMLBYTE *code;
	KIMLUINT codeSize;

	fseek(fp, 0, SEEK_END);
	codeSize = (KIMLUINT)ftell(fp);

	fseek(fp, 0, SEEK_SET);

	code = new KIMLBYTE[codeSize];
	if (!code)
	{
		showError("Cannot allocate memory for bytecode.\n");
		return RET_FAIL_MEMORY;
	}

	fread(code, sizeof(KIMLBYTE), codeSize, fp);
	fclose(fp);

	int result = Run(code, codeSize);

	delete[]code;
	return result;
}

int Compile(void)
{
	KIMLBYTE *code;
	KIMLUINT codeSize;

	int result = Compile(code, codeSize);
	if (result)
		return result;

	FILE *fp = fopen(outputfile.c_str(), "wb");
	if (!fp)
	{
		showError("Cannot open or create: %s.\n", outputfile.c_str());
		return RET_FAIL_IO;
	}

	if (toNative)
	{
		static KIMLBYTE embedMagic[4] = { 0x0A, 0xCE, 0xCA, 0xDE };

		FILE *fpCore = fopen(EXENATIVE, "rb");
		if (!fpCore)
		{
			showError("Cannot open %s.\n", EXENATIVE);
			delete[]code;
			fclose(fp);
			return RET_FAIL_IO;
		}

		fseek(fpCore, 0, SEEK_END);
		KIMLUINT coreSize = (KIMLUINT)ftell(fpCore);
		fseek(fpCore, 0, SEEK_SET);

		KIMLBYTE *coreData = new KIMLBYTE[coreSize];
		if (!coreData)
		{
			showError("Cannot allocate memory.\n");
			delete[]code;
			fclose(fp);
			fclose(fpCore);
			return RET_FAIL_MEMORY;
		}

		fread(coreData, sizeof(KIMLBYTE), coreSize, fpCore);
		fclose(fpCore);

		fwrite(coreData, sizeof(KIMLBYTE), coreSize, fp);
		fwrite(code, sizeof(KIMLBYTE), codeSize, fp);
		fwrite((void *)&coreSize, sizeof(KIMLUINT), 1, fp);
		fwrite((void *)embedMagic, sizeof(KIMLBYTE), sizeof(embedMagic), fp);
		fclose(fp);

		printf("*** Successfully compiled '%s' to '%s'.\n", inputfile.c_str(), outputfile.c_str());

		delete[]code;
		delete[]coreData;
		return RET_OK;
	}
	else
	{
		fwrite(code, sizeof(KIMLBYTE), codeSize, fp);
		fclose(fp);

		printf("*** Successfully compiled '%s' to '%s'.\n", inputfile.c_str(), outputfile.c_str());

		delete[]code;
		return RET_OK;
	}
}

int CompileAndRun(void)
{
	KIMLBYTE *code;
	KIMLUINT codeSize;

	int result = Compile(code, codeSize);
	if (result)
		return result;

	result = Run(code, codeSize);

	delete[]code;
	return result;
}

CommandTypes parseArguments(int argc, char **argv)
{
	bool has_file = false;
	bool has_c = false;
	bool has_x = false;
	bool has_o = false;
	bool has_d = false;
	bool has_h = false;
	bool inputKC = false;

	for (int i = 1; i < argc; ++i)
	{
		char *arg = argv[i];
		if (arg[0] == '-')
		{
			switch (arg[1])
			{
				case 'c':
					if (!has_c)
					{
						if (!has_x)
						{
							has_c = true;
							toNative = false;
						}
						else
						{
							showError("Cannot use both -c and -x.\n");
							showHelp();
							return CommandTypes::Invalid;
						}
					}
					else
					{
						showHelp();
						return CommandTypes::Invalid;
					}
					break;

				case 'x':
					if (!has_x)
					{
						if (!has_c)
						{
							has_x = true;
							toNative = true;
						}
						else
						{
							showError("Cannot use both -c and -x.\n");
							showHelp();
							return CommandTypes::Invalid;
						}
					}
					else
					{
						showHelp();
						return CommandTypes::Invalid;
					}
					break;

				case 'i':
				{
					if (arg[2])
					{
						includeFiles.push_back(arg + 2);
					}
					else
					{
						showError("Include file option given but no file specified.\n");
						showHelp();
						return CommandTypes::Invalid;
					}
				}
				break;

				case 'o':
					if (!has_o)
					{
						if (arg[2])
						{
							has_o = true;
							outputfile = arg + 2;
						}
						else
						{
							showError("Output file option given but no file specified.\n");
							showHelp();
							return CommandTypes::Invalid;
						}
					}
					else
					{
						showHelp();
						return CommandTypes::Invalid;
					}
					break;

				case 'd':
					if (!has_d)
					{
						has_d = true;
						hasDebugInfo = true;
					}
					else
					{
						showHelp();
						return CommandTypes::Invalid;
					}
					break;

				case 'h':
					if (!has_h)
					{
						has_h = true;
					}
					else
					{
						showHelp();
						return CommandTypes::Invalid;
					}
					break;

				default:
					showHelp();
					return CommandTypes::Invalid;
			}
		}
		else
		{
			if (!has_file)
			{
				has_file = true;
				inputfile = arg;

				auto dotpos = inputfile.rfind('.');
				if (dotpos == std::string::npos)
				{
					showError("Unknown input file type.\n");
					showHelp();
					return CommandTypes::Invalid;
				}

				std::string ext = inputfile.substr(dotpos + 1);
				if (!strcmp(ext.c_str(), "kc"))
				{
					inputKC = true;
				}
				else
				{
					inputfileNoExt = inputfile.substr(0, dotpos);
				}
			}
			else
			{
				showError("Possibly duplicate input file.\n");
				return CommandTypes::Invalid;
			}
		}
	}

	if (has_h)
		showHelp(true);

	if (!has_file)
	{
		if (has_c || has_x || has_d || has_o || includeFiles.size())
		{
			return CommandTypes::Invalid;
		}
		else
		{
			showHelp(true);
			return CommandTypes::None;
		}
	}
	else
	{
		if (has_c || has_x || !inputKC)
		{
			if (!inputKC)
			{
				if (!has_o)
				{
					outputfile = inputfileNoExt;
					outputfile.append(toNative ? EXESUFFIX : ".kc");
				}

				if (has_c)
					return CommandTypes::Compile;
				else if (has_x)
					return CommandTypes::CompileNative;
				else
					return CommandTypes::CompileAndRun;
			}
			else
			{
				showError("Input file appears to be in compiled format.\n");
				return CommandTypes::Invalid;
			}
		}
		else
		{
			if (has_d || has_o || includeFiles.size())
			{
				showHelp();
				return CommandTypes::Invalid;
			}
			else
			{
				return CommandTypes::Run;
			}
		}

		return CommandTypes::None;
	}
}

void showHelp(bool withTitle)
{
	if (withTitle)
	{
		printf("SimpL Compiler and Virtual Machine - v%s\n", APPVER);
		printf("Language specification %s\n", LANGVER);
		printf("\n");
	}

	printf("Usage: simpl [options] file\n\n");

	printf("Options:\n");
	printf("\t-c      Compile to KimL binary.\n");
	printf("\t-x      Compile to native executable.\n");
	printf("\t-iFILE  Add include file.\n");
	printf("\t-oFILE  Set output file.\n");
	printf("\t-d      Embed debug info.\n");
	printf("\t-h      Show this help.\n\n");
}

void showError(const char *s, ...)
{
	va_list ap;
	va_start(ap, s);

	vfprintf(stderr, s, ap);

	va_end(ap);
}
