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

#include "rbt/cpu/cpu.h"

#include "cpu/cpu_internal.h"
#include "cpu/decode.h"
#include "cpu/timing.h"
#include "error.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/bus.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// source-inline-begin
#include "cpu/cpu_execute.inc"
// source-inline-end

static RBT_ErrorCode _cpu_check_exception(RBT_Cpu *cpu) {
	assert(cpu);

	// Group 0 - Exception Processing
	// 0: Reset - asserted at startup
	// 1: Address Error - Thrown by bus
	// 2: Bus Error - Thrown by bus
	if (cpu->pending.address_error) {
		cpu->pending.address_error = false;
		return _cpu_raise_exception(cpu, _VEC_ADDR_ERROR);
	}
	if (cpu->pending.bus_error) {
		cpu->pending.bus_error = false;
		return _cpu_raise_exception(cpu, _VEC_BUS_ERROR);
	}

	// Group 1 - Exception Processing
	// 3: Trace
	// 4: Interrupt
	// 5: Illegal - Check at decode or Instruction
	// 6: Privilege - Check at execution
	if (cpu->state.sr.trace1) {
		return _cpu_raise_exception(cpu, _VEC_TRACE);
	}
	if (cpu->pending.interrupt
		&& cpu->pending.interrupt_level > cpu->state.sr.interrupt_priority) {
		RBT_CpuVector autovec = _VEC_AUTOVEC_L1 + (cpu->pending.interrupt_level - 1);
		RBT_ErrorCode err = _cpu_raise_exception(cpu, autovec);
		if (err)
			return err;

		cpu->pending.interrupt = false;
		cpu->state.sr.interrupt_priority = cpu->pending.interrupt_level;
	}

	// Group 2 - Exception Processing
	// 7: TRAP/TRAVP - Instruction
	// 8: CHK - Instruction
	// 9: Zero Div - Instruction

	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode _cpu_raise_exception(RBT_Cpu *cpu, RBT_CpuVector vec) {
	assert(cpu);

	(void)cpu;
	(void)vec;

	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode _stack_push_word(RBT_Cpu *cpu, u16 word) {
	assert(cpu);
	assert(cpu->bus);

	u32 *sp;
	if (cpu->state.sr.supervisor)
		sp = &cpu->state.ssp;
	else
		sp = &cpu->state.usp;

	// Stack grows downwards
	*sp -= 2; // Word is 2-bytes
	cpu->state.gpr.sp = *sp;

	return rbt_bus_write_word(cpu->bus, *sp, word);
}

RBT_ErrorCode _stack_push_long(RBT_Cpu *cpu, u32 long_) {
	assert(cpu);
	assert(cpu->bus);

	u32 *sp;
	if (cpu->state.sr.supervisor)
		sp = &cpu->state.ssp;
	else
		sp = &cpu->state.usp;

	// Stack grows downwards
	*sp -= 4; // Long is 4-bytes
	cpu->state.gpr.sp = *sp;

	return rbt_bus_write_long(cpu->bus, *sp, long_);
}

RBT_ErrorCode _stack_pop_word(RBT_Cpu *cpu, u16 *out) {
	assert(cpu);
	assert(cpu->bus);

	u32 *sp;
	if (cpu->state.sr.supervisor)
		sp = &cpu->state.ssp;
	else
		sp = &cpu->state.usp;

	RBT_ErrorCode err = rbt_bus_read_word(cpu->bus, *sp, out);
	if (err)
		return err;

	// Stack grows downwards
	*sp += 2; // Word is 2-bytes
	cpu->state.gpr.sp = *sp;
	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode _stack_pop_long(RBT_Cpu *cpu, u32 *out) {
	assert(cpu);
	assert(cpu->bus);

	u32 *sp;
	if (cpu->state.sr.supervisor)
		sp = &cpu->state.ssp;
	else
		sp = &cpu->state.usp;

	RBT_ErrorCode err = rbt_bus_read_long(cpu->bus, *sp, out);
	if (err)
		return err;

	// Stack grows downwards
	*sp += 4; // Long is 4-bytes
	cpu->state.gpr.sp = *sp;
	return RBT_ERR_SUCCESS;
}

RBT_Cpu *rbt_create_cpu(const RBT_CpuConfig *config) {
	RBT_Cpu *cpu = malloc(sizeof(RBT_Cpu));
	if (!cpu) {
		_push_fatal(
			RBT_ERR_SYS_OUT_OF_MEMORY, "Failed to allocate memory for CPU object"
		);
		return nullptr;
	}
	memset(cpu, 0, sizeof(RBT_Cpu));
	cpu->cfg.model = config ? config->model : RBT_CPU_M68000;
	cpu->cfg.hook = config ? config->hook : nullptr;
	cpu->cfg.userdata = config ? config->userdata : nullptr;

	return cpu;
}

void rbt_destroy_cpu(RBT_Cpu *cpu) {
	if (!cpu)
		return;
	free(cpu);
}

void rbt_cpu_attach_bus(RBT_Cpu *cpu, RBT_MemoryBus *bus) {
	assert(cpu);
	cpu->bus = bus;
}

RBT_ErrorCode rbt_cpu_reset(RBT_Cpu *cpu) {
	assert(cpu);
	assert(cpu->bus);

	memset(&cpu->state, 0, sizeof(RBT_CpuState));
	cpu->state.vbr = 0;

	u32 ssp_vec_addr = _get_vector_address(&cpu->state, _VEC_INITIAL_SSP);
	u32 pc_vec_addr = _get_vector_address(&cpu->state, _VEC_INITIAL_PC);

	RBT_ErrorCode err = rbt_bus_read_long(cpu->bus, ssp_vec_addr, &cpu->state.ssp);
	if (err)
		return err;

	err = rbt_bus_read_long(cpu->bus, pc_vec_addr, &cpu->state.pc);
	if (err)
		return err;

	cpu->state.sr.supervisor = true;	  // CPU is forced into supervisor
	cpu->state.sr.trace1 = false;		  // Disable tracing
	cpu->state.sr.interrupt_priority = 7; // Interrupt mask is set to 7

	// Mirror SSP into A7 since we are in supervisor mode
	cpu->state.gpr.sp = cpu->state.ssp;
	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode rbt_cpu_step(RBT_Cpu *cpu, u16 *out_cycles) {
	assert(cpu);
	assert(cpu->bus);

	if (cpu->is_halted)
		return RBT_ERR_CPU_HALTED;

	RBT_ErrorCode err = _cpu_check_exception(cpu);
	if (err)
		return err;

	err = _decode_instruction(cpu->bus, cpu->state.pc, &cpu->current_instr);
	if (err)
		return err;

	RBT_Instruction *instr = &cpu->current_instr;

	// Increment PC before executing next instruction
	cpu->state.pc += instr->len;

	err = _cpu_execute(instr, cpu);
	if (err)
		return err;

	if (out_cycles)
		*out_cycles = _calculate_timing(instr, &cpu->timing, cpu->cfg.model);
	memset(&cpu->timing, 0, sizeof(RBT_TimingCtx));

	return RBT_ERR_SUCCESS;
}
