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

#include "cpu/timing.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/bus.h"
#include "rbt/cpu/cpu.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"
#include "rbt/helpers.h"

#include <assert.h>

typedef enum RBT_CpuVector {
	_VEC_INITIAL_SSP = 0, // Reset: Initial SSP
	_VEC_INITIAL_PC = 1,  // Reset: Initial PC
	_VEC_BUS_ERROR = 2,	  // Buss Error
	_VEC_ADDR_ERROR = 3,  // Address Error
	_VEC_ILLEGAL = 4,	  // Illegal Instruction
	_VEC_ZERO_DIV = 5,	  // Divison By Zero
	_VEC_CHK = 6,		  // CHK Instruction
	_VEC_TRAPV = 7,		  // TRAPV Instruction
	_VEC_PRIVILEGE = 8,	  // Privilege Violation
	_VEC_TRACE = 9,		  // Trace
	_VEC_LINE_A = 10,	  // Line 1010 Emulator
	_VEC_LINE_F = 11,	  // Line 1111 Emulator

	// 12-13: (Unassigned, Reserved)

	_VEC_FMT_ERROR = 14,  // Format Error
	_VEC_IRQ_UNINIT = 15, // Unitialized Interrupt Vector

	// 16-23: (Unassigned, Reserved)

	_VEC_SPURIOUS = 24,	  // Spurious Interrupt
	_VEC_AUTOVEC_L1 = 25, // Level 1 Interrupt Autovector
	_VEC_AUTOVEC_L2 = 26, // Level 2 Interrupt Autovector
	_VEC_AUTOVEC_L3 = 27, // Level 3 Interrupt Autovector
	_VEC_AUTOVEC_L4 = 28, // Level 4 Interrupt Autovector
	_VEC_AUTOVEC_L5 = 29, // Level 5 Interrupt Autovector
	_VEC_AUTOVEC_L6 = 30, // Level 6 Interrupt Autovector
	_VEC_AUTOVEC_L7 = 31, // Level 7 Interrupt Autovector

	_VEC_TRAP_0 = 32, // TRAP #$0
	_VEC_TRAP_1 = 33, // TRAP #$1
	_VEC_TRAP_2 = 34, // TRAP #$2
	_VEC_TRAP_3 = 35, // TRAP #$3
	_VEC_TRAP_4 = 36, // TRAP #$4
	_VEC_TRAP_5 = 37, // TRAP #$5
	_VEC_TRAP_6 = 38, // TRAP #$6
	_VEC_TRAP_7 = 39, // TRAP #$7
	_VEC_TRAP_8 = 40, // TRAP #$8
	_VEC_TRAP_9 = 41, // TRAP #$9
	_VEC_TRAP_A = 42, // TRAP #$a
	_VEC_TRAP_B = 43, // TRAP #$b
	_VEC_TRAP_C = 44, // TRAP #$c
	_VEC_TRAP_D = 45, // TRAP #$d
	_VEC_TRAP_E = 46, // TRAP #$e
	_VEC_TRAP_F = 47, // TRAP #$f

	// 48-63: (Unassigned, Reserved)

	_VEC_USER_FIRST = 64,
	_VEC_USER_LAST = 255,
} RBT_CpuVector;

typedef struct RBT_CpuPendingException {
	bool bus_error;
	bool address_error;
	bool interrupt;
	u8 interrupt_level; // 1-7
} RBT_CpuPendingException;

typedef struct RBT_CpuFaultInfo {
	u32 addr;
	u16 opcode;
	u8 function_code;
	bool is_read;
	bool is_fetch;
} RBT_CpuFaultInfo;

typedef struct RBT_Cpu {
	RBT_CpuConfig cfg; // General CPU configuration

	RBT_CpuState state;
	RBT_MemoryBus *bus;
	RBT_Instruction current_instr;

	RBT_CpuFaultInfo fault;
	RBT_TimingCtx timing;
	RBT_CpuPendingException pending;
	bool is_halted;
} RBT_Cpu;

RBT_ErrorCode _cpu_raise_exception(RBT_Cpu *cpu, RBT_CpuVector vec);

RBT_ErrorCode _stack_push_word(RBT_Cpu *cpu, u16 word);
RBT_ErrorCode _stack_push_long(RBT_Cpu *cpu, u32 long_);

RBT_ErrorCode _stack_pop_word(RBT_Cpu *cpu, u16 *out);
RBT_ErrorCode _stack_pop_long(RBT_Cpu *cpu, u32 *out);

[[nodiscard]] static inline u32 _get_vector_address(
	const RBT_CpuState *state, RBT_CpuVector vec
) {
	assert(state);
	return state->vbr + ((u32)vec * 4);
}

[[nodiscard]] static inline u16 _pack_status_register(const RBT_StatusRegister *sr) {
	assert(sr);

	u16 word = 0;

	if (sr->carry)
		word |= 1 << 0;
	if (sr->overflow)
		word |= 1 << 1;
	if (sr->zero)
		word |= 1 << 2;
	if (sr->negative)
		word |= 1 << 3;
	if (sr->extend)
		word |= 1 << 4;

	word |= (sr->interrupt_priority & 0x07) << 8;

	// if (sr->master)
	//	word |= 1 << 12;
	if (sr->supervisor)
		word |= 1 << 13;
	// if (sr->trace0)
	//	word |= 1 << 14;
	if (sr->trace1)
		word |= 1 << 15;

	return word;
}

static inline void _unpack_status_register(RBT_StatusRegister *sr, u16 word) {
	assert(sr);

	sr->carry = word & (1 << 0);
	sr->overflow = word & (1 << 1);
	sr->zero = word & (1 << 2);
	sr->negative = word & (1 << 3);
	sr->extend = word & (1 << 4);
	sr->interrupt_priority = rbt_bits(word, 10, 8);
	// sr->master = word & (1 << 12);
	sr->supervisor = word & (1 << 13);
	// sr->trace0 = word & (1 << 14);
	sr->trace1 = word & (1 << 15);
}
