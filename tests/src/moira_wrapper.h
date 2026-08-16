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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef struct MoiraCpu MoiraCpu;

typedef u8 (*moira_read8)(void *userdata, u32 addr);
typedef u16 (*moira_read16)(void *userdata, u32 addr);
typedef void (*moira_write8)(void *userdata, u32 addr, u8 byte);
typedef void (*moira_write16)(void *userdata, u32 addr, u16 word);

typedef enum MoiraModel {
	MOIRA_MODEL_M68000 = 0,
	MOIRA_MODEL_M68010,
} MoiraModel;

typedef enum MoiraRegisters {
	MOIRA_REG_D0 = 0,
	MOIRA_REG_D1,
	MOIRA_REG_D2,
	MOIRA_REG_D3,
	MOIRA_REG_D4,
	MOIRA_REG_D5,
	MOIRA_REG_D6,
	MOIRA_REG_D7,
	MOIRA_REG_A0,
	MOIRA_REG_A1,
	MOIRA_REG_A2,
	MOIRA_REG_A3,
	MOIRA_REG_A4,
	MOIRA_REG_A5,
	MOIRA_REG_A6,
	MOIRA_REG_A7,

	MOIRA_REG_PC,
	MOIRA_REG_SR,
	MOIRA_REG_SP,
	MOIRA_REG_USP,
	MOIRA_REG_ISP,
	MOIRA_REG_VBR,
	MOIRA_REG_SFC,
	MOIRA_REG_DFC,
	MOIRA_REG_COUNT,
} MoiraRegisters;

typedef enum MoiraExceptionKind {
	MOIRA_EXC_OK = 0,
	MOIRA_EXC_BUS_ERR,
	MOIRA_EXC_ADDR_ERR,
	MOIRA_EXC_ILLEGAL,
	MOIRA_EXC_DIVBYZERO,
	MOIRA_EXC_CHK,
	MOIRA_EXC_TRAPV,
	MOIRA_EXC_PRIVILEGE,
	MOIRA_EXC_TRACE,
	MOIRA_EXC_LINEA,
	MOIRA_EXC_LINEF,
	MOIRA_EXC_FORMAT_ERR,
	MOIRA_EXC_IRQ_UNINIT,
	MOIRA_EXC_IRQ_SPURIOUS,
	MOIRA_EXC_TRAP,
	MOIRA_EXC_BKPT,
} MoiraExceptionKind;

i32 moira_init(
	MoiraCpu *moira, moira_read8 r8, moira_read16 r16, moira_write8 w8, moira_write16 w16
);
void moira_deinit(MoiraCpu *moira);

void moira_set_userdata(MoiraCpu *cpu, void *userdata);

void moira_reset(MoiraCpu *cpu, MoiraModel model);
void moira_execute(MoiraCpu *cpu);

bool moira_is_instruction_valid(const MoiraCpu *cpu, u16 opcode, u16 ext);

i32 moira_disassemble(const MoiraCpu *cpu, char *str, u32 addr);

void moira_get_registers(const MoiraCpu *cpu, u32 *regs);
void moira_set_registers(MoiraCpu *cpu, const u32 *regs);

i64 moira_get_clocks(const MoiraCpu *cpu);
void moira_set_clocks(MoiraCpu *cpu, i64 cycles);

u32 moira_get_pc0(const MoiraCpu *cpu);
u8 moira_read_fc(const MoiraCpu *cpu);
u8 moira_get_ipl(const MoiraCpu *cpu);

void moira_get_exception(const MoiraCpu *cpu, i32 *out_vector, u16 *out_kind);

#ifdef __cplusplus
}
#endif
