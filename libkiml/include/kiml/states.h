#ifndef __STATES_H__
#define __STATES_H__

#include "kiml.h"
#include "object.h"

#include <map>
#include <stack>
#include <vector>

#define TAPESIZE 256

struct RtFunctionInfo
{
	bool isNative;
	union
	{
		KIMLNATIVEFUNC funcptr;
		KIMLUINT       addr;
	};

	RtFunctionInfo(void)
		: isNative(false), addr(0) { }

	RtFunctionInfo(KIMLNATIVEFUNC funcptr)
		: isNative(true), funcptr(funcptr) { }

	RtFunctionInfo(KIMLUINT addr)
		: isNative(false), addr(addr) { }
};

class Engine;

class States
{
	friend class Engine;

private:
	Engine *engine;

	KIMLBYTE *code;
	KIMLUINT codesize;
	KIMLUINT codepointer;
	
	KIMLUINT line;
	KIMLRUNTIMEERRORCODES err;

	KIMLCHAR *stringpool;
	KIMLUINT stringpoolsize;
	
	std::stack<KIMLUINT> callstack;

	Object tape[TAPESIZE];
	KIMLUINT tapepointer;
	std::vector<Object> kstack;
	std::vector<Object> estack;
	
	std::map<KIMLUINT, Object> vars;
	std::map<KIMLUINT, RtFunctionInfo> funcs;
	std::map<KIMLUINT, std::vector<KIMLUINT> > events;

	bool isRunning;

	KIMLVOID *userdata;

public:
	States(void)
		: code(nullptr), codesize(0), codepointer(0),
		line(0), err(KRE_NONE),
		stringpool(nullptr), stringpoolsize(0),
		tapepointer(0),
		isRunning(false),
		userdata(nullptr) { }

	~States(void)
	{
		this->CleanUp();
	}

	void Reset(void)
	{
		codepointer = 0;
		line = 0;
		err = KRE_NONE;

		kstack.clear();
		estack.clear();
	}

	void ClearSymbols(void)
	{
		vars.clear();
		funcs.clear();
		events.clear();
	}

	void CleanUp(void)
	{
		Reset();

		ClearSymbols();

		delete []code;
		code = nullptr;

		delete []stringpool;
		stringpool = nullptr;
	}

	Engine * getEngine(void) { return engine; }

	bool loadByteCode(const KIMLBYTE *byteCodeContent, KIMLUINT byteCodeSize);

	KIMLUINT getLastLine(void) { return line; }

	KIMLRUNTIMEERRORCODES getLastError(void) const { return err; }

	Object * getTape(void) { return tape; }

	KIMLUINT & getTapePointer(void) { return tapepointer; }
	
	std::vector<Object> & getKStack(void) { return kstack; }
	
	std::vector<Object> & getEStack(void) { return estack; }

	std::map<KIMLUINT, Object> & getVars(void) { return vars; }

	std::map<KIMLUINT, RtFunctionInfo> & getFuncs(void) { return funcs; }

	KIMLVOID * getUserData(void) const { return userdata; }

	void setUserData(KIMLVOID *userdata) { this->userdata = userdata; }
};

#endif//__STATES_H__
