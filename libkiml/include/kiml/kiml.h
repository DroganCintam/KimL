#ifndef __KIML_H__
#define __KIML_H__

#ifdef _MSC_VER
	#ifdef KIML_EXPORTS
		#define KIML_API __declspec(dllexport)
	#else
		#define KIML_API __declspec(dllimport)
	#endif
#else
#define KIML_API
#endif/*_MSC_VER*/

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "opcodes.h"

#define KIML_VERSION 1711

/* =====================================================
 * Types
 * =====================================================
 */

/* Handle of compiler service. */
typedef void *        HKIMLCOMPILER;
/* Handle of runtime service. */
typedef void *        HKIMLENGINE;
/* Handle of runtime states, including stack, tape, variables... */
typedef void *        HKIMLSTATES;

typedef void          KIMLVOID;
typedef char          KIMLCHAR;
typedef unsigned char KIMLBYTE;
typedef int32_t       KIMLINT;
typedef uint32_t      KIMLUINT;
typedef float         KIMLFLOAT;
typedef char *        KIMLSTRING;
typedef const char *  KIMLCSTRING;
typedef void *        KIMLOBJECT;
typedef intptr_t      KIMLNINT;
typedef uintptr_t     KIMLNUINT;

typedef int           KIMLBOOL;
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

enum KIMLTYPES
{
	KIML_NOTYPE  = 0x00,
	KIML_BOOL    = 0x01,
	KIML_INT     = 0x02,
	KIML_REAL    = 0x03,
	KIML_STRING  = 0x04,
	KIML_ANYTYPE = 0x05,
	KIML_OBJECT  = 0x06
};

/* =====================================================
 * Compiler services
 * =====================================================
 */

typedef enum KIMLCOMPILEERRORCODES
{
	KCE_NONE,
	KCE_DUPLICATE_VAR,
	KCE_UNDECLARED_VAR,
	KCE_DUPLICATE_LABEL,
	KCE_UNDECLARED_LABEL,
	KCE_DUPLICATE_FUNC,
	KCE_UNDECLARED_FUNC,
	KCE_DUPLICATE_CONST,
	KCE_EXPRESSION_NONCONST,
	KCE_EXPRESSION_NOTINT,
	KCE_TYPE_MISMATCHED,
	KCE_INCONSISTENT_ARG_COUNT,
	KCE_INCLUDE_VARDECL,
	KCE_INCLUDE_FUNCDECL,
	KCE_INCLUDE_ONLY
} KIMLCOMPILEERRORCODES;

typedef struct KIMLCOMPILEERRORINFO
{
	KIMLCOMPILEERRORCODES code;
	KIMLUINT line;
} KIMLCOMPILEERRORINFO;

KIML_API KIMLUINT kimlHashName(KIMLCSTRING name);

KIML_API HKIMLCOMPILER kimlCreateCompiler(void);

KIML_API KIMLVOID kimlFreeCompiler(HKIMLCOMPILER *compiler);

KIML_API KIMLBOOL kimlInitCompiler(HKIMLCOMPILER compiler);

KIML_API KIMLBOOL kimlCompileInclude(HKIMLCOMPILER compiler, KIMLCSTRING filename, FILE *fp);

KIML_API KIMLBOOL kimlCompile(HKIMLCOMPILER compiler, KIMLCSTRING filename, FILE *fp, KIMLBOOL hasDebugInfo);

KIML_API KIMLVOID kimlGetCompileError(HKIMLCOMPILER compiler, KIMLCOMPILEERRORINFO **errors, KIMLUINT *errorCount);

KIML_API KIMLCSTRING kimlGetCompileErrorMessage(KIMLCOMPILEERRORCODES errCode);

KIML_API KIMLVOID kimlGetByteCode(HKIMLCOMPILER compiler, KIMLBYTE **byteCodeContent, KIMLUINT *byteCodeSize);

KIML_API KIMLBOOL kimlDeclFunction(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLTYPES returnType, ...);

KIML_API KIMLBOOL kimlDeclVariable(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLTYPES declType);

KIML_API KIMLBOOL kimlDefineConstantBool(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLBOOL value);

KIML_API KIMLBOOL kimlDefineConstantInt(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLINT value);

KIML_API KIMLBOOL kimlDefineConstantReal(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLFLOAT value);

KIML_API KIMLBOOL kimlDefineConstantString(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLCSTRING value);

KIML_API KIMLBOOL kimlEmit(HKIMLCOMPILER compiler, OpCodes opcode);
KIML_API KIMLBOOL kimlEmitInt(HKIMLCOMPILER compiler, OpCodes opcode, KIMLINT value);
KIML_API KIMLBOOL kimlEmitUInt(HKIMLCOMPILER compiler, OpCodes opcode, KIMLUINT value);
KIML_API KIMLBOOL kimlEmitFloat(HKIMLCOMPILER compiler, OpCodes opcode, KIMLFLOAT value);
KIML_API KIMLBOOL kimlEmitString(HKIMLCOMPILER compiler, OpCodes opcode, KIMLCSTRING value);
KIML_API KIMLBOOL kimlEmitJump(HKIMLCOMPILER compiler, OpCodes opcode, KIMLCSTRING label);
KIML_API KIMLBOOL kimlEmitFuncDecl(HKIMLCOMPILER compiler, KIMLCSTRING funcName);
KIML_API KIMLBOOL kimlAddLabel(HKIMLCOMPILER compiler, KIMLCSTRING label);
KIML_API KIMLBOOL kimlMarkLabel(HKIMLCOMPILER compiler, KIMLCSTRING label);
KIML_API KIMLBOOL kimlBake(HKIMLCOMPILER compiler);

/* =====================================================
 * Runtime services
 * =====================================================
 */

typedef enum KIMLRUNTIMEERRORCODES
{
	KRE_NONE,
	KRE_ILLEGAL_OPCODE,
	KRE_CANT_CVI,
	KRE_CANT_CVR,
	KRE_STACK_EMPTY,
	KRE_TAPE_INDEX_OOR,
	KRE_CODE_OFFSET_OOR,
	KRE_TYPE_MISMATCHED,
	KRE_STACK_PTR_OOR,
	KRE_DIVISION_BY_ZERO,
	KRE_UNKNOWN_VARIABLE,
	KRE_UNKNOWN_FUNCTION
} KIMLRUNTIMEERRORCODES;

typedef struct KIMLVALUE
{
	KIMLTYPES type;
	union
	{
		KIMLBOOL    b;
		KIMLINT     i;
		KIMLFLOAT   f;
		KIMLCSTRING s;
		KIMLOBJECT  o;
	} v;

#ifdef __cplusplus
	KIMLVALUE(void)
		: type(KIML_INT) { this->v.i = 0; }
	KIMLVALUE(KIMLTYPES type, bool v)
		: type(type) { this->v.b = v ? TRUE : FALSE; }
	KIMLVALUE(KIMLTYPES type, KIMLINT v)
		: type(type) { this->v.i = v; }
	KIMLVALUE(KIMLTYPES type, KIMLFLOAT v)
		: type(type) { this->v.f = v; }
	KIMLVALUE(KIMLTYPES type, KIMLCSTRING v)
		: type(type) { this->v.s = v; }
	KIMLVALUE(KIMLTYPES type, KIMLOBJECT v)
		: type(type) { this->v.o = v; }
#endif

} KIMLVALUE;

typedef KIMLBOOL (*KIMLNATIVEFUNC)(HKIMLSTATES states);

KIML_API HKIMLENGINE kimlCreateEngine(void);

KIML_API KIMLVOID kimlFreeEngine(HKIMLENGINE *engine);

KIML_API KIMLBOOL kimlInitEngine(HKIMLENGINE engine);

KIML_API HKIMLSTATES kimlCreateStates(HKIMLENGINE engine);

KIML_API KIMLVOID kimlFreeStates(HKIMLSTATES *states);

KIML_API KIMLBOOL kimlInitEngineWithStates(HKIMLENGINE engine, HKIMLSTATES states);

KIML_API KIMLBOOL kimlLoadByteCode(HKIMLSTATES states, const KIMLBYTE *byteCodeContent, KIMLUINT byteCodeSize);

KIML_API KIMLVOID kimlResetStates(HKIMLSTATES states);

KIML_API KIMLVOID kimlSetUserData(HKIMLSTATES states, KIMLVOID *userdata);

KIML_API KIMLVOID * kimlGetUserData(HKIMLSTATES states);

KIML_API KIMLBOOL kimlRun(HKIMLENGINE engine);

KIML_API KIMLRUNTIMEERRORCODES kimlGetLastRuntimeError(HKIMLSTATES states);

KIML_API KIMLCSTRING kimlGetRuntimeErrorMessage(KIMLRUNTIMEERRORCODES errCode);

KIML_API KIMLUINT kimlGetLastLine(HKIMLSTATES states);

KIML_API KIMLVOID kimlBindFunction(HKIMLSTATES states, KIMLUINT nameHash, KIMLNATIVEFUNC func);

KIML_API KIMLVOID kimlStackPush(HKIMLSTATES states, const KIMLVALUE *value);

KIML_API KIMLVOID kimlStackPushBool(HKIMLSTATES states, KIMLBOOL value);

KIML_API KIMLVOID kimlStackPushInt(HKIMLSTATES states, KIMLINT value);

KIML_API KIMLVOID kimlStackPushReal(HKIMLSTATES states, KIMLFLOAT value);

KIML_API KIMLVOID kimlStackPushString(HKIMLSTATES states, KIMLCSTRING value);

KIML_API KIMLVOID kimlStackPushObject(HKIMLSTATES states, KIMLOBJECT value);

KIML_API KIMLVOID kimlStackPeek(HKIMLSTATES states, KIMLUINT offset, KIMLVALUE *value);

KIML_API KIMLVOID kimlStackPeekBool(HKIMLSTATES states, KIMLUINT offset, KIMLBOOL *value);

KIML_API KIMLVOID kimlStackPeekInt(HKIMLSTATES states, KIMLUINT offset, KIMLINT *value);

KIML_API KIMLVOID kimlStackPeekReal(HKIMLSTATES states, KIMLUINT offset, KIMLFLOAT *value);

KIML_API KIMLVOID kimlStackPeekString(HKIMLSTATES states, KIMLUINT offset, KIMLCSTRING *value);

KIML_API KIMLVOID kimlStackPeekObject(HKIMLSTATES states, KIMLUINT offset, KIMLOBJECT *value);

KIML_API KIMLVOID kimlStackPop(HKIMLSTATES states, KIMLVALUE *value);

KIML_API KIMLVOID kimlStackPopBool(HKIMLSTATES states, KIMLBOOL *value);

KIML_API KIMLVOID kimlStackPopInt(HKIMLSTATES states, KIMLINT *value);

KIML_API KIMLVOID kimlStackPopReal(HKIMLSTATES states, KIMLFLOAT *value);

KIML_API KIMLVOID kimlStackPopString(HKIMLSTATES states, KIMLCSTRING *value);

KIML_API KIMLVOID kimlStackPopObject(HKIMLSTATES states, KIMLOBJECT *value);

KIML_API KIMLVOID kimlStackClear(HKIMLSTATES states, KIMLUINT count);

KIML_API KIMLUINT kimlStackSize(HKIMLSTATES states);

KIML_API KIMLVOID kimlTapeWrite(HKIMLSTATES states, KIMLUINT index, const KIMLVALUE *value);

KIML_API KIMLVOID kimlTapeWriteBool(HKIMLSTATES states, KIMLUINT index, KIMLBOOL value);

KIML_API KIMLVOID kimlTapeWriteInt(HKIMLSTATES states, KIMLUINT index, KIMLINT value);

KIML_API KIMLVOID kimlTapeWriteReal(HKIMLSTATES states, KIMLUINT index, KIMLFLOAT value);

KIML_API KIMLVOID kimlTapeWriteString(HKIMLSTATES states, KIMLUINT index, KIMLCSTRING value);

KIML_API KIMLVOID kimlTapeWriteObject(HKIMLSTATES states, KIMLUINT index, KIMLOBJECT value);

KIML_API KIMLVOID kimlTapeRead(HKIMLSTATES states, KIMLUINT index, KIMLVALUE *value);

KIML_API KIMLVOID kimlTapeReadBool(HKIMLSTATES states, KIMLUINT index, KIMLBOOL *value);

KIML_API KIMLVOID kimlTapeReadInt(HKIMLSTATES states, KIMLUINT index, KIMLINT *value);

KIML_API KIMLVOID kimlTapeReadReal(HKIMLSTATES states, KIMLUINT index, KIMLFLOAT *value);

KIML_API KIMLVOID kimlTapeReadString(HKIMLSTATES states, KIMLUINT index, KIMLCSTRING *value);

KIML_API KIMLVOID kimlTapeReadObject(HKIMLSTATES states, KIMLUINT index, KIMLOBJECT *value);

KIML_API KIMLBOOL kimlCreateVariable(HKIMLSTATES states, KIMLUINT nameHash, const KIMLVALUE *value);

KIML_API KIMLBOOL kimlCreateVariableBool(HKIMLSTATES states, KIMLUINT nameHash, KIMLBOOL value);

KIML_API KIMLBOOL kimlCreateVariableInt(HKIMLSTATES states, KIMLUINT nameHash, KIMLINT value);

KIML_API KIMLBOOL kimlCreateVariableReal(HKIMLSTATES states, KIMLUINT nameHash, KIMLFLOAT value);

KIML_API KIMLBOOL kimlCreateVariableString(HKIMLSTATES states, KIMLUINT nameHash, KIMLCSTRING value);

KIML_API KIMLBOOL kimlCreateVariableObject(HKIMLSTATES states, KIMLUINT nameHash, KIMLOBJECT value);

KIML_API KIMLBOOL kimlDeleteVariable(HKIMLSTATES states, KIMLUINT nameHash);

KIML_API KIMLBOOL kimlGetVariable(HKIMLSTATES states, KIMLUINT nameHash, KIMLVALUE *value);

KIML_API KIMLBOOL kimlGetVariableBool(HKIMLSTATES states, KIMLUINT nameHash, KIMLBOOL *value);

KIML_API KIMLBOOL kimlGetVariableInt(HKIMLSTATES states, KIMLUINT nameHash, KIMLINT *value);

KIML_API KIMLBOOL kimlGetVariableReal(HKIMLSTATES states, KIMLUINT nameHash, KIMLFLOAT *value);

KIML_API KIMLBOOL kimlGetVariableString(HKIMLSTATES states, KIMLUINT nameHash, KIMLCSTRING *value);

KIML_API KIMLBOOL kimlGetVariableObject(HKIMLSTATES states, KIMLUINT nameHash, KIMLOBJECT *value);

KIML_API KIMLBOOL kimlSetVariable(HKIMLSTATES states, KIMLUINT nameHash, const KIMLVALUE *value);

KIML_API KIMLBOOL kimlSetVariableBool(HKIMLSTATES states, KIMLUINT nameHash, KIMLBOOL value);

KIML_API KIMLBOOL kimlSetVariableInt(HKIMLSTATES states, KIMLUINT nameHash, KIMLINT value);

KIML_API KIMLBOOL kimlSetVariableReal(HKIMLSTATES states, KIMLUINT nameHash, KIMLFLOAT value);

KIML_API KIMLBOOL kimlSetVariableString(HKIMLSTATES states, KIMLUINT nameHash, KIMLCSTRING value);

KIML_API KIMLBOOL kimlSetVariableObject(HKIMLSTATES states, KIMLUINT nameHash, KIMLOBJECT value);

KIML_API KIMLTYPES kimlGetVariableType(HKIMLSTATES states, KIMLUINT nameHash);

KIML_API KIMLVOID kimlSetReturnValue(HKIMLSTATES states, const KIMLVALUE *value);

KIML_API KIMLBOOL kimlInvokeFunction(HKIMLSTATES states, KIMLUINT nameHash);

KIML_API KIMLVOID kimlRaiseEvent(HKIMLSTATES states, KIMLUINT nameHash);

KIML_API KIMLVOID kimlFreeData(KIMLVALUE *value);

#ifdef __cplusplus
}
#endif

#endif/*__KIML_H__*/
