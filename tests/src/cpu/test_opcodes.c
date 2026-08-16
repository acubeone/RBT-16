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

#include "cpu/bus_internal.h"
#include "cpu/decode.h"
#include "opcodes.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/bus.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"
#include "unity_internals.h"

#include <stdio.h>
#include <unity.h>

#define _PRINT_PC (0)

enum {
#if (_PRINT_PC)
	_ALIGN_PC = 0,
#else
	_ALIGN_PC = -36,
#endif

	_ALIGN_MNEMONIC = _ALIGN_PC + 40,
	_ALIGN_OPERATORS = _ALIGN_MNEMONIC + 8,
};

static RBT_MemoryBus *_bus;

// clang-format off
static const char *_mnemonics[] = {
	"abcd", "add", "adda", "addi", "addq","addx",
	"and", "andi",
	"asl", "asr",
	"b",
	"bchg", "bclr",
	"bra",
	"bset",
	"bsr",
	"btst",
	"chk",
	"clr",
	"cmp", "cmpa", "cmpi", "cmpm",
	"db",
	"divs", "divu",
	"eor", "eori",
	"exg",
	"ext",
	"illegal",
	"jmp", "jsr",
	"lea",
	"link",
	"lsl", "lsr",
	"move", "movea", "movem", "movep", "moveq",
	"muls", "mulu",
	"nbcd",
	"neg", "negx",
	"nop",
	"not",
	"or", "ori",
	"pea",
	"reset",
	"rol", "ror", "roxl", "roxr",
	"rte", "rtr", "rts",
	"sbcd",
	"s",
	"stop",
	"sub", "suba", "subi", "subq", "subx",
	"swap",
	"tas",
	"trap", "trapv",
	"tst",
	"unlk",

	"bkpt", "movec", "moves", "rtd",

	"linea", "linef",
};

static const char *_conditions[] = {
	"t", "f", "hi", "ls",
	"cc", "cs", "ne", "eq",
	"vc", "vs", "pl", "mi",
	"ge", "lt", "gt", "le",
};
// clang-format on

static i32 _stringfy_effective_address(const RBT_EffectiveAddress *ea, char *out) {
	switch (ea->mode) {
	case RBT_EA_DIRECT_DATA:	  return sprintf(out, "%%d%u", ea->reg);
	case RBT_EA_DIRECT_ADDR:	  return sprintf(out, "%%a%u", ea->reg);
	case RBT_EA_INDIRECT:		  return sprintf(out, "(%%a%u)", ea->indirect);
	case RBT_EA_INDIRECT_POSTINC: return sprintf(out, "(%%a%u)+", ea->indirect);
	case RBT_EA_INDIRECT_PREDEC:  return sprintf(out, "-(%%a%u)", ea->indirect);
	case RBT_EA_INDIRECT_DISPLACEMENT:
		return sprintf(out, "%i(%%a%u)", ea->ind_disp.disp, ea->ind_disp.areg);
	case RBT_EA_INDIRECT_INDEXED: {
		const RBT_IndirectIndexed *ix = &ea->ind_idx;

		char xreg = 'd';
		if (ix->ix.is_addr)
			xreg = 'a';

		char size = 'w';
		if (ix->ix.is_long)
			xreg = 'l';

		return sprintf(
			out, "%i(%%a%u, %%%c%u.%c)", ix->ix.disp, ix->areg, xreg, ix->ix.xreg, size
		);
	};
	case RBT_EA_ABSOLUTE_SHORT:	 return sprintf(out, "(0x%04x).w", ea->absolute_short);
	case RBT_EA_ABSOLUTE_LONG:	 return sprintf(out, "(0x%08x).l", ea->absolute_long);
	case RBT_EA_PC_DISPLACEMENT: return sprintf(out, "%i(%%pc)", ea->pc_disp);
	case RBT_EA_PC_INDEXED:		 {
		char xreg = 'd';
		if (ea->pc_idx.is_addr)
			xreg = 'a';

		char size = 'w';
		if (ea->pc_idx.is_long)
			size = 'l';

		return sprintf(
			out, "%i(%%pc, %%%c%u.%c)", ea->pc_idx.disp, xreg, ea->pc_idx.xreg, size
		);
	};
	case RBT_EA_IMMEDIATE:	  return sprintf(out, "#0x%x", ea->imm);
	case RBT_EA_DISPLACEMENT: return sprintf(out, "%i", ea->disp);
	case RBT_EA_REGISTER_CCR: return sprintf(out, "%%ccr");
	case RBT_EA_REGISTER_SR:  return sprintf(out, "%%sr");
	case RBT_EA_REGISTER_USP: return sprintf(out, "%%usp");
	default:				  return 0;
	}

	unreachable();
}
static i32 _align_text(i32 len, i32 alignment, char *out) {
	while (len < alignment) {
		out[len] = ' ';
		len += 1;
	}

	return len;
}

static void test_opcodes(void) {
	u32 pc = _BUS_ROM_ADDR;

	while (true) {
		RBT_Instruction instr;
		RBT_ErrorCode err = _decode_instruction(_bus, pc, &instr);

		if ((pc - _BUS_ROM_ADDR) >= opcodes_data_size)
			break;

		pc += instr.len;
		char out[256] = {};

		char *size = "";
		if (instr.size == RBT_SIZE_BYTE)
			size = ".b";
		else if (instr.size == RBT_SIZE_WORD)
			size = ".w";
		else if (instr.size == RBT_SIZE_LONG)
			size = ".l";

		i32 len = 0;
		if (_PRINT_PC) {
			len = _align_text(len, _ALIGN_PC, out);
			len += sprintf(&out[len], "%06x: ", instr.start_pc);

			for (i32 i = 0; i < instr.word_count; i += 1) {
				len += sprintf(&out[len], "%04x ", instr.words[i]);
			}
		}

		len = _align_text(len, _ALIGN_MNEMONIC, out);

		if (instr.mnemonic == RBT_OP_Scc || instr.mnemonic == RBT_OP_DBcc
			|| instr.mnemonic == RBT_OP_Bcc) {
			u8 cond = instr.aux.imm;
			len += sprintf(
				&out[len], "%s%s%s ", _mnemonics[instr.mnemonic], _conditions[cond], size
			);
		} else {
			len += sprintf(&out[len], "%s%s ", _mnemonics[instr.mnemonic], size);
		}

		len = _align_text(len, _ALIGN_OPERATORS, out);
		if (instr.mnemonic == RBT_OP_MOVEC) {
			RBT_AddressMode ctrl_reg = RBT_EA_REGISTER_SFC | RBT_EA_REGISTER_DFC
									 | RBT_EA_REGISTER_USP | RBT_EA_REGISTER_VBR;

			char *reg;
			bool to_ctrl = (instr.src.mode & ctrl_reg) != 0;
			RBT_AddressMode ctrl;

			if (to_ctrl)
				ctrl = instr.src.mode;
			else
				ctrl = instr.dst.mode;

			switch (ctrl) {
			case RBT_EA_REGISTER_SFC: reg = "%sfc"; break;
			case RBT_EA_REGISTER_DFC: reg = "%dfc"; break;
			case RBT_EA_REGISTER_USP: reg = "%usp"; break;
			case RBT_EA_REGISTER_VBR: reg = "%vbr"; break;
			default:				  reg = ""; break;
			}

			if (to_ctrl) {
				// movec Rc,Rn
				len += sprintf(&out[len], "%s, ", reg);
				len += _stringfy_effective_address(&instr.dst, &out[len]);
			} else {
				// movec Rn,Rc
				len += _stringfy_effective_address(&instr.src, &out[len]);
				len += sprintf(&out[len], ", %s", reg);
			}
		} else {
			len += _stringfy_effective_address(&instr.src, &out[len]);
			if (instr.src.mode != RBT_EA_NONE && instr.dst.mode != RBT_EA_NONE)
				len += sprintf(&out[len], ", ");
			len += _stringfy_effective_address(&instr.dst, &out[len]);
		}

		fprintf(stdout, "%s\n", out);

		TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, err);
	}
}

void setUp(void) {
	RBT_BusConfig cfg = {
		.ram_slots = { RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE },
	};
	_bus = rbt_create_bus(&cfg);

	TEST_ASSERT_NOT_NULL(_bus);

	TEST_ASSERT_EQUAL(
		RBT_ERR_SUCCESS, rbt_bus_init(_bus, opcodes_data_size, opcodes_data)
	);
}

void tearDown(void) {
	rbt_destroy_bus(_bus);
	rbt_err_flush();
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_opcodes);

	return UNITY_END();
}
