// RBT-16 - Fantasy Retro-Computer Inspired by the Amiga 500 and Atari ST.
//
// Copyright (c) 2026 acubeone
// Email: acube_one@disroot.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include "cpu/cpu_internal.h"
#include "cpu/effective_address.h"
#include "error.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"
#include "rbt/helpers.h"

#include <assert.h>
#include <limits.h>
#include <stdint.h>

typedef RBT_ErrorCode (*RBT_OpExec)(const RBT_Instruction *instr, RBT_Cpu *cpu);

static inline void _ccr_set_nz(RBT_Cpu *cpu, RBT_OperandSize size, u32 result) {
	u32 msb = ((u32)size * 8) - 1; // Most-Significant Bit
	cpu->state.sr.negative = (result >> msb) & 1;
	cpu->state.sr.zero = rbt_truncate(size, result) == 0;
}

static inline void _ccr_set_nzvc(
	RBT_Cpu *cpu, RBT_OperandSize size, u64 src, u64 dst, u64 x, bool is_sub
) {
	u32 mask = rbt_truncate(size, 0xffff'ffff);
	u64 wide;
	if (is_sub)
		wide = (dst & mask) + (~src & mask) + (1 - x);
	else
		wide = (dst & mask) + (src & mask) + x;

	u32 msb = ((u32)size * 8) - 1; // Most-Significant Bit
	u32 result = (u32)wide & mask;

	cpu->state.sr.negative = (result >> msb) & 1;
	cpu->state.sr.zero = result == 0;

	bool carry = (wide >> (msb + 1)) & 1;
	if (is_sub) {
		cpu->state.sr.carry = 1 - carry; // borrow
		cpu->state.sr.overflow = ((dst ^ src) & (dst ^ result) & (1 << msb)) != 0;
	} else {
		cpu->state.sr.carry = carry;
		cpu->state.sr.overflow = ((dst ^ result) & (src ^ result) & (1 << msb)) != 0;
	}
}

static RBT_ErrorCode _op_abcd(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// ADD - Add binary
// Dn + [src] -> Dn
// Syntax:
//   ADD <ea>, Dn
//   ADD Dn, <ea>
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ * * * * * ]
static RBT_ErrorCode _op_add(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 src;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &src);
	if (err)
		return err;

	u32 dst;
	err = _ea_read(&instr->dst, instr->size, cpu, &dst);
	if (err)
		return err;

	u32 result = dst + src;
	err = _ea_write(&instr->dst, instr->size, cpu, result);
	if (err)
		return err;

	_ccr_set_nzvc(cpu, instr->size, src, dst, 0, false);
	cpu->state.sr.extend = cpu->state.sr.carry;
	return RBT_ERR_SUCCESS;
}

// ADDA - Add address
// An + [src] -> An
// Syntax:
//   ADDA <ea>, An
// SIZE = (Word, Long)
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_adda(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 ea_src;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &ea_src);
	if (err)
		return err;

	i32 src = rbt_sign_extend(instr->size, ea_src);

	cpu->state.gpr.addr[instr->dst.reg] += (u32)src;
	return RBT_ERR_SUCCESS;
}

// ADDI - Add immediate
// [dst] + #imm -> [dst]
// Syntax:
//   ADDI #imm, <ea>
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ * * * * * ]
static RBT_ErrorCode _op_addi(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	return _op_add(instr, cpu);
}

// ADDQ - Add quick
// [dst] + #imm -> [dst]
// Syntax:
//   ADDQ #imm, <ea>
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ * * * * * ]
//
// note: CCR is not updated if destination is address register
static RBT_ErrorCode _op_addq(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 src;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &src);
	if (err)
		return err;

	u32 dst;
	err = _ea_read(&instr->dst, instr->size, cpu, &dst);
	if (err)
		return err;

	u32 result = dst + src;
	err = _ea_write(&instr->dst, instr->size, cpu, result);
	if (err)
		return err;

	if (instr->dst.mode != RBT_EA_DIRECT_ADDR) {
		_ccr_set_nzvc(cpu, instr->size, src, dst, 0, false);
		cpu->state.sr.extend = cpu->state.sr.carry;
	}
	return RBT_ERR_SUCCESS;
}

// ADDX - Add with extend
// [dst] + [src] + [X] -> [dst]
// Syntax:
//   ADDX Dy, Dx
//   ADDX -(Ay), -(Ax)
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ * * * * * ]
//
// Z - Cleared if result is non-zero, unchanged otherwise
static RBT_ErrorCode _op_addx(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 src;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &src);
	if (err)
		return err;

	u32 dst;
	err = _ea_read(&instr->dst, instr->size, cpu, &dst);
	if (err)
		return err;

	u32 extend = cpu->state.sr.extend ? 1 : 0;
	u32 result = dst + src + extend;
	err = _ea_write(&instr->dst, instr->size, cpu, result);
	if (err)
		return err;

	bool prev_zero = cpu->state.sr.zero;
	_ccr_set_nzvc(cpu, instr->size, src, dst, extend, false);
	if (rbt_truncate(instr->size, result) == 0)
		cpu->state.sr.zero = prev_zero;
	cpu->state.sr.extend = cpu->state.sr.carry;
	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_and(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_andi(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_asl(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_asr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_bcc(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_bchg(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_bclr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_bra(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_bset(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_bsr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_btst(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_chk(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// CLR - Clear operand
// #0 -> [dst]
// Syntax:
//   CLR <ea>
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ . 0 1 0 0 ]
static RBT_ErrorCode _op_clr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	RBT_ErrorCode err = _ea_write(&instr->dst, instr->size, cpu, 0);
	if (err)
		return err;

	cpu->state.sr.negative = 0;
	cpu->state.sr.zero = 1;
	cpu->state.sr.overflow = 0;
	cpu->state.sr.carry = 0;
	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_cmp(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_cmpa(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_cmpi(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_cmpm(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_dbcc(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// DIVS - Signed division
// Dn / [src] -> Dn, IF SRC=0 THEN TRAP
// Syntax:
//   DIVS <ea>, Dn
// SIZE = Word
//
//   X N Z V C
// [ . * * * 0 ]
//
// N - Set if the quotient is negative, cleared otherwise. Undefined if overflow
// or division by zero
// Z - Set if the quotient is zero, cleared otherwise. Undefined if overflow
// or division by zero
// V - Set if overflow occurs, cleared otherwise. Undefined if divide by zero
static RBT_ErrorCode _op_divs(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 src_word;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &src_word);
	if (err)
		return err;

	i32 diviend = (i32)cpu->state.gpr.data[instr->dst.reg];
	i32 divisor = rbt_sign_extend(RBT_SIZE_WORD, src_word);

	cpu->state.sr.carry = false;

	if (divisor == 0)
		return _cpu_raise_exception(cpu, _VEC_ZERO_DIV);

	// Undefined Behaviour: INT32_MIN / -1
	// In real hardware, it causes overflow
	if (diviend == INT32_MIN && divisor == -1) {
		cpu->state.sr.overflow = true;
		return RBT_ERR_SUCCESS;
	}

	i32 quotient = diviend / divisor;
	i32 remainder = diviend % divisor;

	if (quotient > INT16_MAX || quotient < INT16_MIN) {
		cpu->state.sr.overflow = true;
		return RBT_ERR_SUCCESS;
	}

	u32 result = ((u32)(i16)remainder << 16) | ((u32)(i16)quotient & 0xffff);
	cpu->state.gpr.data[instr->dst.reg] = result;

	_ccr_set_nz(cpu, instr->size, quotient);
	cpu->state.sr.overflow = false;
	return RBT_ERR_SUCCESS;
}

// DIVU - Unsigned division
// Dn / [src] -> Dn, IF SRC=0 THEN TRAP
// Syntax:
//   DIVU <ea>, Dn
// SIZE = Word
//
//   X N Z V C
// [ . * * * 0 ]
//
// N - *Set if the quotient is negative, cleared otherwise. Undefined if overflow
// or division by zero
// Z - Set if the quotient is zero, cleared otherwise. Undefined if overflow
// or division by zero
// V - Set if overflow occurs, cleared otherwise. Undefined if divide by zero
//
// NOTE: Programmer's Manual says that the negative flag is set if quotient is
// negative, which should never happen. But I will follow the Musashi and Moira
// implementation, setting the flag accordingly with the PRM
static RBT_ErrorCode _op_divu(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 src_word;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &src_word);
	if (err)
		return err;

	u32 diviend = cpu->state.gpr.data[instr->dst.reg];
	u32 divisor = src_word;

	cpu->state.sr.carry = false;

	if (divisor == 0)
		return _cpu_raise_exception(cpu, _VEC_ZERO_DIV);

	u32 quotient = diviend / divisor;
	u32 remainder = diviend % divisor;

	if (quotient > UINT16_MAX) {
		cpu->state.sr.overflow = true;
		return RBT_ERR_SUCCESS;
	}

	u32 result = (remainder << 16) | (quotient & 0xffff);
	cpu->state.gpr.data[instr->dst.reg] = result;

	_ccr_set_nz(cpu, instr->size, quotient);
	cpu->state.sr.overflow = false;
	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_eor(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_eori(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// EXG - Exchange Registers
// [Ry] <-> [Rx]
// Syntax:
//   EXG Rx, Ry
// SIZE = Long
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_exg(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 data_y; // always ea_src
	u32 data_x; // always ea_dst

	// ignore err-return, since EA is always a register
	_ea_read(&instr->src, instr->size, cpu, &data_y);
	_ea_read(&instr->dst, instr->size, cpu, &data_x);

	_ea_write(&instr->src, instr->size, cpu, data_x);
	_ea_write(&instr->dst, instr->size, cpu, data_y);
	return RBT_ERR_SUCCESS;
}

// EXT - Sign extend data register
// Dn(sign-extended) -> Dn
// Syntax:
//   EXT Dn
// SIZE = (Word, Long)
//
//   X N Z V C
// [ . * * 0 0 ]
static RBT_ErrorCode _op_ext(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 dst = cpu->state.gpr.data[instr->dst.reg];
	RBT_OperandSize from = instr->size == RBT_SIZE_LONG ? RBT_SIZE_WORD : RBT_SIZE_BYTE;
	dst = rbt_sign_extend(from, dst);

	cpu->state.gpr.data[instr->dst.reg] = dst;

	_ccr_set_nz(cpu, instr->size, dst);
	cpu->state.sr.overflow = false;
	cpu->state.sr.carry = false;
	return RBT_ERR_SUCCESS;
}

// ILLEGAL - Force trigger Illegal exception vector
// Raise Exception: ILLEGAL - 4
// Syntax:
//   ILLEGAL
// SIZE = None
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_illegal(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;

	return _cpu_raise_exception(cpu, _VEC_ILLEGAL);
}

static RBT_ErrorCode _op_jmp(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_jsr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// LEA - Load effective address
// [src] -> An
// Syntax:
//   LEA <ea>, An
// SIZE = Long
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_lea(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	cpu->state.gpr.addr[instr->dst.reg] = _ea_compute_address(&instr->src, cpu);
	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_link(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_lsl(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_lsr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// MOVE - Copy data from source to destination
// [src] -> [dst]
// Syntax:
//   MOVE <ea>, <ea>
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ . * * 0 0 ]
//
//
// MOVE from CCR - Copy data from CCR to destination [M68010+]
// CCR -> [dst]
// Syntax:
//   MOVE CCR, <ea>
// SIZE = Word
//
//   X N Z V C
// [ . . . . . ]
//
//
// MOVE to CCR - Copy data from source to CCR
// [src] -> CCR
// Syntax:
//   MOVE <ea>, CCR
// SIZE = Word
//
//   X N Z V C
// [ * * * * * ]
//
// X - Set if bit 4 in data is set, cleared otherwise
// N - Set if bit 3 in data is set, cleared otherwise
// Z - Set if bit 2 in data is set, cleared otherwise
// V - Set if bit 1 in data is set, cleared otherwise
// C - Set if bit 0 in data is set, cleared otherwise
//
//
// MOVE from SR - Copy data from SR to destination [privilege(MC68010+)]
// SR -> [dst]
// Syntax:
//   MOVE SR, <ea>
// SIZE = Word
//
//   X N Z V C
// [ . . . . . ]
//
//
// MOVE to SR - Copy data from source to SR [privilege]
// [src] -> SR
// Syntax:
//   MOVE <ea>, SR
// SIZE = Word
//
//   X N Z V C
// [ * * * * * ]
//
// X - Set if bit 4 in data is set, cleared otherwise
// N - Set if bit 3 in data is set, cleared otherwise
// Z - Set if bit 2 in data is set, cleared otherwise
// V - Set if bit 1 in data is set, cleared otherwise
// C - Set if bit 0 in data is set, cleared otherwise
//
//
// MOVE USP - Copy data to or from USP [privilege]
// MOVE to USP
//   IF S=1 THEN [An] -> USP ELSE TRAP
// MOVE from USP
//   IF S=1 THEN USP -> [An] ELSE TRAP
// Syntax:
//   MOVE An, USP
//   MOVE USP, An
// SIZE = Long
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_move(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	if (!cpu->state.sr.supervisor) {
		if (instr->src.mode == RBT_EA_REGISTER_USP
			|| instr->dst.mode == RBT_EA_REGISTER_USP)
			return _cpu_raise_exception(cpu, _VEC_PRIVILEGE);

		if (instr->dst.mode == RBT_EA_REGISTER_SR)
			return _cpu_raise_exception(cpu, _VEC_PRIVILEGE);

		if (instr->src.mode == RBT_EA_REGISTER_SR && cpu->cfg.model == RBT_CPU_M68010)
			return _cpu_raise_exception(cpu, _VEC_PRIVILEGE);
	}

	if (instr->src.mode == RBT_EA_REGISTER_CCR && cpu->cfg.model != RBT_CPU_M68010)
		return _cpu_raise_exception(cpu, _VEC_ILLEGAL);

	u32 data;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &data);
	if (err)
		return err;

	err = _ea_write(&instr->dst, instr->size, cpu, data);
	if (err)
		return err;

	u16 reg_modes = RBT_EA_REGISTER_CCR | RBT_EA_REGISTER_SR | RBT_EA_REGISTER_USP;
	if (!(instr->src.mode & reg_modes) && !(instr->dst.mode & reg_modes)) {
		_ccr_set_nz(cpu, instr->size, data);
		cpu->state.sr.overflow = false;
		cpu->state.sr.carry = false;
	}

	return RBT_ERR_SUCCESS;
}

// MOVEA - Copy data to address register
// [src] -> An
// Syntax:
//   MOVEA <ea>, An
// SIZE = (Word, Long)
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_movea(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 data;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &data);
	if (err)
		return err;

	if (instr->size == RBT_SIZE_WORD)
		data = rbt_sign_extend(RBT_SIZE_WORD, data);

	cpu->state.gpr.addr[instr->dst.reg] = data;
	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_movem(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_movep(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// MOVEQ - Move signed 8-bit data quick
// [src] -> Dn
// Syntax:
//   MOVEQ #data:8, Dn
// SIZE = Long
//
//   X N Z V C
// [ . * * 0 0 ]
static RBT_ErrorCode _op_moveq(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 data = rbt_sign_extend(RBT_SIZE_BYTE, instr->src.imm);
	cpu->state.gpr.data[instr->dst.reg] = data;

	_ccr_set_nz(cpu, instr->size, data);
	cpu->state.sr.overflow = false;
	cpu->state.sr.carry = false;
	return RBT_ERR_SUCCESS;
}

// MULS - Signed multiply
// Dn * [src] -> Dn
// Syntax:
//   MULS <ea>, Dn
// SIZE = Word
//
//   X N Z V C
// [ . * * 0 0 ]
static RBT_ErrorCode _op_muls(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 src_word;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &src_word);
	if (err)
		return err;

	i32 src = rbt_sign_extend(RBT_SIZE_WORD, src_word & 0xffff);
	i32 dst = (i16)(cpu->state.gpr.data[instr->dst.reg] & 0xffff);

	i32 result = src * dst;
	cpu->state.gpr.data[instr->dst.reg] = result;

	_ccr_set_nz(cpu, RBT_SIZE_LONG, result);
	cpu->state.sr.overflow = false;
	cpu->state.sr.carry = false;
	return RBT_ERR_SUCCESS;
}

// MULU - Unsigned multiply
// Dn * [src] -> Dn
// Syntax:
//   MULU <ea>, Dn
// SIZE = Word
//
//   X N Z V C
// [ . * * 0 0 ]
static RBT_ErrorCode _op_mulu(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 src;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &src);
	if (err)
		return err;

	u32 dst = cpu->state.gpr.data[instr->dst.reg];

	u32 result = dst * src;
	cpu->state.gpr.data[instr->dst.reg] = result;

	_ccr_set_nz(cpu, RBT_SIZE_LONG, result);
	cpu->state.sr.overflow = false;
	cpu->state.sr.carry = false;
	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_nbcd(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// NEG - Negate
// 0 - [dst] -> [dst]
// Syntax:
//   NEG <ea>
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ * * * * * ]
static RBT_ErrorCode _op_neg(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 data;
	RBT_ErrorCode err = _ea_read(&instr->dst, instr->size, cpu, &data);
	if (err)
		return err;

	u32 result = rbt_truncate(instr->size, ~data + 1);
	err = _ea_write(&instr->dst, instr->size, cpu, result);
	if (err)
		return err;

	_ccr_set_nzvc(cpu, instr->size, data, 0, 0, true);
	cpu->state.sr.extend = cpu->state.sr.carry;
	return RBT_ERR_SUCCESS;
}

// NEGX - Negate with extend
// 0 - [dst] - [X] -> [dst]
// Syntax:
//   NEGX <ea>
// SIZE = (Byte, Word, Long)
//
//   X N Z V C
// [ * * * * * ]
//
// Z - Cleared if result is non-zero, unchanged otherwise
static RBT_ErrorCode _op_negx(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 data;
	RBT_ErrorCode err = _ea_read(&instr->dst, instr->size, cpu, &data);
	if (err)
		return err;

	u32 extend = cpu->state.sr.extend ? 1 : 0;
	u32 result = rbt_truncate(instr->size, ~data + (1 - extend));
	err = _ea_write(&instr->dst, instr->size, cpu, result);
	if (err)
		return err;

	bool prev_zero = cpu->state.sr.zero;
	_ccr_set_nzvc(cpu, instr->size, data, 0, extend, true);
	if (rbt_truncate(instr->size, result) == 0)
		cpu->state.sr.zero = prev_zero;
	cpu->state.sr.extend = cpu->state.sr.carry;
	return RBT_ERR_SUCCESS;
}

// NOP - No Operation
// Consume CPU cycles and does nothing
// Syntax:
//  NOP
//  SIZE = Word
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_nop(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_not(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_or(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_ori(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_pea(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_reset(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_rol(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_ror(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_roxl(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_roxr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_rte(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_rtr(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_rts(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_sbcd(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_scc(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_stop(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_sub(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_suba(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_subi(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_subq(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_subx(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// SWAP - Swap register upper and lower words
// Dn[15:0] <-> Dn[31:16]
// Syntax:
//   SWAP Dn
// SIZE = Word
//
//   X N Z V C
// [ . * * 0 0 ]
static RBT_ErrorCode _op_swap(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	u32 *data = &cpu->state.gpr.data[instr->dst.reg];

	u16 low = *data & 0xffff;
	u16 hiw = (*data >> 16) & 0xffff;
	*data = (low << 16) | hiw;

	_ccr_set_nz(cpu, RBT_SIZE_LONG, *data);
	cpu->state.sr.overflow = false;
	cpu->state.sr.carry = false;
	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_tas(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_trap(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_trapv(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

static RBT_ErrorCode _op_tst(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_unlk(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	(void)cpu;

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// M68010+
static RBT_ErrorCode _op_bkpt(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	if (cpu->cfg.model != RBT_CPU_M68010)
		return _cpu_raise_exception(cpu, _VEC_ILLEGAL);

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// MOVEC - Move Control Register
// IF S=1 THEN [Rc] -> [Rn] or [Rn] -> [Rc] ELSE TRAP
// Syntax:
//   MOVEC Rc, Rn
//   MOVEC Rn, Rc
// SIZE = Long
//
//   X N Z V C
// [ . . . . . ]
static RBT_ErrorCode _op_movec(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	if (cpu->cfg.model != RBT_CPU_M68010)
		return _cpu_raise_exception(cpu, _VEC_ILLEGAL);

	u32 data;
	RBT_ErrorCode err = _ea_read(&instr->src, instr->size, cpu, &data);
	if (err)
		return err;

	err = _ea_write(&instr->dst, instr->size, cpu, data);
	if (err)
		return err;

	return RBT_ERR_SUCCESS;
}

static RBT_ErrorCode _op_moves(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	if (cpu->cfg.model != RBT_CPU_M68010)
		return _cpu_raise_exception(cpu, _VEC_ILLEGAL);

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}
static RBT_ErrorCode _op_rtd(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	(void)instr;
	if (cpu->cfg.model != RBT_CPU_M68010)
		return _cpu_raise_exception(cpu, _VEC_ILLEGAL);

	_push_fatal(RBT_ERR_UNIMPLEMENTED, "Unimplemented instruction");
	return RBT_ERR_UNIMPLEMENTED;
}

// clang-format off
static const RBT_OpExec _op_dispatch_table[_RBT_OP_COUNT] = {
	[RBT_OP_ABCD] = _op_abcd,       [RBT_OP_ADD] = _op_add,
	[RBT_OP_ADDA] = _op_adda,       [RBT_OP_ADDI] = _op_addi,
	[RBT_OP_ADDQ] = _op_addq,       [RBT_OP_ADDX] = _op_addx,
	[RBT_OP_AND] = _op_and,         [RBT_OP_ANDI] = _op_andi,
	[RBT_OP_ASL] = _op_asl,         [RBT_OP_ASR] = _op_asr,
	[RBT_OP_Bcc] = _op_bcc,         [RBT_OP_BCHG] = _op_bchg,
	[RBT_OP_BCLR] = _op_bclr,       [RBT_OP_BRA] = _op_bra,
	[RBT_OP_BSET] = _op_bset,       [RBT_OP_BSR] = _op_bsr,
	[RBT_OP_BTST] = _op_btst,       [RBT_OP_CHK] = _op_chk,
	[RBT_OP_CLR] = _op_clr,         [RBT_OP_CMP] = _op_cmp,
	[RBT_OP_CMPA] = _op_cmpa,       [RBT_OP_CMPI] = _op_cmpi,
	[RBT_OP_CMPM] = _op_cmpm,       [RBT_OP_DBcc] = _op_dbcc,
	[RBT_OP_DIVS] = _op_divs,       [RBT_OP_DIVU] = _op_divu,
	[RBT_OP_EOR] = _op_eor,         [RBT_OP_EORI] = _op_eori,
	[RBT_OP_EXG] = _op_exg,         [RBT_OP_EXT] = _op_ext,
	[RBT_OP_ILLEGAL] = _op_illegal, [RBT_OP_JMP] = _op_jmp,
	[RBT_OP_JSR] = _op_jsr,         [RBT_OP_LEA] = _op_lea,
	[RBT_OP_LINK] = _op_link,       [RBT_OP_LSL] = _op_lsl,
	[RBT_OP_LSR] = _op_lsr,         [RBT_OP_MOVE] = _op_move,
	[RBT_OP_MOVEA] = _op_movea,     [RBT_OP_MOVEM] = _op_movem,
	[RBT_OP_MOVEP] = _op_movep,     [RBT_OP_MOVEQ] = _op_moveq,
	[RBT_OP_MULS] = _op_muls,       [RBT_OP_MULU] = _op_mulu,
	[RBT_OP_NBCD] = _op_nbcd,       [RBT_OP_NEG] = _op_neg,
	[RBT_OP_NEGX] = _op_negx,       [RBT_OP_NOP] = _op_nop,
	[RBT_OP_NOT] = _op_not,         [RBT_OP_OR] = _op_or,
	[RBT_OP_ORI] = _op_ori,         [RBT_OP_PEA] = _op_pea,
	[RBT_OP_RESET] = _op_reset,     [RBT_OP_ROL] = _op_rol,
	[RBT_OP_ROR] = _op_ror,         [RBT_OP_ROXL] = _op_roxl,
	[RBT_OP_ROXR] = _op_roxr,       [RBT_OP_RTE] = _op_rte,
	[RBT_OP_RTR] = _op_rtr,         [RBT_OP_RTS] = _op_rts,
	[RBT_OP_SBCD] = _op_sbcd,       [RBT_OP_Scc] = _op_scc,
	[RBT_OP_STOP] = _op_stop,       [RBT_OP_SUB] = _op_sub,
	[RBT_OP_SUBA] = _op_suba,       [RBT_OP_SUBI] = _op_subi,
	[RBT_OP_SUBQ] = _op_subq,       [RBT_OP_SUBX] = _op_subx,
	[RBT_OP_SWAP] = _op_swap,       [RBT_OP_TAS] = _op_tas,
	[RBT_OP_TRAP] = _op_trap,       [RBT_OP_TRAPV] = _op_trapv,
	[RBT_OP_TST] = _op_tst,         [RBT_OP_UNLK] = _op_unlk,

	// M68010+
	[RBT_OP_BKPT] = _op_bkpt,   [RBT_OP_MOVEC] = _op_movec,
	[RBT_OP_MOVES] = _op_moves, [RBT_OP_RTD] = _op_rtd,
};
// clang-format on

static RBT_ErrorCode _cpu_execute(const RBT_Instruction *instr, RBT_Cpu *cpu) {
	assert(instr);
	assert(cpu);

	if (instr->mnemonic == RBT_OP_LINEA)
		return _cpu_raise_exception(cpu, _VEC_LINE_A);

	if (instr->mnemonic == RBT_OP_LINEF)
		return _cpu_raise_exception(cpu, _VEC_LINE_F);

	RBT_OpExec op_exec = _op_dispatch_table[instr->mnemonic];
	if (!op_exec) {
		_push_error(
			RBT_ERR_DECODE_ILLEGAL, "No handler for mnemonic %d", instr->mnemonic
		);
		return _cpu_raise_exception(cpu, _VEC_ILLEGAL);
	}

	return op_exec(instr, cpu);
}
