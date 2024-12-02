#ifndef __SIMPL_COMPILER_H__
#define __SIMPL_COMPILER_H__

#include <kiml/kiml.h>
#include <stack>
#include "simpl.h"
#include "errmsg.h"
#include "ast.h"
#include "constants.h"
#include "functions.h"
#include "scope.h"

class SimpLCompiler
{
private:
	struct CompileUnit
	{
		std::string fileName;
		AstProgram *root;
	};

private:
	HKIMLCOMPILER compiler;
	ConstTable *constTable;
	SimpLFunctionTable *functionTable;

	std::vector<CompileUnit> compileUnits;
	KIMLUINT currentCompileUnitIndex;

	std::vector<SIMPLCOMPILEERRORINFO> errors;

	bool hasDebugInfo;

	Scope *rootScope;
	Scope *currentScope;

	const SimpLFunctionInfo *currentFunction;

	std::stack<std::string> loopBeginLabels;
	std::stack<std::string> loopEndLabels;

public:
	SimpLCompiler(void);
	~SimpLCompiler(void);

public:
	bool Parse(const char *fileName, const char *fileContent);
	bool Parse(const char *fileName, FILE *fp);
	bool Compile(bool hasDebugInfo);
	bool Finish(void);
	bool GetByteCode(KIMLBYTE **byteCode, KIMLUINT *byteCodeSize);
	void CleanUpAll(void);

	const std::vector<SIMPLCOMPILEERRORINFO> & GetErrors(void) const { return errors; }

private:
	bool ScanDeclarations(AstProgram *root);
	bool Emit(AstProgram *root);
	bool Emit(AstStatement *statement);
	bool Emit(AstVar *statement);
	bool Emit(AstFunc *statement);
	bool Emit(AstStatementList *statement);
	bool Emit(AstIf *statement);
	bool Emit(AstWhile *statement);
	bool Emit(AstDo *statement);
	bool Emit(AstBreak *statement);
	bool Emit(AstContinue *statement);
	bool Emit(AstReturn *statement);
	bool Emit(AstExprStatement *statement);
	bool Emit(AstExpression *expression);
	void EmitCast(int typeMatchResult, KIMLTYPES toType);

	void Emit(OpCodes opcode);
	void Emit(OpCodes opcode, KIMLINT value);
	void Emit(OpCodes opcode, KIMLUINT value);
	void Emit(OpCodes opcode, KIMLFLOAT value);
	void Emit(OpCodes opcode, KIMLCSTRING value);
	void AddLabel(const std::string &label);
	void MarkLabel(const std::string &label);
	void EmitJump(OpCodes opcode, const std::string &label);

	bool PerformTypeCheck(AstExpression *expression);
	bool FoldExpression(AstExpression *expression);

	void EnterScope(void);
	void LeaveScope(void);

	std::string GetScopedName(const char *name);
	std::string GetScopedName(const char *name, int depth);
	std::string AllocLabel(void);
};

#endif//__SIMPL_COMPILER_H__
