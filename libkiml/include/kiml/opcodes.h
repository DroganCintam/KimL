#ifndef __OPCODES_H__
#define __OPCODES_H__

enum OpCodes
{
	op_nop    = 0x00,    // no operation
	op_ldi    = 0x01,    // load int
	op_ldr    = 0x02,    // load real
	op_lds    = 0x03,    // load string
	op_ldes   = 0x04,    // load empty string
	op_ldvar  = 0x05,    // load variable
	op_stvar  = 0x06,    // store variable
	op_new    = 0x07,    // new variable
	op_free   = 0x08,    // delete object variable
	op_cvi    = 0x09,    // convert to int
	op_cvr    = 0x0A,    // convert to real
	op_cvs    = 0x0B,    // convert to string
	op_cvin   = 0x0C,    // convert to int, Nothrow
	op_cvrn   = 0x0D,    // convert to real, Nothrow
	op_in     = 0x0E,    // standard input
	op_out    = 0x0F,    // standard output
	op_push   = 0x10,    // push to k-stack
	op_pop    = 0x11,    // pop from k-stack
	op_stf    = 0x12,    // stack transfer (k-stack to eval-stack)
	op_smv    = 0x13,    // stack move (k-stack to eval-stack)
	op_swap   = 0x14,    // stack swap (k-stack)
	op_sclr   = 0x15,    // stack clear (k-stack)
	op_sclrc  = 0x16,    // stack clear, counted (k-stack)
	op_trd    = 0x17,    // tape read
	op_trdx   = 0x18,    // tape read at indeX
	op_twr    = 0x19,    // tape write
	op_twrx   = 0x1A,    // tape write at indeX
	op_tnxt   = 0x1B,    // tape next
	op_tprv   = 0x1C,    // tape previous
	op_tmov   = 0x1D,    // tape move
	op_jmp    = 0x1E,    // jump
	op_cjmp   = 0x1F,    // conditional jump
	op_call   = 0x20,    // call
	op_ccall  = 0x21,    // conditional call
	op_ret    = 0x22,    // return
	op_hlt    = 0x23,    // halt (terminate the program)

	op_add    = 0x24,    // addition
	op_sub    = 0x25,    // subtraction
	op_mul    = 0x26,    // multiplication
	op_div    = 0x27,    // division
	op_idiv   = 0x28,    // integral division
	op_mod    = 0x29,    // modulus
	op_pow    = 0x2A,    // exponentiation
	op_equ    = 0x2B,    // equal
	op_neq    = 0x2C,    // not equal
	op_lt     = 0x2D,    // less than
	op_lte    = 0x2E,    // less than or equal
	op_gt     = 0x2F,    // greater than
	op_gte    = 0x30,    // greater than or equal
	op_cat    = 0x31,    // concatenate (string)
	op_and    = 0x32,    // logical and
	op_or     = 0x33,    // logical or
	op_xor    = 0x34,    // logical exclusive or
	op_not    = 0x35,    // logical not
	op_neg    = 0x36,    // negation
	op_func   = 0x37,    // built-in function

	op_trace  = 0x38,    // for debug mode

	// added in 13.08
	op_ldtrue = 0x39,    // load constant TRUE
	op_ldfalse= 0x3A,    // load constant FALSE
	op_cvb    = 0x3B,    // convert to bool
	op_fdecl  = 0x3C,    // decl a function
	op_fcall  = 0x3D,    // call a function
	op_ehnd   = 0x3E,    // handle an event
	op_eunhnd = 0x3F,    // unhandle an event
	op_eraise = 0x40,    // raise an event

	// added in 14.11
	op_srd    = 0x41,    // stack read
	op_swr    = 0x42,    // stack write
	op_srd1   = 0x43,    // stack read offset 1
	op_srd2   = 0x44,    // stack read offset 2
	op_srd3   = 0x45,    // stack read offset 3
	op_srd4   = 0x46,    // stack read offset 4
	op_srd5   = 0x47,    // stack read offset 5
	op_swr1   = 0x48,    // stack write offset 1
	op_swr2   = 0x49,    // stack write offset 2
	op_swr3   = 0x4A,    // stack write offset 3
	op_swr4   = 0x4B,    // stack write offset 4
	op_swr5   = 0x4C,    // stack write offset 5
	op_trd0   = 0x4D,    // tape read index 0
	op_twr0   = 0x4E,    // tape write index 0
	op_trs0   = 0x4F,    // tape read index 0 to stack
	op_ldnull = 0x50,    // load null
	op_cvo    = 0x51,    // convert to object

	op_x      = 0xFE,    // RESERVED
	op_z      = 0xFF,    // RESERVED
	op_TOTAL  = op_cvo + 1
};

#endif//__OPCODES_H__
