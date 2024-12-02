#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "kiml.h"

#include "compiler.h"
#include "engine.h"
#include "states.h"
#include "utils.h"
#include "errmsg.h"

#include <vector>
#include <stdarg.h>

#define COMP static_cast<Compiler *>(compiler)
#define ENGINE static_cast<Engine *>(engine)
#define STATES static_cast<States *>(states)

KIML_API KIMLUINT kimlHashName(KIMLCSTRING name)
{
	KIMLUINT h = 0;
	KIMLCSTRING p;

	for (p = name; *p; ++p)
		h = ((h << 4) | (h >> 28)) ^ *p; // _rotl(h, 4) ^ *s

	return h;
}

KIML_API HKIMLCOMPILER kimlCreateCompiler(void)
{
	return new Compiler();
}

KIML_API KIMLVOID kimlFreeCompiler(HKIMLCOMPILER *compiler)
{
	static_cast<Compiler *>(*compiler)->CleanUpAll();
	delete static_cast<Compiler *>(*compiler);
	*compiler = nullptr;
}

KIML_API KIMLBOOL kimlInitCompiler(HKIMLCOMPILER compiler)
{
	return TRUE;
}

KIML_API KIMLBOOL kimlCompileInclude(HKIMLCOMPILER compiler, KIMLCSTRING filename, FILE *fp)
{
	if (!COMP->CompileInclude(filename, fp))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlCompile(HKIMLCOMPILER compiler, KIMLCSTRING filename, FILE *fp, KIMLBOOL hasDebugInfo)
{
	if (!COMP->Compile(filename, fp, hasDebugInfo == TRUE))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLVOID kimlGetCompileError(HKIMLCOMPILER compiler, KIMLCOMPILEERRORINFO **errors, KIMLUINT *errorCount)
{
	auto errList = COMP->GetCompileErrors();
	*errorCount = errList.size();
	*errors = new KIMLCOMPILEERRORINFO[errList.size()];
	if (*errorCount)
		memcpy(*errors, (void *)&errList[0], sizeof(KIMLCOMPILEERRORINFO) * errList.size());
}

KIML_API KIMLCSTRING kimlGetCompileErrorMessage(KIMLCOMPILEERRORCODES errCode)
{
	return compileErrorMessages[errCode];
}

KIML_API KIMLVOID kimlGetByteCode(HKIMLCOMPILER compiler, KIMLBYTE **byteCodeContent, KIMLUINT *byteCodeSize)
{
	KIMLCHAR magic[] = { 'k', 'i', 'm', 'l' };
	KIMLCHAR ver[]   = { '1', '7', '1', '1' };
	KIMLBYTE pad[8]  = { 0 };

	auto byteCodeStream = COMP->GetByteCodeStream();
	auto stringPool = COMP->GetStringPool();

	// header size
	KIMLUINT totalSize = sizeof(magic) + sizeof(ver) + sizeof(pad);
	// code size (and codeSize field size)
	totalSize += byteCodeStream.GetStreamSize() + sizeof(KIMLUINT);
	// string pool size (and poolSize field size)
	totalSize += stringPool.GetPoolSize() + sizeof(KIMLUINT);

	std::vector<KIMLBYTE> stream;
	stream.reserve(totalSize);
	KIMLBYTE buf[16];

	stream.insert(stream.end(), magic, magic + sizeof(magic));
	stream.insert(stream.end(), ver, ver + sizeof(ver));
	stream.insert(stream.end(), pad, pad + sizeof(pad));

	kimlSerialize(buf, byteCodeStream.GetStreamSize());
	stream.insert(stream.end(), buf, buf + sizeof(KIMLUINT));

	if (byteCodeStream.GetStreamSize())
		stream.insert(stream.end(), byteCodeStream.GetStream(), byteCodeStream.GetStream() + byteCodeStream.GetStreamSize());

	kimlSerialize(buf, stringPool.GetPoolSize());
	stream.insert(stream.end(), buf, buf + sizeof(KIMLUINT));

	if (stringPool.GetPoolSize())
		stream.insert(stream.end(), stringPool.GetPool(), stringPool.GetPool() + stringPool.GetPoolSize());

	*byteCodeSize = totalSize;
	*byteCodeContent = new KIMLBYTE[totalSize];
	memcpy(*byteCodeContent, &stream[0], totalSize);
}

KIML_API KIMLBOOL kimlDeclFunction(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLTYPES returnType, ...)
{
	std::vector<KIMLTYPES> params;

	va_list ap;
	va_start(ap, returnType);

	KIMLTYPES paramt = (KIMLTYPES)va_arg(ap, int);
	while (paramt != KIML_NOTYPE)
	{
		params.push_back(paramt);
		paramt = (KIMLTYPES)va_arg(ap, int);
	}

	va_end(ap);

	if (!COMP->AddFunction(name, returnType, params))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlDeclVariable(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLTYPES declType)
{
	if (!COMP->AddVariable(name, declType))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlDefineConstantBool(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLBOOL value)
{
	Object obj = value == TRUE;
	if (!COMP->AddConst(name, obj))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlDefineConstantInt(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLINT value)
{
	Object obj = value;
	if (!COMP->AddConst(name, obj))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlDefineConstantReal(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLFLOAT value)
{
	Object obj = value;
	if (!COMP->AddConst(name, obj))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlDefineConstantString(HKIMLCOMPILER compiler, KIMLCSTRING name, KIMLCSTRING value)
{
	Object obj = value;
	if (!COMP->AddConst(name, obj))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlEmit(HKIMLCOMPILER compiler, OpCodes opcode)
{
	if (!COMP->Emit(opcode))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlEmitInt(HKIMLCOMPILER compiler, OpCodes opcode, KIMLINT value)
{
	if (!COMP->Emit(opcode, value))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlEmitUInt(HKIMLCOMPILER compiler, OpCodes opcode, KIMLUINT value)
{
	if (!COMP->Emit(opcode, value))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlEmitFloat(HKIMLCOMPILER compiler, OpCodes opcode, KIMLFLOAT value)
{
	if (!COMP->Emit(opcode, value))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlEmitString(HKIMLCOMPILER compiler, OpCodes opcode, KIMLCSTRING value)
{
	if (!COMP->Emit(opcode, value))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlEmitJump(HKIMLCOMPILER compiler, OpCodes opcode, KIMLCSTRING label)
{
	if (!COMP->EmitJump(opcode, label))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlEmitFuncDecl(HKIMLCOMPILER compiler, KIMLCSTRING funcName)
{
	if (!COMP->EmitFuncDecl(funcName))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlAddLabel(HKIMLCOMPILER compiler, KIMLCSTRING label)
{
	if (!COMP->AddLabel(label))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlMarkLabel(HKIMLCOMPILER compiler, KIMLCSTRING label)
{
	if (!COMP->MarkLabel(label))
		return FALSE;
	else
		return TRUE;
}

KIML_API KIMLBOOL kimlBake(HKIMLCOMPILER compiler)
{
	if (!COMP->Bake())
		return FALSE;
	else
		return TRUE;
}

/* =====================================================
 * Runtime services
 * =====================================================
 */

KIML_API HKIMLENGINE kimlCreateEngine(void)
{
	return new Engine();
}

KIML_API KIMLVOID kimlFreeEngine(HKIMLENGINE *engine)
{
	delete static_cast<Engine *>(*engine);
	*engine = nullptr;
}

KIML_API KIMLBOOL kimlInitEngine(HKIMLENGINE engine)
{
	return TRUE;
}

KIML_API HKIMLSTATES kimlCreateStates(HKIMLENGINE engine)
{
	return new States();
}

KIML_API KIMLVOID kimlFreeStates(HKIMLSTATES *states)
{
	delete static_cast<States *>(*states);
	*states = nullptr;
}

KIML_API KIMLBOOL kimlInitEngineWithStates(HKIMLENGINE engine, HKIMLSTATES states)
{
	ENGINE->Init(STATES);
	return TRUE;
}

KIML_API KIMLBOOL kimlLoadByteCode(HKIMLSTATES states, const KIMLBYTE *byteCodeContent, KIMLUINT byteCodeSize)
{
	return STATES->loadByteCode(byteCodeContent, byteCodeSize);
}

KIML_API KIMLVOID kimlResetStates(HKIMLSTATES states)
{
	STATES->Reset();
}

KIML_API KIMLVOID kimlSetUserData(HKIMLSTATES states, KIMLVOID *userdata)
{
	STATES->setUserData(userdata);
}

KIML_API KIMLVOID * kimlGetUserData(HKIMLSTATES states)
{
	return STATES->getUserData();
}

KIML_API KIMLBOOL kimlRun(HKIMLENGINE engine)
{
	return ENGINE->Run();
}

KIML_API KIMLRUNTIMEERRORCODES kimlGetLastRuntimeError(HKIMLSTATES states)
{
	return STATES->getLastError();
}

KIML_API KIMLCSTRING kimlGetRuntimeErrorMessage(KIMLRUNTIMEERRORCODES errCode)
{
	return runtimeErrorMessages[errCode];
}

KIML_API KIMLUINT kimlGetLastLine(HKIMLSTATES states)
{
	return STATES->getLastLine();
}

KIML_API KIMLVOID kimlBindFunction(HKIMLSTATES states, KIMLUINT nameHash, KIMLNATIVEFUNC func)
{
	STATES->getFuncs()[nameHash] = RtFunctionInfo(func);
}

KIML_API KIMLVOID kimlStackPush(HKIMLSTATES states, const KIMLVALUE *value)
{
	Object obj(0);
	obj.From(*value);
	STATES->getKStack().push_back(obj);
}

KIML_API KIMLVOID kimlStackPushBool(HKIMLSTATES states, KIMLBOOL value)
{
	STATES->getKStack().push_back(Object(value == TRUE));
}

KIML_API KIMLVOID kimlStackPushInt(HKIMLSTATES states, KIMLINT value)
{
	STATES->getKStack().push_back(Object(value));
}

KIML_API KIMLVOID kimlStackPushReal(HKIMLSTATES states, KIMLFLOAT value)
{
	STATES->getKStack().push_back(Object(value));
}

KIML_API KIMLVOID kimlStackPushString(HKIMLSTATES states, KIMLCSTRING value)
{
	STATES->getKStack().push_back(Object(value));
}

KIML_API KIMLVOID kimlStackPushObject(HKIMLSTATES states, KIMLOBJECT value)
{
	STATES->getKStack().push_back(Object(value));
}

KIML_API KIMLVOID kimlStackPeek(HKIMLSTATES states, KIMLUINT offset, KIMLVALUE *value)
{
	(&STATES->getKStack().back() - (offset - 1))->To(*value);
}

KIML_API KIMLVOID kimlStackPeekBool(HKIMLSTATES states, KIMLUINT offset, KIMLBOOL *value)
{
	*value = (&STATES->getKStack().back() - (offset - 1))->getBool();
}

KIML_API KIMLVOID kimlStackPeekInt(HKIMLSTATES states, KIMLUINT offset, KIMLINT *value)
{
	*value = (&STATES->getKStack().back() - (offset - 1))->getInt();
}

KIML_API KIMLVOID kimlStackPeekReal(HKIMLSTATES states, KIMLUINT offset, KIMLFLOAT *value)
{
	*value = (&STATES->getKStack().back() - (offset - 1))->getFloat();
}

KIML_API KIMLVOID kimlStackPeekString(HKIMLSTATES states, KIMLUINT offset, KIMLCSTRING *value)
{
	*value = (&STATES->getKStack().back() - (offset - 1))->getString();
}

KIML_API KIMLVOID kimlStackPeekObject(HKIMLSTATES states, KIMLUINT offset, KIMLOBJECT *value)
{
	*value = (&STATES->getKStack().back() - (offset - 1))->getObject();
}

KIML_API KIMLVOID kimlStackPop(HKIMLSTATES states, KIMLVALUE *value)
{
	STATES->getKStack().back().To(*value);
	STATES->getKStack().pop_back();
}

KIML_API KIMLVOID kimlStackPopBool(HKIMLSTATES states, KIMLBOOL *value)
{
	*value = STATES->getKStack().back().getBool();
	STATES->getKStack().pop_back();
}

KIML_API KIMLVOID kimlStackPopInt(HKIMLSTATES states, KIMLINT *value)
{
	*value = STATES->getKStack().back().getInt();
	STATES->getKStack().pop_back();
}

KIML_API KIMLVOID kimlStackPopReal(HKIMLSTATES states, KIMLFLOAT *value)
{
	*value = STATES->getKStack().back().getFloat();
	STATES->getKStack().pop_back();
}

KIML_API KIMLVOID kimlStackPopString(HKIMLSTATES states, KIMLCSTRING *value)
{
	*value = STATES->getKStack().back().getString();
	STATES->getKStack().pop_back();
}

KIML_API KIMLVOID kimlStackPopObject(HKIMLSTATES states, KIMLOBJECT *value)
{
	*value = STATES->getKStack().back().getObject();
	STATES->getKStack().pop_back();
}

KIML_API KIMLVOID kimlStackClear(HKIMLSTATES states, KIMLUINT count)
{
	std::vector<Object> &stack = STATES->getKStack();
	if (count)
	{
		for (KIMLUINT i = 0; i < count && stack.size(); ++i)
			stack.pop_back();
	}
	else
	{
		stack.clear();
	}
}

KIML_API KIMLUINT kimlStackSize(HKIMLSTATES states)
{
	return STATES->getKStack().size();
}

KIML_API KIMLVOID kimlTapeWrite(HKIMLSTATES states, KIMLUINT index, const KIMLVALUE *value)
{
	Object obj;
	obj.From(*value);
	STATES->getTape()[index] = obj;
}

KIML_API KIMLVOID kimlTapeWriteBool(HKIMLSTATES states, KIMLUINT index, KIMLBOOL value)
{
	STATES->getTape()[index] = value;
}

KIML_API KIMLVOID kimlTapeWriteInt(HKIMLSTATES states, KIMLUINT index, KIMLINT value)
{
	STATES->getTape()[index] = value;
}

KIML_API KIMLVOID kimlTapeWriteReal(HKIMLSTATES states, KIMLUINT index, KIMLFLOAT value)
{
	STATES->getTape()[index] = value;
}

KIML_API KIMLVOID kimlTapeWriteString(HKIMLSTATES states, KIMLUINT index, KIMLCSTRING value)
{
	STATES->getTape()[index] = value;
}

KIML_API KIMLVOID kimlTapeWriteObject(HKIMLSTATES states, KIMLUINT index, KIMLOBJECT value)
{
	STATES->getTape()[index] = value;
}

KIML_API KIMLVOID kimlTapeRead(HKIMLSTATES states, KIMLUINT index, KIMLVALUE *value)
{
	STATES->getTape()[index].To(*value);
}

KIML_API KIMLVOID kimlTapeReadBool(HKIMLSTATES states, KIMLUINT index, KIMLBOOL *value)
{
	*value = STATES->getTape()[index].getBool();
}

KIML_API KIMLVOID kimlTapeReadInt(HKIMLSTATES states, KIMLUINT index, KIMLINT *value)
{
	*value = STATES->getTape()[index].getInt();
}

KIML_API KIMLVOID kimlTapeReadReal(HKIMLSTATES states, KIMLUINT index, KIMLFLOAT *value)
{
	*value = STATES->getTape()[index].getFloat();
}

KIML_API KIMLVOID kimlTapeReadString(HKIMLSTATES states, KIMLUINT index, KIMLCSTRING *value)
{
	*value = STATES->getTape()[index].getString();
}

KIML_API KIMLVOID kimlTapeReadObject(HKIMLSTATES states, KIMLUINT index, KIMLOBJECT *value)
{
	*value = STATES->getTape()[index].getObject();
}

KIML_API KIMLBOOL kimlCreateVariable(HKIMLSTATES states, KIMLUINT nameHash, const KIMLVALUE *value)
{
	std::map<KIMLUINT, Object> &vars = STATES->getVars();
	if (vars.find(nameHash) != vars.end())
		return FALSE;
	else
	{
		Object obj;
		obj.From(*value);
		vars[nameHash] = obj;
		return TRUE;
	}
}

KIML_API KIMLBOOL kimlCreateVariableBool(HKIMLSTATES states, KIMLUINT nameHash, KIMLBOOL value)
{
	std::map<KIMLUINT, Object> &vars = STATES->getVars();
	if (vars.find(nameHash) != vars.end())
		return FALSE;
	else
	{
		vars[nameHash] = value == TRUE;
		return TRUE;
	}
}

KIML_API KIMLBOOL kimlCreateVariableInt(HKIMLSTATES states, KIMLUINT nameHash, KIMLINT value)
{
	std::map<KIMLUINT, Object> &vars = STATES->getVars();
	if (vars.find(nameHash) != vars.end())
		return FALSE;
	else
	{
		vars[nameHash] = value;
		return TRUE;
	}
}

KIML_API KIMLBOOL kimlCreateVariableReal(HKIMLSTATES states, KIMLUINT nameHash, KIMLFLOAT value)
{
	std::map<KIMLUINT, Object> &vars = STATES->getVars();
	if (vars.find(nameHash) != vars.end())
		return FALSE;
	else
	{
		vars[nameHash] = value;
		return TRUE;
	}
}

KIML_API KIMLBOOL kimlCreateVariableString(HKIMLSTATES states, KIMLUINT nameHash, KIMLCSTRING value)
{
	std::map<KIMLUINT, Object> &vars = STATES->getVars();
	if (vars.find(nameHash) != vars.end())
		return FALSE;
	else
	{
		KIMLVALUE v(KIML_STRING, value);
		vars[nameHash].From(v);
		return TRUE;
	}
}

KIML_API KIMLBOOL kimlCreateVariableObject(HKIMLSTATES states, KIMLUINT nameHash, KIMLOBJECT value)
{
	std::map<KIMLUINT, Object> &vars = STATES->getVars();
	if (vars.find(nameHash) != vars.end())
		return FALSE;
	else
	{
		vars[nameHash] = value;
		return TRUE;
	}
}

KIML_API KIMLBOOL kimlDeleteVariable(HKIMLSTATES states, KIMLUINT nameHash)
{
	STATES->getVars().erase(nameHash);
	return TRUE;
}

KIML_API KIMLBOOL kimlGetVariable(HKIMLSTATES states, KIMLUINT nameHash, KIMLVALUE *value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		it->second.To(*value);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlGetVariableBool(HKIMLSTATES states, KIMLUINT nameHash, KIMLBOOL *value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		*value = it->second.getBool();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlGetVariableInt(HKIMLSTATES states, KIMLUINT nameHash, KIMLINT *value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		*value = it->second.getInt();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlGetVariableReal(HKIMLSTATES states, KIMLUINT nameHash, KIMLFLOAT *value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		*value = it->second.getFloat();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlGetVariableString(HKIMLSTATES states, KIMLUINT nameHash, KIMLCSTRING *value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		*value = it->second.getString();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlGetVariableObject(HKIMLSTATES states, KIMLUINT nameHash, KIMLOBJECT *value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		*value = it->second.getObject();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlSetVariable(HKIMLSTATES states, KIMLUINT nameHash, const KIMLVALUE *value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		Object obj;
		obj.From(*value);
		it->second = obj;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlSetVariableBool(HKIMLSTATES states, KIMLUINT nameHash, KIMLBOOL value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		it->second = value == TRUE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlSetVariableInt(HKIMLSTATES states, KIMLUINT nameHash, KIMLINT value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		it->second = value;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlSetVariableReal(HKIMLSTATES states, KIMLUINT nameHash, KIMLFLOAT value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		it->second = value;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlSetVariableString(HKIMLSTATES states, KIMLUINT nameHash, KIMLCSTRING value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		it->second = value;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLBOOL kimlSetVariableObject(HKIMLSTATES states, KIMLUINT nameHash, KIMLOBJECT value)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
	{
		it->second = value;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

KIML_API KIMLTYPES kimlGetVariableType(HKIMLSTATES states, KIMLUINT nameHash)
{
	auto it = STATES->getVars().find(nameHash);
	if (it != STATES->getVars().end())
		return it->second.getDataType();
	else
		return KIML_NOTYPE;
}

KIML_API KIMLVOID kimlSetReturnValue(HKIMLSTATES states, const KIMLVALUE *value)
{
	Object obj;
	obj.From(*value);
	STATES->getTape()[0] = obj;
}

KIML_API KIMLBOOL kimlInvokeFunction(HKIMLSTATES states, KIMLUINT nameHash)
{
	return STATES->getEngine()->InvokeFunction(nameHash);
}

KIML_API KIMLVOID kimlRaiseEvent(HKIMLSTATES states, KIMLUINT nameHash)
{
	STATES->getEngine()->RaiseEvent(nameHash);
}

KIML_API KIMLVOID kimlFreeData(KIMLVALUE *value)
{
	if (value->type == KIML_STRING)
		delete []value->v.s;

	memset(value, 0, sizeof(KIMLVALUE));
}
