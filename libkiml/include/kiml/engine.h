#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "kiml.h"

class States;

typedef void (*OPEXECUTOR)(States *states);
typedef void (*FUNCEXECUTOR)(States *states);

class Engine
{
private:
	States *states;

	static OPEXECUTOR Executors[256];
	static OPEXECUTOR DefaultExecutors[];
	static FUNCEXECUTOR FuncExecutors[];

private:
	static void do_nop(States *states);
	static void do_ldi(States *states);
	static void do_ldr(States *states);
	static void do_lds(States *states);
	static void do_ldes(States *states);
	static void do_ldvar(States *states);
	static void do_stvar(States *states);
	static void do_new(States *states);
	static void do_free(States *states);
	static void do_cvi(States *states);
	static void do_cvr(States *states);
	static void do_cvs(States *states);
	static void do_cvin(States *states);
	static void do_cvrn(States *states);
	static void do_in(States *states);
	static void do_out(States *states);
	static void do_push(States *states);
	static void do_pop(States *states);
	static void do_stf(States *states);
	static void do_smv(States *states);
	static void do_swap(States *states);
	static void do_sclr(States *states);
	static void do_sclrc(States *states);
	static void do_trd(States *states);
	static void do_trdx(States *states);
	static void do_twr(States *states);
	static void do_twrx(States *states);
	static void do_tnxt(States *states);
	static void do_tprv(States *states);
	static void do_tmov(States *states);
	static void do_jmp(States *states);
	static void do_cjmp(States *states);
	static void do_call(States *states);
	static void do_ccall(States *states);
	static void do_ret(States *states);
	static void do_hlt(States *states);
	static void do_add(States *states);
	static void do_sub(States *states);
	static void do_mul(States *states);
	static void do_div(States *states);
	static void do_idiv(States *states);
	static void do_mod(States *states);
	static void do_pow(States *states);
	static void do_equ(States *states);
	static void do_neq(States *states);
	static void do_lt(States *states);
	static void do_lte(States *states);
	static void do_gt(States *states);
	static void do_gte(States *states);
	static void do_cat(States *states);
	static void do_and(States *states);
	static void do_or(States *states);
	static void do_xor(States *states);
	static void do_not(States *states);
	static void do_neg(States *states);
	static void do_func(States *states);
	static void do_trace(States *states);
	
	static void do_ldtrue(States *states);
	static void do_ldfalse(States *states);
	static void do_cvb(States *states);
	static void do_fdecl(States *states);
	static void do_fcall(States *states);
	static void do_ehnd(States *states);
	static void do_eunhnd(States *states);
	static void do_eraise(States *states);

	static void do_srd(States *states);
	static void do_swr(States *states);
	static void do_srd1(States *states);
	static void do_srd2(States *states);
	static void do_srd3(States *states);
	static void do_srd4(States *states);
	static void do_srd5(States *states);
	static void do_swr1(States *states);
	static void do_swr2(States *states);
	static void do_swr3(States *states);
	static void do_swr4(States *states);
	static void do_swr5(States *states);
	static void do_trd0(States *states);
	static void do_twr0(States *states);
	static void do_trs0(States *states);
	static void do_ldnull(States *states);
	static void do_cvo(States *states);

	static void do_illegal(States *states);

	static void f_abs(States *states);
	static void f_sqrt(States *states);
	static void f_sin(States *states);
	static void f_cos(States *states);
	static void f_tan(States *states);
	static void f_asin(States *states);
	static void f_acos(States *states);
	static void f_atan(States *states);
	static void f_chr(States *states);
	static void f_asc(States *states);
	static void f_len(States *states);
	static void f_left(States *states);
	static void f_mid(States *states);
	static void f_right(States *states);
	static void f_iif(States *states);
	static void f_tape(States *states);
	static void f_stack(States *states);
	static void f_pop(States *states);
	static void f_peek(States *states);
	static void f_min(States *states);
	static void f_max(States *states);
	static void f_random(States *states);
	static void f_print(States *states);
	static void f_readln(States *states);

public:
	static void PrepareExecutors(void);

	void Init(States *states);
	bool Run(void);
	bool InvokeFunction(KIMLUINT nameHash);
	void RaiseEvent(KIMLUINT nameHash);
};

#endif//__ENGINE_H__
