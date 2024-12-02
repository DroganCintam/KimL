#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "kiml.h"

#include "ast.h"
#include "labels.h"
#include "symbols.h"
#include "constants.h"
#include "functions.h"
#include "stringpool.h"
#include "opcodes.h"
#include "bytecode.h"

#include <stdio.h>
#include <stack>
#include <map>
#include <string>

class Compiler
{
private:
	LabelTable *labelTable;
	SymbolTable *symbolTable;
	ConstTable *constTable;
	FunctionTable *funcTable;
	std::map<std::string, KIMLUINT> vars;
	
	ByteCodeStream *byteCodeStream;
	KIMLUINT programSize;
	KIMLUINT line;

	StringPool *stringPool;
	KIMLUINT stringPoolSize;

	bool hasDebugInfo;

	std::vector<KIMLCOMPILEERRORINFO> errors;

	FILE *fp;

	bool inIncludeMode;

public:
	Compiler(void);
	~Compiler(void);

public:
	bool AddFunction(KIMLCSTRING name, KIMLTYPES returnType, const std::vector<KIMLTYPES> &paramTypes);
	bool AddVariable(KIMLCSTRING name, KIMLTYPES type);
	bool AddConst(KIMLCSTRING name, const Object &value);

	bool CompileInclude(KIMLCSTRING filename, FILE *fp);

	bool Compile(KIMLCSTRING filename, FILE *fp, bool hasDebugInfo);

	const ByteCodeStream & GetByteCodeStream(void) const { return *this->byteCodeStream; }
	const StringPool & GetStringPool(void) const { return *this->stringPool; }

	const std::vector<KIMLCOMPILEERRORINFO> & GetCompileErrors(void) const { return this->errors; }

	void CleanUpAll();

public: // Code Emit API
	bool Emit(OpCodes opcode);
	bool Emit(OpCodes opcode, KIMLINT value);
	bool Emit(OpCodes opcode, KIMLUINT value);
	bool Emit(OpCodes opcode, KIMLFLOAT value);
	bool Emit(OpCodes opcode, KIMLCSTRING value);
	bool EmitJump(OpCodes opcode, KIMLCSTRING label);
	bool EmitFuncDecl(KIMLCSTRING funcName);
	bool AddLabel(KIMLCSTRING label);
	bool MarkLabel(KIMLCSTRING label);
	bool Bake(void);

private:
	bool StartCompileInclude(void);
	bool StartCompile(void);

	void Emit(AstProgram *program);
	void Emit(AstStatement *statement);
	void Emit(AstExpression *expression);

private:
	bool AnalyseSemantics(AstProgram *program);
	bool PerformTypeCheck(AstStatement *statement);
	bool PerformTypeCheck(AstExpression *expression);

	bool AnalyseSemanticsInclude(AstProgram *program);
	bool PerformTypeCheckInclude(AstStatement *statement);

	bool FoldExpression(AstExpression *expression);
};

#endif//__COMPILER_H__
