#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "engine.h"

#include "opcodes.h"
#include "states.h"

#include <iostream>
#include <algorithm>

OPEXECUTOR Engine::Executors[256];

OPEXECUTOR Engine::DefaultExecutors[] =
{
	Engine::do_nop,    // no operation
	Engine::do_ldi,    // load int
	Engine::do_ldr,    // load real
	Engine::do_lds,    // load string
	Engine::do_ldes,   // load empty string
	Engine::do_ldvar,  // load variable
	Engine::do_stvar,  // store variable
	Engine::do_new,    // new real variable
	Engine::do_free,   // delete object variable
	Engine::do_cvi,    // convert to int
	Engine::do_cvr,    // convert to real
	Engine::do_cvs,    // convert to string
	Engine::do_cvin,   // convert to int, Nothrow
	Engine::do_cvrn,   // convert to real, Nothrow
	Engine::do_in,     // standard input
	Engine::do_out,    // standard output
	Engine::do_push,   // push to k-stack
	Engine::do_pop,    // pop from k-stack
	Engine::do_stf,    // stack transfer (k-stack to eval-stack)
	Engine::do_smv,    // stack move (k-stack to eval-stack)
	Engine::do_swap,   // stack swap (k-stack)
	Engine::do_sclr,   // stack clear (k-stack)
	Engine::do_sclrc,  // stack clear, counted (k-stack)
	Engine::do_trd,    // tape read
	Engine::do_trdx,   // tape read at indeX
	Engine::do_twr,    // tape write
	Engine::do_twrx,   // tape write at indeX
	Engine::do_tnxt,   // tape next
	Engine::do_tprv,   // tape previous
	Engine::do_tmov,   // tape move
	Engine::do_jmp,    // jump
	Engine::do_cjmp,   // conditional jump
	Engine::do_call,   // call
	Engine::do_ccall,  // conditional call
	Engine::do_ret,    // return
	Engine::do_hlt,    // halt (terminate the program)

	Engine::do_add,    // addition
	Engine::do_sub,    // subtraction
	Engine::do_mul,    // multiplication
	Engine::do_div,    // division
	Engine::do_idiv,   // integral division
	Engine::do_mod,    // modulus
	Engine::do_pow,    // exponentiation
	Engine::do_equ,    // equal
	Engine::do_neq,    // not equal
	Engine::do_lt,     // less than
	Engine::do_lte,    // less than or equal
	Engine::do_gt,     // greater than
	Engine::do_gte,    // greater than or equal
	Engine::do_cat,    // concatenate (string)
	Engine::do_and,    // logical and
	Engine::do_or,     // logical or
	Engine::do_xor,    // logical exclusive or
	Engine::do_not,    // logical not
	Engine::do_neg,    // negation
	Engine::do_func,   // built-in function
	Engine::do_trace,  // for debug mode

	Engine::do_ldtrue,   // load constant TRUE
	Engine::do_ldfalse,  // load constant FALSE
	Engine::do_cvb,      // convert to bool (no-throw)
	Engine::do_fdecl,    // declare function
	Engine::do_fcall,    // call function
	Engine::do_ehnd,     // handle event
	Engine::do_eunhnd,   // unhandle event
	Engine::do_eraise,   // raise event

	Engine::do_srd,      // stack read
	Engine::do_swr,      // stack write
	Engine::do_srd1,
	Engine::do_srd2,
	Engine::do_srd3,
	Engine::do_srd4,
	Engine::do_srd5,
	Engine::do_swr1,
	Engine::do_swr2,
	Engine::do_swr3,
	Engine::do_swr4,
	Engine::do_swr5,
	Engine::do_trd0,     // tape read 0
	Engine::do_twr0,     // tape write 0
	Engine::do_trs0,     // tape read 0 to stack
	Engine::do_ldnull,
	Engine::do_cvo,
};

FUNCEXECUTOR Engine::FuncExecutors[] =
{
	Engine::f_abs,
	Engine::f_sqrt,
	Engine::f_sin,
	Engine::f_cos,
	Engine::f_tan,
	Engine::f_asin,
	Engine::f_acos,
	Engine::f_atan,
	Engine::f_chr,
	Engine::f_asc,
	Engine::f_len,
	Engine::f_left,
	Engine::f_mid,
	Engine::f_right,
	Engine::f_iif,
	Engine::f_tape,
	Engine::f_stack,
	Engine::f_pop,
	Engine::f_peek,
	Engine::f_min,
	Engine::f_max,
	Engine::f_random,
	Engine::f_print,
	Engine::f_readln,
};

void Engine::PrepareExecutors(void)
{
	static bool prepared = false;

	if (prepared)
		return;

	int i = 0;

	// Each legal opcode has a distinct executor:
	for (; i < op_TOTAL; ++i)
		Engine::Executors[i] = Engine::DefaultExecutors[i];

	// All illegal opcodes have the same executor:
	for (; i < 256; ++i)
		Engine::Executors[i] = Engine::do_illegal;

	prepared = true;
}

void Engine::Init(States *states)
{
	this->states = states;
	states->engine = this;
}

bool Engine::Run(void)
{
	Engine::PrepareExecutors();

	States *states = this->states;
	
	states->codepointer = 0;
	states->line = 0;
	states->err = KRE_NONE;

	states->isRunning = true;

	while (states->isRunning)
		Executors[states->code[states->codepointer]](states);

	if (states->err)
		return false;
	else
		return true;
}

bool Engine::InvokeFunction(KIMLUINT nameHash)
{
	Engine::PrepareExecutors();

	States *states = this->states;

	states->isRunning = true;

	const RtFunctionInfo &fi = states->funcs[nameHash];
	if (fi.isNative)
	{
		fi.funcptr(states);
	}
	else
	{
		states->codepointer = fi.addr;
		while (states->isRunning)
			Executors[states->code[states->codepointer]](states);
	}

	if (states->err)
		return false;
	else
		return true;
}

void Engine::RaiseEvent(KIMLUINT nameHash)
{
	Engine::PrepareExecutors();

	States *states = this->states;

	states->Reset();

	auto it = states->events.find(nameHash);
	if (it != states->events.end())
	{
		std::vector<KIMLUINT> handlers = it->second;

		for (auto it_handler = handlers.begin(); it_handler != handlers.end(); ++it_handler)
		{
			KIMLUINT offset = *it_handler;
			states->callstack.push(states->codepointer);
			states->codepointer = offset;

			states->isRunning = true;
			KIMLBYTE opcode;
			while (states->isRunning && (opcode = states->code[states->codepointer]) != op_ret)
				Executors[opcode](states);

			if (states->err)
				break;
		}
	}
}

void Engine::do_nop(States *states)
{
	++states->codepointer;
}

void Engine::do_ldi(States *states)
{
	KIMLINT i = 0;
	
	// skips op_ldi
	++states->codepointer;

	// takes the KIMLINT
	i = kimlDeserializeInt(states->code + states->codepointer);

	// increments states->codepointer
	states->codepointer += sizeof(KIMLINT);

	states->estack.push_back(Object(i));
}

void Engine::do_ldr(States *states)
{
	KIMLFLOAT r = (KIMLFLOAT)0;

	++states->codepointer;
	r = kimlDeserializeFloat(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLFLOAT);

	states->estack.push_back(Object(r));
}

void Engine::do_lds(States *states)
{
	KIMLUINT offset = 0;
	
	++states->codepointer;
	offset = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	states->estack.push_back(Object(states->stringpool + offset));
}

void Engine::do_ldes(States *states)
{
	++states->codepointer;

	states->estack.push_back(Object(""));
}

void Engine::do_ldvar(States *states)
{
	KIMLUINT h = 0;

	++states->codepointer;
	h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	auto it = states->vars.find(h);
	if (it != states->vars.end())
	{
		states->estack.push_back(it->second);
	}
	else
	{
		states->err = KRE_UNKNOWN_VARIABLE;
		states->isRunning = false;
	}
}

void Engine::do_stvar(States *states)
{
	KIMLUINT h = 0;

	++states->codepointer;
	h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);
	
	auto it = states->vars.find(h);
	if (it != states->vars.end())
	{
		it->second = states->estack.back();
		states->estack.pop_back();
	}
	else
	{
		states->err = KRE_UNKNOWN_VARIABLE;
		states->isRunning = false;
	}
}

void Engine::do_new(States *states)
{
	KIMLUINT h = 0;

	++states->codepointer;
	h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	states->vars[h] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_free(States *states)
{
	KIMLUINT h = 0;

	++states->codepointer;
	h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	states->vars.erase(h);
}

void Engine::do_cvi(States *states)
{
	++states->codepointer;

	Object obj(states->estack.back());
	states->estack.pop_back();

	if (obj.getDataType() == KIML_BOOL || obj.getDataType() == KIML_STRING)
	{
		states->err = KRE_CANT_CVI;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(Object(obj.getInt()));
}

void Engine::do_cvr(States *states)
{
	++states->codepointer;

	Object obj(states->estack.back());
	states->estack.pop_back();

	if (obj.getDataType() == KIML_BOOL || obj.getDataType() == KIML_STRING)
	{
		states->err = KRE_CANT_CVR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(Object(obj.getFloat()));
}

void Engine::do_cvs(States *states)
{
	++states->codepointer;

	KIMLCSTRING s = states->estack.back().getString();
	Object obj(s);
	states->estack.pop_back();

	states->estack.push_back(obj);
	delete []s;
}

void Engine::do_cvin(States *states)
{
	++states->codepointer;

	Object obj(states->estack.back().getInt());
	states->estack.pop_back();

	states->estack.push_back(obj);
}

void Engine::do_cvrn(States *states)
{
	++states->codepointer;

	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(obj);
}

void Engine::do_in(States *states)
{
	++states->codepointer;

	std::string s;
	std::getline(std::cin, s);

	states->estack.push_back(Object(s.c_str()));
}

void Engine::do_out(States *states)
{
	++states->codepointer;

	KIMLCSTRING s = states->estack.back().getString();
	printf("%s", s);
	states->estack.pop_back();
	delete []s;
}

void Engine::do_push(States *states)
{
	++states->codepointer;

	states->kstack.push_back(states->estack.back());
	states->estack.pop_back();
}

void Engine::do_pop(States *states)
{
	++states->codepointer;

	if (states->kstack.empty())
	{
		states->err = KRE_STACK_EMPTY;
		states->isRunning = false;
		return;
	}

	states->kstack.pop_back();
}

void Engine::do_stf(States *states)
{
	++states->codepointer;
	
	if (states->kstack.empty())
	{
		states->err = KRE_STACK_EMPTY;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(states->kstack.back());
}

void Engine::do_smv(States *states)
{
	++states->codepointer;
	
	if (states->kstack.empty())
	{
		states->err = KRE_STACK_EMPTY;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(states->kstack.back());
	states->kstack.pop_back();
}

void Engine::do_swap(States *states)
{
	++states->codepointer;

	Object a, b;
	
	if (states->kstack.size() < 2)
	{
		states->err = KRE_STACK_EMPTY;
		states->isRunning = false;
		return;
	}

	a = states->kstack.back(); states->kstack.pop_back();
	b = states->kstack.back(); states->kstack.pop_back();

	states->kstack.push_back(a);
	states->kstack.push_back(b);
}

void Engine::do_sclr(States *states)
{
	++states->codepointer;

	states->kstack.clear();
}

void Engine::do_sclrc(States *states)
{
	KIMLUINT c = 0;

	++states->codepointer;
	c = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	KIMLUINT i;
	for (i = 0; i < c && !states->kstack.empty(); ++i)
		states->kstack.pop_back();

	if (i < c)
	{
		states->err = KRE_STACK_EMPTY;
		states->isRunning = false;
	}
}

void Engine::do_trd(States *states)
{
	++states->codepointer;

	states->estack.push_back(states->tape[states->tapepointer]);
}

void Engine::do_trdx(States *states)
{
	KIMLUINT idx = 0;

	++states->codepointer;
	idx = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	if (idx < 0 || idx >= TAPESIZE)
	{
		states->err = KRE_TAPE_INDEX_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(states->tape[idx]);
}

void Engine::do_twr(States *states)
{
	++states->codepointer;

	states->tape[states->tapepointer] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_twrx(States *states)
{
	KIMLUINT idx = 0;

	++states->codepointer;
	idx = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	if (idx < 0 || idx >= TAPESIZE)
	{
		states->err = KRE_TAPE_INDEX_OOR;
		states->isRunning = false;
		return;
	}

	states->tape[idx] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_tnxt(States *states)
{
	++states->codepointer;

	if (states->tapepointer < TAPESIZE - 1)
		++states->tapepointer;
	else
		states->tapepointer = 0;
}

void Engine::do_tprv(States *states)
{
	++states->codepointer;

	if (states->tapepointer > 0)
		--states->tapepointer;
	else
		states->tapepointer = TAPESIZE - 1;
}

void Engine::do_tmov(States *states)
{
	++states->codepointer;

	Object obj(states->estack.back());
	states->estack.pop_back();

	KIMLINT idx = obj.getInt();
	if (idx < 0 || idx >= TAPESIZE)
	{
		states->err = KRE_TAPE_INDEX_OOR;
		states->isRunning = false;
		return;
	}
	
	states->tapepointer = idx;
}

void Engine::do_jmp(States *states)
{
	KIMLUINT offset = 0;

	++states->codepointer;
	offset = kimlDeserializeUInt(states->code + states->codepointer);

	states->codepointer = offset;
}

void Engine::do_cjmp(States *states)
{
	KIMLUINT offset = 0;

	++states->codepointer;
	offset = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	Object obj(states->estack.back());
	states->estack.pop_back();

	if (obj.getBool())
		states->codepointer = offset;
}

void Engine::do_call(States *states)
{
	KIMLUINT offset = 0;

	++states->codepointer;
	offset = kimlDeserializeUInt(states->code + states->codepointer);

	states->callstack.push(states->codepointer + sizeof(KIMLUINT));
	states->codepointer = offset;
}

void Engine::do_ccall(States *states)
{
	KIMLUINT offset = 0;

	++states->codepointer;
	offset = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	Object obj(states->estack.back());
	states->estack.pop_back();

	if (obj.getBool())
	{
		states->callstack.push(states->codepointer);
		states->codepointer = offset;
	}
}

void Engine::do_ret(States *states)
{
	if (states->callstack.empty())
	{
		states->codepointer = 0;
		states->isRunning = false;
	}
	else
	{
		states->codepointer = states->callstack.top();
		states->callstack.pop();
	}
}

void Engine::do_hlt(States *states)
{
	++states->codepointer;
	states->isRunning = false;
}

void Engine::do_add(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) > KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a + b);
}

void Engine::do_sub(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a - b);
}

void Engine::do_mul(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a * b);
}

void Engine::do_div(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	if (b.getDataType() == KIML_INT && b.getInt() == 0
		|| b.getDataType() == KIML_REAL && b.getFloat() == 0)
	{
		states->err = KRE_DIVISION_BY_ZERO;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a / b);
}

void Engine::do_idiv(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	if (b.getDataType() == KIML_INT && b.getInt() == 0
		|| b.getDataType() == KIML_REAL && b.getFloat() == 0)
	{
		states->err = KRE_DIVISION_BY_ZERO;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a / b);
}

void Engine::do_mod(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a % b);
}

void Engine::do_pow(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(Object::Pow(a, b));
}

void Engine::do_equ(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	states->estack.push_back(a == b);
}

void Engine::do_neq(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	states->estack.push_back(a != b);
}

void Engine::do_lt(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a < b);
}

void Engine::do_lte(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a <= b);
}

void Engine::do_gt(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a > b);
}

void Engine::do_gte(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(a >= b);
}

void Engine::do_cat(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	states->estack.push_back(Object::Concat(a, b));
}

void Engine::do_and(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	states->estack.push_back(a && b);
}

void Engine::do_or(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	states->estack.push_back(a || b);
}

void Engine::do_xor(States *states)
{
	++states->codepointer;

	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	states->estack.push_back(a ^ b);
}

void Engine::do_not(States *states)
{
	++states->codepointer;

	Object a(states->estack.back());
	states->estack.pop_back();

	states->estack.push_back(!a);
}

void Engine::do_neg(States *states)
{
	++states->codepointer;

	Object a(states->estack.back());
	states->estack.pop_back();

	if (a.getDataType() == KIML_STRING)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(-a);
}

void Engine::do_func(States *states)
{
	KIMLUINT fidx = 0;

	++states->codepointer;
	fidx = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	FuncExecutors[fidx](states);
}

void Engine::do_trace(States *states)
{
	++states->codepointer;
	states->line = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLINT);
}

void Engine::do_ldtrue(States *states)
{
	++states->codepointer;
	states->estack.push_back(Object((KIMLBOOL)TRUE));
}

void Engine::do_ldfalse(States *states)
{
	++states->codepointer;
	states->estack.push_back(Object((KIMLBOOL)FALSE));
}

void Engine::do_cvb(States *states)
{
	++states->codepointer;

	Object obj(states->estack.back().getBool() != 0);
	states->estack.pop_back();

	states->estack.push_back(obj);
}

void Engine::do_fdecl(States *states)
{
	++states->codepointer;

	KIMLUINT h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	KIMLUINT addr = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	states->funcs[h] = RtFunctionInfo(addr);
}

void Engine::do_fcall(States *states)
{
	++states->codepointer;

	KIMLUINT h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	auto it = states->funcs.find(h);
	if (it != states->funcs.end())
	{
		const RtFunctionInfo &fi = it->second;
		if (fi.isNative)
		{
			fi.funcptr(states);
		}
		else
		{
			states->callstack.push(states->codepointer);
			states->codepointer = fi.addr;
		}
	}
	else
	{
		states->err = KRE_UNKNOWN_FUNCTION;
		states->isRunning = false;
	}
}

void Engine::do_ehnd(States *states)
{
	++states->codepointer;

	KIMLUINT h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	KIMLUINT offset = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	auto it = states->events.find(h);
	if (it != states->events.end())
	{
		std::vector<KIMLUINT> &handlers = it->second;
		auto it_handler = std::find(handlers.begin(), handlers.end(), offset);
		if (it_handler == handlers.end())
			handlers.push_back(offset);
	}
	else
	{
		std::vector<KIMLUINT> handlers;
		handlers.push_back(offset);
		states->events[h] = handlers;
	}
}

void Engine::do_eunhnd(States *states)
{
	++states->codepointer;

	KIMLUINT h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	KIMLUINT offset = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	auto it = states->events.find(h);
	if (it != states->events.end())
	{
		std::vector<KIMLUINT> &handlers = it->second;
		auto it_handler = std::find(handlers.begin(), handlers.end(), offset);
		if (it_handler != handlers.end())
			handlers.erase(it_handler);
	}
}

void Engine::do_eraise(States *states)
{
	++states->codepointer;

	KIMLUINT h = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	auto it = states->events.find(h);
	if (it != states->events.end())
	{
		std::vector<KIMLUINT> &handlers = it->second;

		for (auto it_handler = handlers.begin(); it_handler != handlers.end(); ++it_handler)
		{
			KIMLUINT offset = *it_handler;
			states->callstack.push(states->codepointer);
			states->codepointer = offset;

			states->isRunning = true;
			KIMLBYTE opcode;
			while (states->isRunning && (opcode = states->code[states->codepointer]) != op_ret)
				Executors[opcode](states);

			if (states->err)
				break;
		}
	}
}

void Engine::do_srd(States *states)
{
	KIMLUINT offset = 0;

	++states->codepointer;
	offset = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	if (offset < 1 || offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(*( &states->kstack.back() - (offset - 1) ));
}

void Engine::do_swr(States *states)
{
	KIMLUINT offset = 0;

	++states->codepointer;
	offset = kimlDeserializeUInt(states->code + states->codepointer);
	states->codepointer += sizeof(KIMLUINT);

	if (offset < 1 || offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->kstack[states->kstack.size() - offset] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_srd1(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 1;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(*(&states->kstack.back() - (offset - 1)));
}

void Engine::do_srd2(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 2;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(*(&states->kstack.back() - (offset - 1)));
}

void Engine::do_srd3(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 3;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(*(&states->kstack.back() - (offset - 1)));
}

void Engine::do_srd4(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 4;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(*(&states->kstack.back() - (offset - 1)));
}

void Engine::do_srd5(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 5;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(*(&states->kstack.back() - (offset - 1)));
}

void Engine::do_swr1(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 1;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->kstack[states->kstack.size() - offset] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_swr2(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 2;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->kstack[states->kstack.size() - offset] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_swr3(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 3;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->kstack[states->kstack.size() - offset] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_swr4(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 4;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->kstack[states->kstack.size() - offset] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_swr5(States *states)
{
	++states->codepointer;

	const KIMLUINT offset = 5;
	if (offset > states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->kstack[states->kstack.size() - offset] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_trd0(States *states)
{
	++states->codepointer;
	states->estack.push_back(states->tape[0]);
}

void Engine::do_twr0(States *states)
{
	++states->codepointer;

	states->tape[0] = states->estack.back();
	states->estack.pop_back();
}

void Engine::do_trs0(States *states)
{
	++states->codepointer;
	states->kstack.push_back(states->tape[0]);
}

void Engine::do_ldnull(States *states)
{
	++states->codepointer;
	states->estack.push_back(Object((KIMLOBJECT)nullptr));
}

void Engine::do_cvo(States *states)
{
	++states->codepointer;

	Object obj(states->estack.back().getObject());
	states->estack.push_back(obj);
}

void Engine::do_illegal(States *states)
{
	states->err = KRE_ILLEGAL_OPCODE;
	states->isRunning = false;
}

void Engine::f_abs(States *states)
{
	Object obj(states->estack.back());
	states->estack.pop_back();

	if (obj.getDataType() == KIML_INT)
		states->estack.push_back(Object(abs(obj.getInt())));
	else
		states->estack.push_back(Object((KIMLFLOAT)abs(obj.getFloat())));
}

void Engine::f_sqrt(States *states)
{
	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLFLOAT)sqrt(obj.getFloat())));
}

void Engine::f_sin(States *states)
{
	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLFLOAT)sin(obj.getFloat())));
}

void Engine::f_cos(States *states)
{
	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLFLOAT)cos(obj.getFloat())));
}

void Engine::f_tan(States *states)
{
	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLFLOAT)tan(obj.getFloat())));
}

void Engine::f_asin(States *states)
{
	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLFLOAT)asin(obj.getFloat())));
}

void Engine::f_acos(States *states)
{
	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLFLOAT)acos(obj.getFloat())));
}

void Engine::f_atan(States *states)
{
	Object obj(states->estack.back().getFloat());
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLFLOAT)atan(obj.getFloat())));
}

void Engine::f_chr(States *states)
{
	KIMLINT c = states->estack.back().getInt();
	states->estack.pop_back();

	char ac[1];
	ac[0] = (char)(c % 256);

	states->estack.push_back(Object(ac));
}

void Engine::f_asc(States *states)
{
	KIMLCSTRING s = states->estack.back().getString();
	states->estack.pop_back();

	if (s[0] == 0)
		states->estack.push_back(Object((KIMLINT)0));
	else
		states->estack.push_back(Object((KIMLINT)s[0]));

	delete []s;
}

void Engine::f_len(States *states)
{
	KIMLCSTRING s = states->estack.back().getString();
	states->estack.pop_back();

	states->estack.push_back(Object((KIMLINT)strlen(s)));
	delete []s;
}

void Engine::f_left(States *states)
{
	KIMLINT length;

	length = states->estack.back().getInt();
	states->estack.pop_back();

	KIMLCSTRING s = states->estack.back().getString();
	states->estack.pop_back();

	if (length >= (KIMLINT)strlen(s))
	{
		states->estack.push_back(Object(s));
	}
	else
	{
		KIMLSTRING substr = new KIMLCHAR[length + 1];
		strncpy(substr, s, length);
		substr[length] = 0;
		states->estack.push_back(Object(substr));
		delete []substr;
	}

	delete []s;
}

void Engine::f_mid(States *states)
{
	KIMLINT startIndex, length;

	length = states->estack.back().getInt();
	states->estack.pop_back();

	startIndex = states->estack.back().getInt();
	states->estack.pop_back();

	KIMLCSTRING s = states->estack.back().getString();
	states->estack.pop_back();

	KIMLINT slen = strlen(s);
	if (startIndex >= slen || length >= slen)
	{
		states->estack.push_back(Object(s));
	}
	else if (startIndex + length >= slen)
	{
		states->estack.push_back(Object(s));
	}
	else
	{
		KIMLSTRING substr = new KIMLCHAR[length + 1];
		strncpy(substr, s + startIndex, length);
		substr[length] = 0;
		states->estack.push_back(Object(substr));
		delete []substr;
	}

	delete []s;
}

void Engine::f_right(States *states)
{
	KIMLINT length;

	length = states->estack.back().getInt();
	states->estack.pop_back();

	KIMLCSTRING s = states->estack.back().getString();
	states->estack.pop_back();

	KIMLINT slen = strlen(s);
	if (length >= slen)
	{
		states->estack.push_back(Object(s));
	}
	else
	{
		KIMLSTRING substr = new KIMLCHAR[length + 1];
		strncpy(substr, s + (slen - length), length);
		substr[length] = 0;
		states->estack.push_back(Object(substr));
		delete []substr;
	}

	delete []s;
}

void Engine::f_iif(States *states)
{
	Object cond, tv, fv;

	fv = states->estack.back();
	states->estack.pop_back();

	tv = states->estack.back();
	states->estack.pop_back();

	cond = states->estack.back();
	states->estack.pop_back();

	if (cond.getBool())
		states->estack.push_back(tv);
	else
		states->estack.push_back(fv);
}

void Engine::f_tape(States *states)
{
	KIMLINT idx = states->estack.back().getInt();
	states->estack.pop_back();

	if (idx < 0 || idx >= TAPESIZE)
	{
		states->err = KRE_TAPE_INDEX_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(states->tape[idx]);
}

void Engine::f_stack(States *states)
{
	KIMLINT diff = states->estack.back().getInt();
	states->estack.pop_back();

	if (diff <= 0 || diff > (KIMLINT)states->kstack.size())
	{
		states->err = KRE_STACK_PTR_OOR;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(*( &states->kstack.back() - (diff - 1) ));
}

void Engine::f_pop(States *states)
{
	if (states->kstack.empty())
	{
		states->err = KRE_STACK_EMPTY;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(states->kstack.back());
	states->kstack.pop_back();
}

void Engine::f_peek(States *states)
{
	if (states->kstack.empty())
	{
		states->err = KRE_STACK_EMPTY;
		states->isRunning = false;
		return;
	}

	states->estack.push_back(states->kstack.back());
}

void Engine::f_min(States *states)
{
	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) > KIML_REAL)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	if ((a > b).getBool())
		states->estack.push_back(b);
	else
		states->estack.push_back(a);
}

void Engine::f_max(States *states)
{
	Object a, b;

	b = states->estack.back(); states->estack.pop_back();
	a = states->estack.back(); states->estack.pop_back();

	if (HigherType(a.getDataType(), b.getDataType()) > KIML_REAL)
	{
		states->err = KRE_TYPE_MISMATCHED;
		states->isRunning = false;
		return;
	}

	if ((a > b).getBool())
		states->estack.push_back(a);
	else
		states->estack.push_back(b);
}

void Engine::f_random(States *states)
{
	KIMLFLOAT v = (KIMLFLOAT)rand() / (KIMLFLOAT)RAND_MAX;
	states->estack.push_back(Object(v));
}

void Engine::f_print(States *states)
{
	Object obj;

	obj = states->estack.back(); states->estack.pop_back();

	const char *str = obj.getString();
	printf("%s", str);

	delete[]str;

	states->estack.push_back(Object(0));
}

void Engine::f_readln(States *states)
{
	std::string s;
	std::getline(std::cin, s);

	states->estack.push_back(Object(s.c_str()));
}
