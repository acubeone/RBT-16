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

#include "rbt/basic_types.h"

enum {
	RBT_MAX_INSTR_WORDS = 16,
};

typedef enum RBT_CpuModel {
	RBT_CPU_M68000 = 1 << 0,
	RBT_CPU_M68010 = 1 << 1,
} RBT_CpuModel;

typedef enum RBT_OperandSize {
	RBT_SIZE_BYTE = 1, // .b
	RBT_SIZE_WORD = 2, // .w
	RBT_SIZE_LONG = 4, // .l
	RBT_SIZE_NONE = 0,
} RBT_OperandSize;

typedef enum RBT_AddressMode : u64 {
	RBT_EA_NONE = 0,

	RBT_EA_DIRECT_DATA = 1 << 0,		   // mode:000, reg:Dn  | Dn
	RBT_EA_DIRECT_ADDR = 1 << 1,		   // mode:001, reg:An  | An
	RBT_EA_INDIRECT = 1 << 2,			   // mode:010, reg:An  | (An)
	RBT_EA_INDIRECT_POSTINC = 1 << 3,	   // mode:011, reg:An  | (An)+
	RBT_EA_INDIRECT_PREDEC = 1 << 4,	   // mode:100, reg:An  | -(An)
	RBT_EA_INDIRECT_DISPLACEMENT = 1 << 5, // mode:101, reg:An  | (d16, An)
	RBT_EA_INDIRECT_INDEXED = 1 << 6,	   // mode:110, reg:An  | (d8, Xi, An)
	RBT_EA_ABSOLUTE_SHORT = 1 << 7,		   // mode:111, reg:000 | (xxx).w
	RBT_EA_ABSOLUTE_LONG = 1 << 8,		   // mode:111, reg:001 | (xxx).l
	RBT_EA_PC_DISPLACEMENT = 1 << 9,	   // mode:111, reg:010 | (d16, PC)
	RBT_EA_PC_INDEXED = 1 << 10,		   // mode:111, reg:011 | (d8, Xi, PC)
	RBT_EA_IMMEDIATE = 1 << 11,			   // mode:111, reg:100 | #imm

	RBT_EA_DISPLACEMENT = 1 << 12, // d16 | Displacement
	RBT_EA_REGISTER_CCR = 1 << 13, // CCR | Condition Code Register
	RBT_EA_REGISTER_SR = 1 << 14,  // SR  | Status Register
	RBT_EA_REGISTER_USP = 1 << 15, // USP | User Stack Pointer
	RBT_EA_REGISTER_DFC = 1 << 16, // DFC | Destination Function Code Register
	RBT_EA_REGISTER_SFC = 1 << 17, // SFC | Source Function Code Register
	RBT_EA_REGISTER_VBR = 1 << 18, // VBR | Vector Base Register

	// GROUPS
	RBT_EA_GROUP_REG = RBT_EA_DIRECT_DATA | RBT_EA_DIRECT_ADDR,
	RBT_EA_GROUP_IND = RBT_EA_INDIRECT | RBT_EA_INDIRECT_POSTINC | RBT_EA_INDIRECT_PREDEC
					 | RBT_EA_INDIRECT_DISPLACEMENT | RBT_EA_INDIRECT_INDEXED,
	RBT_EA_GROUP_IDX = RBT_EA_INDIRECT_INDEXED | RBT_EA_PC_INDEXED,
	RBT_EA_GROUP_ABS = RBT_EA_ABSOLUTE_SHORT | RBT_EA_ABSOLUTE_LONG,
	RBT_EA_GROUP_PCR = RBT_EA_PC_DISPLACEMENT | RBT_EA_PC_INDEXED,
	RBT_EA_GROUP_DSP = RBT_EA_INDIRECT_DISPLACEMENT | RBT_EA_PC_DISPLACEMENT,
	RBT_EA_GROUP_REL = RBT_EA_INDIRECT_POSTINC | RBT_EA_INDIRECT_PREDEC,
	RBT_EA_GROUP_MEM = RBT_EA_GROUP_IND | RBT_EA_GROUP_ABS | RBT_EA_GROUP_PCR
					 | RBT_EA_IMMEDIATE,
} RBT_AddressMode;

typedef enum RBT_OpCondition {
	RBT_COND_T = 0b0000,  // True
	RBT_COND_F = 0b0001,  // False
	RBT_COND_HI = 0b0010, // HIgh
	RBT_COND_LS = 0b0011, // Lower or Same
	RBT_COND_CC = 0b0100, // Carry Clear
	RBT_COND_CS = 0b0101, // Carry Set
	RBT_COND_NE = 0b0110, // Not Equal
	RBT_COND_EQ = 0b0111, // EQual
	RBT_COND_VC = 0b1000, // oVerflow Clear
	RBT_COND_VS = 0b1001, // oVerflow Set
	RBT_COND_PL = 0b1010, // PLus
	RBT_COND_MI = 0b1011, // MInus
	RBT_COND_GE = 0b1100, // Greater or Equal
	RBT_COND_LT = 0b1101, // Less Than
	RBT_COND_GT = 0b1110, // Greater Than
	RBT_COND_LE = 0b1111, // Lower or Equal
} RBT_OpCondition;

// clang-format off
typedef enum RBT_OpMnemonic {
	RBT_OP_ABCD,  RBT_OP_ADD,   RBT_OP_ADDA,    RBT_OP_ADDI,
	RBT_OP_ADDQ,  RBT_OP_ADDX,  RBT_OP_AND,     RBT_OP_ANDI,
	RBT_OP_ASL,   RBT_OP_ASR,   RBT_OP_Bcc,     RBT_OP_BCHG,
	RBT_OP_BCLR,  RBT_OP_BRA,   RBT_OP_BSET,    RBT_OP_BSR,
	RBT_OP_BTST,  RBT_OP_CHK,   RBT_OP_CLR,     RBT_OP_CMP,
	RBT_OP_CMPA,  RBT_OP_CMPI,  RBT_OP_CMPM,    RBT_OP_DBcc,
	RBT_OP_DIVS,  RBT_OP_DIVU,  RBT_OP_EOR,     RBT_OP_EORI,
	RBT_OP_EXG,   RBT_OP_EXT,   RBT_OP_ILLEGAL, RBT_OP_JMP,
	RBT_OP_JSR,   RBT_OP_LEA,   RBT_OP_LINK,    RBT_OP_LSL,
	RBT_OP_LSR,   RBT_OP_MOVE,  RBT_OP_MOVEA,   RBT_OP_MOVEM,
	RBT_OP_MOVEP, RBT_OP_MOVEQ, RBT_OP_MULS,    RBT_OP_MULU,
	RBT_OP_NBCD,  RBT_OP_NEG,   RBT_OP_NEGX,    RBT_OP_NOP,
	RBT_OP_NOT,   RBT_OP_OR,    RBT_OP_ORI,     RBT_OP_PEA,
	RBT_OP_RESET, RBT_OP_ROL,   RBT_OP_ROR,     RBT_OP_ROXL,
	RBT_OP_ROXR,  RBT_OP_RTE,   RBT_OP_RTR,     RBT_OP_RTS,
	RBT_OP_SBCD,  RBT_OP_Scc,   RBT_OP_STOP,    RBT_OP_SUB,
	RBT_OP_SUBA,  RBT_OP_SUBI,  RBT_OP_SUBQ,    RBT_OP_SUBX,
	RBT_OP_SWAP,  RBT_OP_TAS,   RBT_OP_TRAP,    RBT_OP_TRAPV,
	RBT_OP_TST,   RBT_OP_UNLK,

	// M68010+
	RBT_OP_BKPT, RBT_OP_MOVEC, RBT_OP_MOVES, RBT_OP_RTD,

	_RBT_OP_COUNT,

	// Unimplemeted lines
	RBT_OP_LINEA, RBT_OP_LINEF,
} RBT_OpMnemonic;
// clang-format on

typedef struct RBT_IndexExtension {
	bool is_addr;
	bool is_long;
	u8 xreg;
	// u8 scale; // M68020+; Must always be 0 on M68000/MC68008/MC68010
	i32 disp;
} RBT_IndexExtension;

typedef struct RBT_IndirectDisp {
	u8 areg; // An
	i32 disp;
} RBT_IndirectDisp;

typedef struct RBT_IndirectIndexed {
	u8 areg; // An
	RBT_IndexExtension ix;
} RBT_IndirectIndexed;

typedef struct RBT_EffectiveAddress {
	RBT_AddressMode mode;
	u32 start_pc;
	RBT_OperandSize size;

	union {
		u8 reg;						 // EA = Dn or An
		u8 indirect;				 // EA = (An) / (An) + SIZE / (An) - SIZE
		RBT_IndirectDisp ind_disp;	 // EA = (An) + d16
		RBT_IndirectIndexed ind_idx; // EA = (An) + (Xi) + d8
		u32 absolute_short;			 // EA = (xxx).w | Sign Extended
		u32 absolute_long;			 // EA = (xxx).l
		i32 pc_disp;				 // EA = (PC) + d16
		RBT_IndexExtension pc_idx;	 // EA = (PC) + (Xi) + d8
		u32 imm;					 // EA = #imm, reg-list, conditions
		i32 disp;					 // EA = d16
	};
} RBT_EffectiveAddress;

typedef struct RBT_Instruction {
	RBT_OpMnemonic mnemonic;
	RBT_OperandSize size; // General size of instruction
	u32 start_pc;
	u8 len;

	RBT_EffectiveAddress aux;
	RBT_EffectiveAddress src;
	RBT_EffectiveAddress dst;

	// Raw instruction words (max: opcode + 15 extension words)
	u16 words[RBT_MAX_INSTR_WORDS];
	u8 word_count;
} RBT_Instruction;

// F E D C B A  9  8  7 6 5 4 3 2 1 0
// T 0 S M 0 I2 I1 I0 0 0 0 X N Z V C
typedef struct RBT_StatusRegister {
	bool carry;
	bool overflow;
	bool zero;
	bool negative;
	bool extend;
	u8 interrupt_priority;
	// bool master; // M68020+
	bool supervisor;
	// bool trace0; // M68020+
	bool trace1;
} RBT_StatusRegister;

typedef union RBT_GeneralRegisters {
	u32 flat[16]; // D0-D7 + A0-A7

	struct {
		u32 data[8]; // D0-D7
		u32 addr[8]; // A0-A7
	};
	struct {
		u32 __padding[15];
		u32 sp; // A7 - Stack Pointer
	};
} RBT_GeneralRegisters;

typedef struct RBT_CpuState {
	u32 pc;	 // Current Program Counter
	u32 usp; // User Stack Pointer
	u32 ssp; // System Stack Pointer

	RBT_StatusRegister sr;
	RBT_GeneralRegisters gpr;

	// M68010+
	u32 vbr; // Vector Base Register
	u8 dfc;	 // Destination Function Code
	u8 sfc;	 // Source Function Code
} RBT_CpuState;
