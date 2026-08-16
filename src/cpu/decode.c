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

#include "cpu/decode.h"

#include "cpu/bus_internal.h"
#include "cpu/effective_address.h"
#include "error.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/bus.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"
#include "rbt/helpers.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Opcode field bits
#define _OP_GROUP(word)	   (rbt_bits((word), 15, 12))
#define _OP_SUBGROUP(word) (rbt_bits((word), 11, 8))

#define _OP_REG(word)	 (rbt_bits((word), 11, 9))
#define _OP_SIZE(word)	 (rbt_bits((word), 7, 6))
#define _OP_COND(word)	 (rbt_bits((word), 11, 8))
#define _OP_OFFSET(word) (rbt_bits((word), 7, 0))

#define _OP_EA_MODE(word) (rbt_bits((word), 5, 3))
#define _OP_EA_REG(word)  (rbt_bits((word), 2, 0))

#define _OP_TYPE(word)	_OP_REG(word)
#define _OP_SHIFT(word) _OP_REG(word)
#define _OP_QUICK(word) _OP_REG(word)
#define _OP_REGY(word)	_OP_EA_REG(word)
#define _OP_REGX(word)	_OP_REG(word)

#define _OP_MOVEQ_QUICK(word) _OP_OFFSET(word)

#define _OP_MOVE_SRC_MODE(word) _OP_EA_MODE(word)
#define _OP_MOVE_SRC_REG(word)	_OP_EA_REG(word)
#define _OP_MOVE_DST_MODE(word) (rbt_bits((word), 8, 6))
#define _OP_MOVE_DST_REG(word)	(rbt_bits((word), 11, 9))

[[nodiscard]] static bool _validate_ea(
	const RBT_EffectiveAddress *ea,
	u16 invalid_modes,
	const char *instr_name,
	const char *operand_name,
	u32 pc
) {
	if ((ea->mode & invalid_modes) == 0u) {
		return true;
	}

	static char buf[128] = {};

	i32 pos = 0;
	if (invalid_modes & RBT_EA_DIRECT_DATA)
		pos += snprintf(buf + pos, sizeof(buf) - pos, "%sDn", pos > 0 ? "|" : "");
	if (invalid_modes & RBT_EA_DIRECT_ADDR)
		pos += snprintf(buf + pos, sizeof(buf) - pos, "%sAn", pos > 0 ? "|" : "");
	if (invalid_modes & RBT_EA_IMMEDIATE)
		pos += snprintf(buf + pos, sizeof(buf) - pos, "%s#imm", pos > 0 ? "|" : "");
	if (invalid_modes & RBT_EA_GROUP_PCR)
		pos += snprintf(buf + pos, sizeof(buf) - pos, "%sPC-rel", pos > 0 ? "|" : "");
	if (invalid_modes & RBT_EA_INDIRECT_POSTINC)
		pos += snprintf(buf + pos, sizeof(buf) - pos, "%s(An)+", pos > 0 ? "|" : "");
	if (invalid_modes & RBT_EA_INDIRECT_PREDEC)
		pos += snprintf(buf + pos, sizeof(buf) - pos, "%s-(An)", pos > 0 ? "|" : "");

	_push_warn(
		"%s: %s EA(%s) isn't allowed, at: 0x%06x", instr_name, operand_name, buf, pc
	);
	return false;
}

[[nodiscard]] static inline RBT_OperandSize _decode_size(u8 size) {
	switch (size) {
	case 0b00: return RBT_SIZE_BYTE;
	case 0b01: return RBT_SIZE_WORD;
	case 0b10: return RBT_SIZE_LONG;
	default:   return RBT_SIZE_NONE;
	}
}

[[nodiscard]] static inline RBT_AddressMode _decode_ctrl_register(u16 ctrl) {
	switch (ctrl) {
	case 0x000: return RBT_EA_REGISTER_SFC;
	case 0x001: return RBT_EA_REGISTER_DFC;
	case 0x800: return RBT_EA_REGISTER_USP;
	case 0x801: return RBT_EA_REGISTER_VBR;
	default:	return RBT_EA_NONE;
	}

	unreachable();
}

[[nodiscard]] static u8 _store_operand_as_words(
	const RBT_EffectiveAddress *ea, u16 *words
) {
	assert(ea);

	switch (ea->mode) {
	case RBT_EA_INDIRECT_DISPLACEMENT: //
		words[0] = ea->ind_disp.disp & 0xffff;
		return 1;
	case RBT_EA_INDIRECT_INDEXED: //
		words[0] = _indexext_to_word(&ea->ind_idx.ix);
		return 1;
	case RBT_EA_ABSOLUTE_SHORT: //
		words[0] = ea->absolute_short & 0xffff;
		return 1;
	case RBT_EA_ABSOLUTE_LONG:
		words[0] = (ea->absolute_long >> 16) & 0xffff;
		words[1] = ea->absolute_long & 0xffff;
		return 2;
	case RBT_EA_PC_DISPLACEMENT: //
		words[0] = ea->pc_disp & 0xffff;
		return 1;
	case RBT_EA_PC_INDEXED: //
		words[0] = _indexext_to_word(&ea->pc_idx);
		return 1;
	case RBT_EA_IMMEDIATE:
		if (ea->size == RBT_SIZE_LONG) {
			words[0] = (ea->imm >> 16) & 0xffff;
			words[1] = ea->imm & 0xffff;
			return 2;
		}

		words[0] = ea->imm & 0xffff;
		return (ea->size == RBT_SIZE_NONE) ? 0 : 1;
	default: return 0;
	case RBT_EA_DISPLACEMENT:
		words[0] = ea->disp & 0xffff;
		return (ea->size == RBT_SIZE_NONE) ? 0 : 1;
	}

	unreachable();
}

// Static BTST/BCHG/BCLR/BSET: 0000 1000 TT MMMRRR [B.L]
// Dynamic BTST/BCHG/BCLR/BSET: 0000 DDD1 TT MMMRRR [B.L]
static u8 _decode_bit(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 dreg = _OP_REG(opcode);
	u8 type = rbt_bits(opcode, 7, 6);
	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	instr->size = (ea_mode == 0b000) ? RBT_SIZE_LONG : RBT_SIZE_BYTE;

	switch (type) {
	case 0b00: instr->mnemonic = RBT_OP_BTST; break;
	case 0b01: instr->mnemonic = RBT_OP_BCHG; break;
	case 0b10: instr->mnemonic = RBT_OP_BCLR; break;
	case 0b11: instr->mnemonic = RBT_OP_BSET; break;
	}

	// Is dynamic?
	if (rbt_bits(opcode, 11, 8) == 0b1000) {
		u16 bits;
		if (rbt_bus_read_word(bus, curr_pc, &bits)) {
			const RBT_ErrorEntry *last = rbt_query_last_error();
			return last ? last->code : RBT_ERR_GENERIC;
		}
		curr_pc += 2;

		instr->src.mode = RBT_EA_IMMEDIATE;
		instr->src.size = RBT_SIZE_WORD;
		instr->src.imm = bits;
	} else if (RBT_BIT(opcode, 8)) {
		instr->src.mode = RBT_EA_DIRECT_DATA;
		instr->src.reg = dreg;
	} else {
		_push_error(
			RBT_ERR_DECODE_ILLEGAL, "BIT: Unknown enconding at 0x%06x", instr->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	// <ea> as Dn is long-only
	curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	// EA invalid: An, [PC-relative](if BTST), [#imm](if BTST and not dyn)
	u16 ea_invalid = RBT_EA_DIRECT_ADDR;
	if (instr->mnemonic != RBT_OP_BTST) {
		ea_invalid |= RBT_EA_GROUP_PCR;
	} else if (instr->src.mode == RBT_EA_IMMEDIATE) {
		ea_invalid |= RBT_EA_IMMEDIATE;
	}

	if (!_validate_ea(&instr->dst, ea_invalid, "BIT", "Target", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// ORI/ANDI/SUBI/ADDI/EORI/CMPI: 0000 TTT0 SS MMMRRR [BWL]
static u8 _decode_imm(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 type = _OP_TYPE(opcode);
	u8 size = _OP_SIZE(opcode);
	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("IMM: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	switch (type) {
	case 0b000: instr->mnemonic = RBT_OP_ORI; break;
	case 0b001: instr->mnemonic = RBT_OP_ANDI; break;
	case 0b010: instr->mnemonic = RBT_OP_SUBI; break;
	case 0b011: instr->mnemonic = RBT_OP_ADDI; break;
	case 0b101: instr->mnemonic = RBT_OP_EORI; break;
	case 0b110: instr->mnemonic = RBT_OP_CMPI; break;
	default:
		_push_warn(
			"IMM: Unknown immediate type 0x%02x at: 0x%06x", type, instr->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	instr->src.mode = RBT_EA_IMMEDIATE;
	instr->src.size = instr->size;

	if (_bus_fetch_imm(bus, instr->size, curr_pc, &instr->src.imm)) {
		const RBT_ErrorEntry *last = rbt_query_last_error();
		return last ? last->code : RBT_ERR_GENERIC;
	}

	// Skip out immediate words
	curr_pc += (instr->size == RBT_SIZE_LONG) ? 4 : 2;

	// Is destination CCR/SR?
	if (ea_mode == 0b111 && ea_reg == 0b100) {
		if (instr->mnemonic != RBT_OP_ORI && instr->mnemonic != RBT_OP_ANDI
			&& instr->mnemonic != RBT_OP_EORI) {
			_push_warn("IMM: Illegal CCR/SR destination at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}

		if (instr->size == RBT_SIZE_LONG || instr->size == RBT_SIZE_NONE) {
			_push_warn("IMM: Illegal implied register at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}

		if (instr->size == RBT_SIZE_BYTE)
			instr->dst.mode = RBT_EA_REGISTER_CCR;
		else
			instr->dst.mode = RBT_EA_REGISTER_SR;

		return RBT_ERR_SUCCESS;
	}

	curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	// EA invalid: An, #imm, [PC-relative](if not CMPI)
	u16 ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE;
	if (instr->mnemonic != RBT_OP_CMPI)
		ea_invalid |= RBT_EA_GROUP_PCR;

	if (!_validate_ea(&instr->dst, ea_invalid, "IMM", "Dest", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// MOVEP: 0000 DDD1 OO 001RRR [.WL]
//        OFFSET
// MOVES: 0000 1110 SS MMMRRR [BWL] (M68010+)
//        ARRR d000 0000 0000
static u8 _decode_moves_movep(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	// Is MOVEP?
	if (RBT_BIT(opcode, 8)) {
		if (ea_mode != 0b001) {
			_push_warn("MOVEP: Invalid encoding at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}
		instr->mnemonic = RBT_OP_MOVEP;

		u16 disp;
		if (rbt_bus_read_word(bus, curr_pc, &disp)) {
			const RBT_ErrorEntry *last = rbt_query_last_error();
			return last ? last->code : RBT_ERR_GENERIC;
		}
		curr_pc += 2;

		// OP-MODE:
		//	100: word, mem->reg
		//	101: long, mem->reg
		//	110: word, reg->mem
		//	111: long, reg->mem
		u8 op = rbt_bits(opcode, 7, 6);
		bool to_mem = RBT_BIT(op, 1);

		instr->size = RBT_BIT(op, 0) ? RBT_SIZE_LONG : RBT_SIZE_WORD;
		if (to_mem) {
			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->src.reg = _OP_REG(opcode);

			instr->dst.mode = RBT_EA_INDIRECT_DISPLACEMENT;
			instr->dst.ind_disp.areg = ea_reg;
			instr->dst.ind_disp.disp = rbt_sign_extend(RBT_SIZE_WORD, disp);
		} else {
			instr->src.mode = RBT_EA_INDIRECT_DISPLACEMENT;
			instr->src.ind_disp.areg = ea_reg;
			instr->src.ind_disp.disp = rbt_sign_extend(RBT_SIZE_WORD, disp);

			instr->dst.mode = RBT_EA_DIRECT_DATA;
			instr->dst.reg = _OP_REG(opcode);
		}
	} else {
		if (rbt_bits(opcode, 11, 9) != 0b111) {
			_push_warn("MOVES: Invalid encoding at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}
		instr->mnemonic = RBT_OP_MOVES;
		instr->size = _decode_size(_OP_SIZE(opcode));
		if (instr->size == RBT_SIZE_NONE) {
			_push_warn("Invalid operand size at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}

		u16 ext;
		if (rbt_bus_read_word(bus, curr_pc, &ext)) {
			const RBT_ErrorEntry *last = rbt_query_last_error();
			return last ? last->code : RBT_ERR_GENERIC;
		}
		curr_pc += 2;

		// Store extension word as auxiliar operand
		instr->aux.size = RBT_SIZE_WORD;
		instr->aux.mode = RBT_EA_IMMEDIATE;
		instr->aux.imm = ext;

		// 0: mem->reg; 1: reg->mem
		const RBT_EffectiveAddress *target_ea;
		if (RBT_BIT(ext, 11)) {
			target_ea = &instr->dst;

			instr->src.mode = RBT_BIT(ext, 15) ? RBT_EA_DIRECT_ADDR : RBT_EA_DIRECT_DATA;
			instr->src.reg = rbt_bits(ext, 14, 12);

			curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
		} else {
			target_ea = &instr->src;

			curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);

			instr->dst.mode = RBT_BIT(ext, 15) ? RBT_EA_DIRECT_ADDR : RBT_EA_DIRECT_DATA;
			instr->dst.reg = rbt_bits(ext, 14, 12);
		}

		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		// EA invalid: Dn, An, #imm, PC-relative
		u16 ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_DIRECT_DATA | RBT_EA_IMMEDIATE
					   | RBT_EA_GROUP_PCR;
		if (!_validate_ea(target_ea, ea_invalid, "MOVES", "Target", instr->start_pc)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}
	}

	return RBT_ERR_SUCCESS;
}

// MOVE:  00SS RRRMMM MMMRRR [BWL]
// MOVEA: 00SS RRR001 MMMRRR [.WL]
static u8 _decode_move_movea(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_src_mode = _OP_MOVE_SRC_MODE(opcode);
	u8 ea_src_reg = _OP_MOVE_SRC_REG(opcode);
	u8 ea_dst_mode = _OP_MOVE_DST_MODE(opcode);
	u8 ea_dst_reg = _OP_MOVE_DST_REG(opcode);

	// MOVEA is literally just MOVE with An as EA destination
	instr->mnemonic = (ea_dst_mode == 0b001) ? RBT_OP_MOVEA : RBT_OP_MOVE;

	// Which size?
	switch (rbt_bits(opcode, 13, 12)) {
	case 0b01: instr->size = RBT_SIZE_BYTE; break;
	case 0b11: instr->size = RBT_SIZE_WORD; break;
	case 0b10: instr->size = RBT_SIZE_LONG; break;
	default:
		_push_warn("MOVE/MOVEA: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	if (instr->mnemonic == RBT_OP_MOVEA && instr->size == RBT_SIZE_BYTE) {
		_push_warn("MOVEA: Cannot be byte-sized, at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	curr_pc = _ea_decode(ea_src_mode, ea_src_reg, instr->size, bus, curr_pc, &instr->src);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	curr_pc = _ea_decode(ea_dst_mode, ea_dst_reg, instr->size, bus, curr_pc, &instr->dst);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	// EA invalid: #imm, PC-relative
	u16 ea_invalid = RBT_EA_IMMEDIATE | RBT_EA_GROUP_PCR;
	if (!_validate_ea(&instr->dst, ea_invalid, "MOVE", "Source", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	// An is Word/Long only
	if (instr->src.mode == RBT_EA_DIRECT_ADDR && instr->size == RBT_SIZE_BYTE) {
		_push_warn(
			"MOVE/MOVEA: Source EA(An) cannot be byte-sized, at: 0x%06x", instr->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// MOVE fr SR:  0100 000 011 MMMRRR [.W.]
// MOVE fr CCR: 0100 001 011 MMMRRR [.W.]
// MOVE to CCR: 0100 010 011 MMMRRR [.W.]
// MOVE to SR:  0100 011 011 MMMRRR [.W.]
static u8 _decode_move_reg(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	if (rbt_bits(opcode, 8, 6) != 0b011) {
		_push_warn(
			"MOVE <> SR/CCR: Invalid register encoding at: 0x%06x", instr->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	instr->mnemonic = RBT_OP_MOVE;
	instr->size = RBT_SIZE_WORD;

	switch (rbt_bits(opcode, 11, 9)) {
	case 0b000: instr->src.mode = RBT_EA_REGISTER_SR; break;  // FROM SR
	case 0b001: instr->src.mode = RBT_EA_REGISTER_CCR; break; // FROM CCR
	case 0b010: instr->dst.mode = RBT_EA_REGISTER_CCR; break; // TO CCR
	case 0b011: instr->dst.mode = RBT_EA_REGISTER_SR; break;  // TO SR
	default:
		_push_warn("MOVE <> SR/CCR: Unknown register at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	// EA invalid: An, [#imm, PC-relative](if from CCR/SR)
	u16 ea_invalid;
	RBT_EffectiveAddress *target_ea;
	if (instr->src.mode == RBT_EA_NONE) {
		target_ea = &instr->src;
		ea_invalid = RBT_EA_DIRECT_ADDR;
	} else {
		target_ea = &instr->dst;
		ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE | RBT_EA_GROUP_PCR;
	}

	curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, target_ea);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	if (!_validate_ea(
			target_ea, ea_invalid, "MOVE <> SR/CCR", "Target", instr->start_pc
		)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// NEGX: 0100 0000 SS MMMRRR [BWL]
// NEG:  0100 0100 SS MMMRRR [BWL]
// CLR:  0100 0010 SS MMMRRR [BWL]
// NOT:  0100 0110 SS MMMRRR [BWL]
static u8 _decode_negx_clr_not(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 subgroup = _OP_SUBGROUP(opcode);
	u8 size = _OP_SIZE(opcode);
	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	switch (subgroup) {
	case 0b0000: instr->mnemonic = RBT_OP_NEGX; break;
	case 0b0100: instr->mnemonic = RBT_OP_NEG; break;
	case 0b0010: instr->mnemonic = RBT_OP_CLR; break;
	case 0b0110: instr->mnemonic = RBT_OP_NOT; break;
	default:
		_push_warn("MISC: Unknown opcode encoding at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("MISC: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	// EA invalid: An, PC-relative, #imm
	u16 ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE | RBT_EA_GROUP_PCR;
	if (!_validate_ea(&instr->dst, ea_invalid, "MISC", "Dest", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// MOVEM: 0100 1d001s MMMRRR [.WL]
//        Register List Mask
static u8 _decode_movem(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	instr->mnemonic = RBT_OP_MOVEM;
	instr->size = RBT_BIT(opcode, 6) ? RBT_SIZE_LONG : RBT_SIZE_WORD;

	u16 regs;
	if (rbt_bus_read_word(bus, curr_pc, &regs)) {
		const RBT_ErrorEntry *last = rbt_query_last_error();
		return last ? last->code : RBT_ERR_GENERIC;
	}
	curr_pc += 2;

	// If mem->reg: EA invalid: Dn, An, -(An), #imm
	// If reg->mem: EA invalid: Dn, An, (An)+, PC-relative, #imm
	u16 ea_invalid = RBT_EA_DIRECT_DATA | RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE;
	const RBT_EffectiveAddress *target_ea;

	// 0: regs->mem; 1: mem->regs
	if (RBT_BIT(opcode, 10)) {
		target_ea = &instr->src;
		ea_invalid |= RBT_EA_INDIRECT_PREDEC;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);

		instr->dst.mode = RBT_EA_IMMEDIATE;
		instr->dst.size = RBT_SIZE_WORD;
		instr->dst.imm = regs;
	} else {
		target_ea = &instr->dst;
		ea_invalid |= RBT_EA_INDIRECT_POSTINC | RBT_EA_GROUP_PCR;

		instr->src.mode = RBT_EA_IMMEDIATE;
		instr->src.size = RBT_SIZE_WORD;
		instr->src.imm = regs;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
	}

	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	if (!_validate_ea(target_ea, ea_invalid, "MOVEM", "Target", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// EXT:  0100 100 ooo 000DDD [.WL]
// NBCD: 0100 100 000 MMMRRR [B..]
// SWAP: 0100 100 001 000RRR [.W.]
// BKPT: 0100 100 001 001NNN [...] (M68010+)
// PEA:  0100 100 001 MMMRRR [..L]
static u8 _decode_ext_nbcd_swap_bkpt_pea(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 op = rbt_bits(opcode, 8, 6);
	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	if (RBT_BIT(op, 1)) {
		// OP-MODE: 010 -> byte to word; 011 -> word to long
		instr->mnemonic = RBT_OP_EXT;
		instr->size = RBT_BIT(op, 0) ? RBT_SIZE_LONG : RBT_SIZE_WORD;

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = ea_reg;

		return RBT_ERR_SUCCESS;
	}

	if (op == 0b000) {
		instr->mnemonic = RBT_OP_NBCD;
		instr->size = RBT_SIZE_BYTE;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		// EA invalid: An, #imm, PC-relative
		u16 ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE | RBT_EA_GROUP_PCR;
		if (!_validate_ea(&instr->dst, ea_invalid, "NBCD", "Dest", instr->start_pc)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	if (op == 0b001) {
		if (ea_mode == 0b000) {
			instr->mnemonic = RBT_OP_SWAP;
			instr->size = RBT_SIZE_WORD;

			instr->dst.mode = RBT_EA_DIRECT_DATA;
			instr->dst.reg = ea_reg;
			return RBT_ERR_SUCCESS;
		}

		if (ea_mode == 0b001) {
			instr->mnemonic = RBT_OP_BKPT;
			instr->size = RBT_SIZE_NONE;

			instr->src.size = RBT_SIZE_NONE;
			instr->src.mode = RBT_EA_IMMEDIATE;
			instr->src.imm = rbt_bits(opcode, 2, 0);
			return RBT_ERR_SUCCESS;
		}

		instr->mnemonic = RBT_OP_PEA;
		instr->size = RBT_SIZE_LONG;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		// EA invalid: Dn, An, (An)+, -(An), #imm
		u16 ea_invalid = RBT_EA_DIRECT_DATA | RBT_EA_DIRECT_ADDR | RBT_EA_GROUP_REL;
		if (!_validate_ea(&instr->src, ea_invalid, "PEA", "Source", instr->start_pc)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	_push_warn("MISC: Unknown opcode encoding at: 0x%06x", instr->start_pc);
	return RBT_ERR_DECODE_ILLEGAL;
}

// ILLEGAL: 0100 1010 11111100 [...]
// TAS:     0100 1010 11MMMRRR [B..]
// TST:     0100 1010 SSMMMRRR [BWL]
static u8 _decode_illegal_tas_tst(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];

	if (rbt_bits(opcode, 7, 0) == 0xfc) {
		instr->mnemonic = RBT_OP_ILLEGAL; // >:3c
		instr->size = RBT_SIZE_NONE;
		return RBT_ERR_SUCCESS;
	}

	u8 size = _OP_SIZE(opcode);
	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);
	if (size == 0b11) {
		instr->mnemonic = RBT_OP_TAS;
		instr->size = RBT_SIZE_BYTE;
	} else {
		instr->mnemonic = RBT_OP_TST;
		instr->size = _decode_size(size);
	}
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("TAS/TST: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	u32 new_pc = _ea_decode(
		ea_mode, ea_reg, instr->size, bus, instr->start_pc + 2, &instr->dst
	);
	if (new_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	// EA invalid: Dn, An, #imm, [PC-relative](if TAS)
	u16 ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE;
	if (instr->mnemonic == RBT_OP_TAS)
		ea_invalid |= RBT_EA_GROUP_PCR;

	if (!_validate_ea(&instr->dst, ea_invalid, "TAS/TST", "Dest", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// JMP:      0100 1110 11 MMMRRR [...]
// JSR:      0100 1110 10 MMMRRR [...]
// TRAP:     0100 1110 0100 VVVV [...]
// UNLK:     0100 1110 0101 1RRR [...]
// LINK:     0100 1110 0101 0RRR [.W.]
//           Offset
// MOVE USP: 0100 1110 0110 dRRR [..L]
// MOVEC:    0100 1110 0111 101d [..L] (M68010+)
//           ARRR CTRL_REGISTER
// RESET:    0100 1110 0111 0000 [...]
// NOP:      0100 1110 0111 0001 [...]
// STOP:     0100 1110 0111 0010 [...]
//           Immediate
// RTE:      0100 1110 0111 0011 [...]
// RTD:      0100 1110 0111 0100 [...] (M68010+)
//           Offset
// RTS:      0100 1110 0111 0101 [...]
// TRAPV:    0100 1110 0111 0110 [...]
// RTR:      0100 1110 0111 0111 [...]
static u8 _decode_misc(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	if (RBT_BIT(opcode, 7)) {
		u8 ea_mode = _OP_EA_MODE(opcode);
		u8 ea_reg = _OP_EA_REG(opcode);

		instr->mnemonic = RBT_BIT(opcode, 6) ? RBT_OP_JMP : RBT_OP_JSR;
		instr->size = RBT_SIZE_NONE;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		// EA invalid: Dn, An, #imm, (An)+, -(An)
		u16 ea_invalid = RBT_EA_DIRECT_DATA | RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE
					   | RBT_EA_GROUP_REL;
		if (!_validate_ea(&instr->dst, ea_invalid, "JMP/JSR", "Dest", instr->start_pc)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	u8 subtype = rbt_bits(opcode, 7, 4);

	// TRAP:  0100 1110 0100 VVVV [...]
	if (subtype == 0b0100) {
		instr->mnemonic = RBT_OP_TRAP;
		instr->size = RBT_SIZE_NONE;

		instr->src.size = instr->size;
		instr->src.mode = RBT_EA_IMMEDIATE;
		instr->src.imm = rbt_bits(opcode, 3, 0);
		return RBT_ERR_SUCCESS;
	}

	// UNLK:  0100 1110 0101 1RRR [...]
	// LINK:  0100 1110 0101 0RRR [.W.]
	//        Offset
	if (subtype == 0b0101) {
		instr->mnemonic = RBT_BIT(opcode, 3) ? RBT_OP_UNLK : RBT_OP_LINK;

		instr->src.mode = RBT_EA_DIRECT_ADDR;
		instr->src.reg = rbt_bits(opcode, 2, 0);

		if (instr->mnemonic == RBT_OP_LINK) {
			u16 offset;
			if (rbt_bus_read_word(bus, curr_pc, &offset)) {
				const RBT_ErrorEntry *last = rbt_query_last_error();
				return last ? last->code : RBT_ERR_GENERIC;
			}

			instr->size = RBT_SIZE_WORD;
			instr->dst.mode = RBT_EA_DISPLACEMENT;
			instr->dst.size = instr->size;
			instr->dst.disp = rbt_sign_extend(RBT_SIZE_WORD, offset);
		}

		return RBT_ERR_SUCCESS;
	}

	// MOVE to USP: 0100 1110 0110 0RRR [..L]
	// MOVE fr USP: 0100 1110 0110 1RRR [..L]
	if (subtype == 0b0110) {
		instr->mnemonic = RBT_OP_MOVE;
		instr->size = RBT_SIZE_LONG;

		if (RBT_BIT(opcode, 3) == 0) {
			instr->src.mode = RBT_EA_DIRECT_ADDR;
			instr->src.reg = _OP_EA_REG(opcode);

			instr->dst.mode = RBT_EA_REGISTER_USP;
		} else {
			instr->src.mode = RBT_EA_REGISTER_USP;

			instr->dst.mode = RBT_EA_DIRECT_ADDR;
			instr->dst.reg = _OP_EA_REG(opcode);
		}

		return RBT_ERR_SUCCESS;
	}

	// MOVEC: 0100 1110 0111 101d [..L] (M68010+)
	//        ARRR CTRL_REGISTER
	if (rbt_bits(opcode, 3, 1) == 0b101) {
		u16 aux;
		if (rbt_bus_read_word(bus, curr_pc, &aux)) {
			const RBT_ErrorEntry *last = rbt_query_last_error();
			return last ? last->code : RBT_ERR_GENERIC;
		}

		instr->mnemonic = RBT_OP_MOVEC;
		instr->size = RBT_SIZE_LONG;

		// Store extension word as auxiliar operand
		instr->aux.size = RBT_SIZE_WORD;
		instr->aux.mode = RBT_EA_IMMEDIATE;
		instr->aux.imm = aux;

		u8 ctrl = rbt_bits(aux, 12, 0);
		u8 reg = rbt_bits(aux, 14, 12);
		bool is_addr = RBT_BIT(aux, 15);

		bool to_ctrl = RBT_BIT(opcode, 0); // 1: Rn->Rc; 0: Rc->Rn
		if (to_ctrl) {
			instr->src.mode = is_addr ? RBT_EA_DIRECT_ADDR : RBT_EA_DIRECT_DATA;
			instr->src.reg = reg;

			instr->dst.mode = _decode_ctrl_register(ctrl);
		} else {
			instr->src.mode = _decode_ctrl_register(ctrl);

			instr->dst.mode = is_addr ? RBT_EA_DIRECT_ADDR : RBT_EA_DIRECT_DATA;
			instr->dst.reg = reg;
		}

		return RBT_ERR_SUCCESS;
	}

	// RESET: 0100 1110 0111 0000 [...]
	// NOP:   0100 1110 0111 0001 [...]
	// STOP:  0100 1110 0111 0010 [...]
	//        Immediate
	// RTE:   0100 1110 0111 0011 [...]
	// RTD:   0100 1110 0111 0100 [...] (M68010+)
	//        Offset
	// RTS:   0100 1110 0111 0101 [...]
	// TRAPV: 0100 1110 0111 0110 [...]
	// RTR:   0100 1110 0111 0111 [...]
	switch (rbt_bits(opcode, 3, 0)) {
	case 0b000: instr->mnemonic = RBT_OP_RESET; break;
	case 0b001: instr->mnemonic = RBT_OP_NOP; break;
	case 0b010: instr->mnemonic = RBT_OP_STOP; break;
	case 0b011: instr->mnemonic = RBT_OP_RTE; break;
	case 0b100: instr->mnemonic = RBT_OP_RTD; break;
	case 0b101: instr->mnemonic = RBT_OP_RTS; break;
	case 0b110: instr->mnemonic = RBT_OP_TRAPV; break;
	case 0b111: instr->mnemonic = RBT_OP_RTR; break;
	default:	unreachable();
	}

	if (instr->mnemonic == RBT_OP_STOP) {
		instr->src.size = RBT_SIZE_WORD;
		instr->src.mode = RBT_EA_IMMEDIATE;

		if (_bus_fetch_imm(bus, instr->src.size, curr_pc, &instr->src.imm)) {
			const RBT_ErrorEntry *last = rbt_query_last_error();
			return last ? last->code : RBT_ERR_GENERIC;
		}
	}

	if (instr->mnemonic == RBT_OP_RTD) {
		u16 disp;
		if (rbt_bus_read_word(bus, curr_pc, &disp)) {
			const RBT_ErrorEntry *last = rbt_query_last_error();
			return last ? last->code : RBT_ERR_GENERIC;
		}

		instr->src.mode = RBT_EA_DISPLACEMENT;
		instr->src.size = RBT_SIZE_WORD;
		instr->src.disp = rbt_sign_extend(RBT_SIZE_WORD, disp);
	}

	return RBT_ERR_SUCCESS;
}

// CHK: 0100 DDD 110 MMMRRR [.W.]
// LEA: 0100 AAA 111 MMMRRR [..L]
static u8 _decode_chk_lea(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 subtype = rbt_bits(opcode, 8, 6);
	u8 reg = _OP_MOVE_DST_REG(opcode);
	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);

	instr->mnemonic = RBT_BIT(subtype, 0) ? RBT_OP_LEA : RBT_OP_CHK;
	if (instr->mnemonic == RBT_OP_CHK) {
		instr->size = RBT_SIZE_WORD;
		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = reg;
	} else {
		instr->size = RBT_SIZE_LONG;
		instr->dst.mode = RBT_EA_DIRECT_ADDR;
		instr->dst.reg = reg;
	}

	curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	// EA invalid: An, [Dn, (An)+, -(An), #imm](Only LEA)
	u16 ea_invalid = RBT_EA_DIRECT_ADDR;
	if (instr->mnemonic == RBT_OP_LEA)
		ea_invalid |= RBT_EA_DIRECT_DATA | RBT_EA_IMMEDIATE | RBT_EA_GROUP_REL;

	if (!_validate_ea(&instr->src, ea_invalid, "LEA/CHK", "Source", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// ADDQ: 0101 QQQ0 SS MMMRRR [BWL]
// SUBQ: 0101 QQQ1 SS MMMRRR [BWL]
// Scc:  0101 cccc 11 MMMRRR [B..]
// DBcc: 0101 cccc 11 001RRR [.W.]
static u8 _decode_addq_subq(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);
	u8 size = _OP_SIZE(opcode);

	if (size == 0b11) {
		u8 cond = _OP_COND(opcode);

		if (ea_mode == 0b001) {
			u16 offset;
			if (rbt_bus_read_word(bus, curr_pc, &offset)) {
				const RBT_ErrorEntry *last = rbt_query_last_error();
				return last ? last->code : RBT_ERR_GENERIC;
			}

			instr->mnemonic = RBT_OP_DBcc;
			instr->size = RBT_SIZE_WORD;

			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->src.reg = ea_reg;

			instr->dst.mode = RBT_EA_DISPLACEMENT;
			instr->dst.size = RBT_SIZE_WORD;
			instr->dst.disp = rbt_sign_extend(RBT_SIZE_WORD, offset);

			instr->aux.size = RBT_SIZE_NONE;
			instr->aux.mode = RBT_EA_IMMEDIATE;
			instr->aux.imm = cond;

			return RBT_ERR_SUCCESS;
		}

		instr->mnemonic = RBT_OP_Scc;
		instr->size = RBT_SIZE_BYTE;

		instr->aux.mode = RBT_EA_IMMEDIATE;
		instr->aux.size = RBT_SIZE_NONE;
		instr->aux.imm = cond;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		// EA invalid: An, PC-relative, #imm
		u16 ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_GROUP_PCR | RBT_EA_IMMEDIATE;
		if (!_validate_ea(&instr->dst, ea_invalid, "Scc", "Target", instr->start_pc)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	instr->mnemonic = RBT_BIT(opcode, 8) ? RBT_OP_SUBQ : RBT_OP_ADDQ;
	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("ADDQ/SUBQ: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	instr->src.mode = RBT_EA_IMMEDIATE;
	instr->src.size = RBT_SIZE_NONE;
	instr->src.imm = _OP_QUICK(opcode);

	curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	// An is Word/Long only
	if (instr->dst.mode == RBT_EA_DIRECT_ADDR && instr->size == RBT_SIZE_BYTE) {
		_push_warn(
			"ADDQ/SUBQ: Source EA(An) cannot be byte-sized, at: 0x%06x", instr->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	// EA invalid: PC-relative, #imm
	u16 ea_invalid = RBT_EA_GROUP_PCR | RBT_EA_IMMEDIATE;
	if (!_validate_ea(&instr->dst, ea_invalid, "ADDQ/SUBQ", "Dest", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// BRA: 0110 0000 DDDDDDDD [BW.]
// BSR: 0110 0001 DDDDDDDD [BW.]
// Bcc: 0110 cccc DDDDDDDD [BW.]
static u8 _decode_branch(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 cond = _OP_COND(opcode);
	u16 offset = _OP_OFFSET(opcode);
	instr->size = RBT_SIZE_BYTE;

	// Read 16-bits offset if 8-bits offset is 0x00
	if (offset == 0x00) {
		instr->size = RBT_SIZE_WORD;
		if (rbt_bus_read_word(bus, curr_pc, &offset)) {
			const RBT_ErrorEntry *last = rbt_query_last_error();
			return last ? last->code : RBT_ERR_GENERIC;
		}
	}

	if (cond == RBT_COND_T) {
		instr->mnemonic = RBT_OP_BRA;
	} else if (cond == RBT_COND_F) {
		instr->mnemonic = RBT_OP_BSR;
	} else {
		instr->mnemonic = RBT_OP_Bcc;
	}

	instr->aux.mode = RBT_EA_IMMEDIATE;
	instr->aux.size = RBT_SIZE_NONE;
	instr->aux.imm = cond;

	instr->dst.mode = RBT_EA_DISPLACEMENT;
	instr->dst.size = (instr->size == RBT_SIZE_BYTE) ? RBT_SIZE_NONE : RBT_SIZE_WORD;
	instr->dst.disp = rbt_sign_extend(instr->size, offset);

	return RBT_ERR_SUCCESS;
}

// DIVU: 1000 DDD0 11 MMMRRR [.W.]
// DIVS: 1000 DDD1 11 MMMRRR [.W.]
// SBCD: 1000 RRR 10000 mRRR [B..]
// OR:   1000 DDDd SS MMMRRR [BWL]
static u8 _decode_ordiv(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);
	u8 size = _OP_SIZE(opcode);
	u8 dreg = _OP_REG(opcode);

	if (size == 0b11) {
		instr->mnemonic = RBT_BIT(opcode, 8) ? RBT_OP_DIVS : RBT_OP_DIVU;
		instr->size = RBT_SIZE_WORD;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = dreg;

		// EA invalid: An
		u16 ea_invalid = RBT_EA_DIRECT_ADDR;
		if (!_validate_ea(
				&instr->src, ea_invalid, "DIVU/DIVS", "Source", instr->start_pc
			)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	if (rbt_bits(opcode, 8, 4) == 0b10000) {
		instr->mnemonic = RBT_OP_SBCD;
		instr->size = RBT_SIZE_BYTE;

		// 1: -(Ay)->-(Ax); 0: Dy->Dx
		if (RBT_BIT(opcode, 3)) {
			instr->src.mode = RBT_EA_INDIRECT_PREDEC;
			instr->src.indirect = _OP_REGY(opcode);

			instr->dst.mode = RBT_EA_INDIRECT_PREDEC;
			instr->dst.indirect = _OP_REGX(opcode);
		} else {
			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->src.reg = _OP_REGY(opcode);

			instr->dst.mode = RBT_EA_DIRECT_DATA;
			instr->dst.reg = _OP_REGX(opcode);
		}

		return RBT_ERR_SUCCESS;
	}

	instr->mnemonic = RBT_OP_OR;
	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("OR: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	// EA invalid: An, [Dn, PC-rel, #imm](if <ea> is dest)
	u16 ea_invalid = RBT_EA_DIRECT_ADDR;
	RBT_EffectiveAddress *target_ea;

	// 1: <ea> or Dn -> <ea>
	// 0: Dn or <ea> -> Dn
	if (RBT_BIT(opcode, 8)) {
		target_ea = &instr->dst;
		ea_invalid |= RBT_EA_DIRECT_DATA | RBT_EA_GROUP_PCR | RBT_EA_IMMEDIATE;

		instr->src.mode = RBT_EA_DIRECT_DATA;
		instr->src.reg = dreg;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
	} else {
		target_ea = &instr->src;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = dreg;
	}

	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	if (!_validate_ea(target_ea, ea_invalid, "OR", "Target", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// SUB:  1001 DDDd SS MMMRRR [BWL]
// SUBX: 1001 RRR1 SS 00mRRR [BWL]
// SUBA: 1001 AAAs 11 MMMRRR [.WL]
static u8 _decode_subsubx(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);
	u8 size = _OP_SIZE(opcode);

	if (size == 0b11) {
		instr->mnemonic = RBT_OP_SUBA;
		instr->size = RBT_BIT(opcode, 8) ? RBT_SIZE_LONG : RBT_SIZE_WORD;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		instr->dst.mode = RBT_EA_DIRECT_ADDR;
		instr->dst.reg = _OP_REG(opcode);

		return RBT_ERR_SUCCESS;
	}

	if (RBT_BIT(opcode, 8) && rbt_bits(opcode, 5, 4) == 0b00) {
		instr->mnemonic = RBT_OP_SUBX;
		instr->size = _decode_size(size);
		if (instr->size == RBT_SIZE_NONE) {
			_push_warn("SUBX: Invalid operand size at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}

		u8 reg_y = _OP_EA_REG(opcode); // Source
		u8 reg_x = _OP_REG(opcode);	   // Dest

		// 1: -(An)->-(An); 0: Dn -> Dn
		if (RBT_BIT(opcode, 3)) {
			instr->src.mode = RBT_EA_INDIRECT_PREDEC;
			instr->src.indirect = reg_y;

			instr->dst.mode = RBT_EA_INDIRECT_PREDEC;
			instr->dst.indirect = reg_x;
		} else {
			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->src.reg = reg_y;

			instr->dst.mode = RBT_EA_DIRECT_DATA;
			instr->dst.reg = reg_x;
		}

		return RBT_ERR_SUCCESS;
	}

	instr->mnemonic = RBT_OP_SUB;
	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("SUB: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	// EA invalid: [Dn, An, PC-relative, #imm](if <ea> is dest)
	u16 ea_invalid = 0;
	RBT_EffectiveAddress *target_ea;

	// 1: <ea> - Dn -> <ea>
	// 0: Dn - <ea> -> Dn
	if (RBT_BIT(opcode, 8)) {
		target_ea = &instr->dst;
		ea_invalid = RBT_EA_DIRECT_DATA | RBT_EA_DIRECT_ADDR | RBT_EA_GROUP_PCR
				   | RBT_EA_IMMEDIATE;

		instr->src.mode = RBT_EA_DIRECT_DATA;
		instr->src.reg = _OP_REG(opcode);

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);

		if (instr->dst.mode == RBT_EA_DIRECT_ADDR && instr->size == RBT_SIZE_BYTE) {
			_push_warn(
				"SUB: Dest EA(An) cannot be byte-sized, at: 0x%06x", instr->start_pc
			);
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}
	} else {
		target_ea = &instr->src;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = _OP_REG(opcode);
	}

	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	if (!_validate_ea(target_ea, ea_invalid, "SUB", "Target", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// EOR:  1011 DDD1 SS MMMRRR [BWL]
// CMPM: 1011 AAA1 SS 001RRR [BWL]
// CMP:  1011 DDD0 SS MMMRRR [BWL]
// CMPA: 1011 AAAS 11 MMMRRR [.WL]
static u8 _decode_cmp_eor(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);
	u8 size = _OP_SIZE(opcode);

	if (RBT_BIT(opcode, 8) && size != 0b11) {
		RBT_OperandSize op_size = _decode_size(size);

		if (ea_mode == 0b001) {
			instr->mnemonic = RBT_OP_CMPM;
			instr->size = op_size;

			u8 reg_y = _OP_EA_REG(opcode); // Source
			u8 reg_x = _OP_REG(opcode);	   // Dest

			instr->src.mode = RBT_EA_INDIRECT_POSTINC;
			instr->src.indirect = reg_y;

			instr->dst.mode = RBT_EA_INDIRECT_POSTINC;
			instr->dst.indirect = reg_x;

			return RBT_ERR_SUCCESS;
		}

		instr->mnemonic = RBT_OP_EOR;
		instr->size = op_size;

		instr->src.mode = RBT_EA_DIRECT_DATA;
		instr->src.reg = _OP_REG(opcode);

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		// EA invalid: An, PC-relative, #imm
		u16 ea_invalid = RBT_EA_DIRECT_ADDR | RBT_EA_GROUP_PCR | RBT_EA_IMMEDIATE;
		if (!_validate_ea(&instr->dst, ea_invalid, "EOR", "Dest", instr->start_pc)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	if (size == 0b11) {
		instr->mnemonic = RBT_OP_CMPA;
		instr->size = RBT_BIT(opcode, 8) ? RBT_SIZE_LONG : RBT_SIZE_WORD;
	} else {
		instr->mnemonic = RBT_OP_CMP;
		instr->size = _decode_size(size);
		if (instr->size == RBT_SIZE_NONE) {
			_push_warn("CMP: Invalid operand size at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}
	}

	curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);
	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	if (instr->mnemonic == RBT_OP_CMPA) {
		instr->dst.mode = RBT_EA_DIRECT_ADDR;
		instr->dst.reg = _OP_REG(opcode);
	} else {
		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = _OP_REG(opcode);
	}

	if (instr->src.mode == RBT_EA_DIRECT_ADDR && instr->size == RBT_SIZE_BYTE) {
		_push_warn(
			"CMP: Source EA(An) cannot be byte-sized, at: 0x%06x", instr->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// MULU: 1100 DDD0 11 MMMRRR [.W.]
// MULS: 1100 DDD1 11 MMMRRR [.W.]
// ABCD: 1100 RRR 10000 mRRR [B..]
// EXG:  1100 RRR1 ooooo RRR [..L]
// AND:  1100 DDDd SS MMMRRR [BWL]
static u8 _decode_and_mul(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);
	u8 dreg = _OP_REG(opcode);
	u8 size = _OP_SIZE(opcode);

	if (size == 0b11) {
		instr->mnemonic = RBT_BIT(opcode, 8) ? RBT_OP_MULS : RBT_OP_MULU;
		instr->size = RBT_SIZE_WORD;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = dreg;

		// EA invalid: An
		u16 ea_invalid = RBT_EA_DIRECT_ADDR;
		if (!_validate_ea(
				&instr->src, ea_invalid, "MULU/MULS", "Source", instr->start_pc
			)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	if (rbt_bits(opcode, 8, 4) == 0b10000) {
		instr->mnemonic = RBT_OP_ABCD;
		instr->size = RBT_SIZE_BYTE;

		u8 reg_y = _OP_EA_REG(opcode); // Source
		u8 reg_x = _OP_REG(opcode);	   // Dest

		// 1: -(Ay)->-(Ax); 0: Dy->Dx
		if (RBT_BIT(opcode, 3)) {
			instr->src.mode = RBT_EA_INDIRECT_PREDEC;
			instr->src.indirect = reg_y;

			instr->dst.mode = RBT_EA_INDIRECT_PREDEC;
			instr->dst.indirect = reg_x;
		} else {
			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->src.reg = reg_y;

			instr->dst.mode = RBT_EA_DIRECT_DATA;
			instr->dst.reg = reg_x;
		}

		return RBT_ERR_SUCCESS;
	}

	u8 op = rbt_bits(opcode, 7, 3);
	if (RBT_BIT(opcode, 8) && (op == 0b01000 || op == 0b01001 || op == 0b10001)) {
		instr->mnemonic = RBT_OP_EXG;
		instr->size = RBT_SIZE_LONG;

		instr->src.reg = _OP_REGX(opcode);
		instr->dst.reg = _OP_REGY(opcode);

		// 0b01000: Dy <-> Dx
		// 0b01001: Ay <-> Ax
		// 0b10001: Ay <-> Dx
		if (op == 0b01000) {
			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->dst.mode = RBT_EA_DIRECT_DATA;
		} else if (op == 0b01001) {
			instr->src.mode = RBT_EA_DIRECT_ADDR;
			instr->dst.mode = RBT_EA_DIRECT_ADDR;
		} else if (op == 0b10001) {
			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->dst.mode = RBT_EA_DIRECT_ADDR;
		}

		return RBT_ERR_SUCCESS;
	}

	instr->mnemonic = RBT_OP_AND;
	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("AND: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	// EA invalid: An, [Dn, PC-rel, #imm](if <ea> is dest)
	u16 ea_invalid = RBT_EA_DIRECT_ADDR;
	RBT_EffectiveAddress *target_ea;

	// 1: <ea> and Dn -> <ea>
	// 0: Dn and <ea> -> Dn
	if (RBT_BIT(opcode, 8)) {
		target_ea = &instr->dst;
		ea_invalid |= RBT_EA_DIRECT_DATA | RBT_EA_GROUP_PCR | RBT_EA_IMMEDIATE;

		instr->src.mode = RBT_EA_DIRECT_DATA;
		instr->src.reg = dreg;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
	} else {
		target_ea = &instr->src;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = dreg;
	}

	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	if (!_validate_ea(target_ea, ea_invalid, "AND", "Target", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// ADD:  1101 DDDd SS MMMRRR [BWL]
// ADDX: 1101 RRR1 SS 00mRRR [BWL]
// ADDA: 1101 AAAs 11 MMMRRR [.WL]
static u8 _decode_addaddx(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 ea_mode = _OP_EA_MODE(opcode);
	u8 ea_reg = _OP_EA_REG(opcode);
	u8 size = _OP_SIZE(opcode);

	if (size == 0b11) {
		instr->mnemonic = RBT_OP_ADDA;
		instr->size = RBT_BIT(opcode, 8) ? RBT_SIZE_LONG : RBT_SIZE_WORD;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->src);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		instr->dst.mode = RBT_EA_DIRECT_ADDR;
		instr->dst.reg = _OP_REG(opcode);

		return RBT_ERR_SUCCESS;
	}

	if (RBT_BIT(opcode, 8) && rbt_bits(opcode, 5, 4) == 0b00) {
		instr->mnemonic = RBT_OP_ADDX;
		instr->size = _decode_size(size);
		if (instr->size == RBT_SIZE_NONE) {
			_push_warn("ADDX: Invalid operand size at: 0x%06x", instr->start_pc);
			return RBT_ERR_DECODE_ILLEGAL;
		}

		u8 reg_y = _OP_EA_REG(opcode); // Source
		u8 reg_x = _OP_REG(opcode);	   // Dest

		// 1: -(Ay)->-(Ax); 0: Dy->Dx
		if (RBT_BIT(opcode, 3)) {
			instr->src.mode = RBT_EA_INDIRECT_PREDEC;
			instr->src.indirect = reg_y;

			instr->dst.mode = RBT_EA_INDIRECT_PREDEC;
			instr->dst.indirect = reg_x;
		} else {
			instr->src.mode = RBT_EA_DIRECT_DATA;
			instr->src.reg = reg_y;

			instr->dst.mode = RBT_EA_DIRECT_DATA;
			instr->dst.reg = reg_x;
		}

		return RBT_ERR_SUCCESS;
	}

	instr->mnemonic = RBT_OP_ADD;
	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("ADD: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	// EA invalid: [Dn, An, PC-relative, #imm](if <ea> is dest)
	u16 ea_invalid = 0;
	RBT_EffectiveAddress *target_ea;

	// 1: <ea> + Dn -> <ea>
	// 0: Dn + <ea> -> Dn
	if (RBT_BIT(opcode, 8)) {
		target_ea = &instr->dst;
		ea_invalid = RBT_EA_DIRECT_DATA | RBT_EA_DIRECT_ADDR | RBT_EA_GROUP_PCR
				   | RBT_EA_IMMEDIATE;

		instr->src.mode = RBT_EA_DIRECT_DATA;
		instr->src.reg = _OP_REG(opcode);

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);

		if (instr->dst.mode == RBT_EA_DIRECT_ADDR && instr->size == RBT_SIZE_BYTE) {
			_push_warn(
				"ADD: Dest EA(An) cannot be byte-sized, at: 0x%06x", instr->start_pc
			);
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}
	} else {
		target_ea = &instr->src;

		instr->src.size = instr->size;
		curr_pc = _ea_decode(ea_mode, ea_reg, instr->src.size, bus, curr_pc, &instr->src);

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = _OP_REG(opcode);
	}

	if (curr_pc == UINT32_MAX) {
		return RBT_ERR_DECODE_INVALID_EA;
	}

	if (!_validate_ea(target_ea, ea_invalid, "ADD", "Target", instr->start_pc)) {
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

// ASd:  1110 000d 11 MMMRRR [.W.]
// LSd:  1110 001d 11 MMMRRR [.W.]
// ROXd: 1110 010d 11 MMMRRR [.W.]
// ROd:  1110 011d 11 MMMRRR [.W.]
// ASd:  1110 rrrd SS m00DDD [BWL]
// LSd:  1110 rrrd SS m01DDD [BWL]
// ROXd: 1110 rrrd SS m10DDD [BWL]
// ROd:  1110 rrrd SS m11DDD [BWL]
static u8 _decode_shift(RBT_Instruction *instr, RBT_MemoryBus *bus) {
	u16 opcode = instr->words[0];
	u32 curr_pc = instr->start_pc + 2;

	u8 size = _OP_SIZE(opcode);

	// Is memory shift?
	if (size == 0b11) {
		u8 ea_mode = _OP_EA_MODE(opcode);
		u8 ea_reg = _OP_EA_REG(opcode);

		switch (rbt_bits(opcode, 11, 8)) {
		case 0b0000: instr->mnemonic = RBT_OP_ASR; break;
		case 0b0001: instr->mnemonic = RBT_OP_ASL; break;
		case 0b0010: instr->mnemonic = RBT_OP_LSR; break;
		case 0b0011: instr->mnemonic = RBT_OP_LSL; break;
		case 0b0100: instr->mnemonic = RBT_OP_ROXR; break;
		case 0b0101: instr->mnemonic = RBT_OP_ROXL; break;
		case 0b0110: instr->mnemonic = RBT_OP_ROR; break;
		case 0b0111: instr->mnemonic = RBT_OP_ROL; break;
		default:
			_push_warn(
				"SHIFT: Illegal decoding for memory shift, at: 0x%06x", instr->start_pc
			);
			return RBT_ERR_DECODE_ILLEGAL;
		}

		instr->size = RBT_SIZE_WORD;

		curr_pc = _ea_decode(ea_mode, ea_reg, instr->size, bus, curr_pc, &instr->dst);
		if (curr_pc == UINT32_MAX) {
			return RBT_ERR_DECODE_INVALID_EA;
		}

		// EA invalid: Dn, An, PC-relative, #imm
		u16 ea_invalid = RBT_EA_DIRECT_DATA | RBT_EA_DIRECT_ADDR | RBT_EA_IMMEDIATE
					   | RBT_EA_GROUP_PCR;
		if (!_validate_ea(&instr->dst, ea_invalid, "SHIFT", "Dest", instr->start_pc)) {
			return RBT_ERR_DECODE_ILLEGAL_EA;
		}

		return RBT_ERR_SUCCESS;
	}

	bool dir = RBT_BIT(opcode, 8) == 1u; // 0: Right; 1: Left
	switch (rbt_bits(opcode, 4, 3)) {
	case 0b00: instr->mnemonic = dir ? RBT_OP_ASL : RBT_OP_ASR; break;
	case 0b01: instr->mnemonic = dir ? RBT_OP_LSL : RBT_OP_LSR; break;
	case 0b10: instr->mnemonic = dir ? RBT_OP_ROXL : RBT_OP_ROXR; break;
	case 0b11: instr->mnemonic = dir ? RBT_OP_ROL : RBT_OP_ROR; break;
	default:   unreachable();
	}

	instr->size = _decode_size(size);
	if (instr->size == RBT_SIZE_NONE) {
		_push_warn("SHIFT: Invalid operand size at: 0x%06x", instr->start_pc);
		return RBT_ERR_DECODE_ILLEGAL;
	}

	instr->dst.mode = RBT_EA_DIRECT_DATA;
	instr->dst.reg = _OP_REGY(opcode);

	// 1: Dn; 0: #imm
	if (RBT_BIT(opcode, 5)) {
		instr->src.mode = RBT_EA_DIRECT_DATA;
		instr->src.reg = _OP_REG(opcode);
	} else {
		instr->src.mode = RBT_EA_IMMEDIATE;
		instr->src.size = RBT_SIZE_NONE;
		instr->src.imm = _OP_SHIFT(opcode);
	}

	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode _decode_instruction(RBT_MemoryBus *bus, u32 pc, RBT_Instruction *instr) {
	assert(bus);
	assert(instr);

	memset(instr, 0, sizeof(RBT_Instruction));
	instr->start_pc = pc & 0xff'ffff;
	instr->word_count = 1;

	if (rbt_bus_read_word(bus, instr->start_pc, &instr->words[0])) {
		_push_error(RBT_ERR_MEM_BUS_ERROR, "Failed to fetch instruction word");
		return RBT_ERR_MEM_BUS_ERROR;
	}

	u16 opcode = instr->words[0];
	RBT_OpGroup group = _OP_GROUP(opcode);
	RBT_ErrorCode status = RBT_ERR_SUCCESS;

	switch (group) {
	case _OPGROUP_BITMOVEPIMM: {
		u8 subgroup = _OP_SUBGROUP(opcode);

		if (subgroup == 0x0e) {
			// MOVES: 0000 1110 SS MMMRRR [BWL] (M68010+)
			status = _decode_moves_movep(instr, bus);
			break;
		}

		if (subgroup == 0x08) {
			// Static BTST/BCHG/BCLR/BSET: 0000 1000 TT MMMRRR [B.L]
			status = _decode_bit(instr, bus);
			break;
		}

		if (RBT_BIT(opcode, 8)) {
			// We can check here, since An is an invalid EA mode in next opcodes
			if (_OP_EA_MODE(opcode) == 0b001) {
				// MOVEP: 0000 DDD1 OO 001RRR [.WL]
				status = _decode_moves_movep(instr, bus);
				break;
			}

			// Dynamic BTST/BCHG/BCLR/BSET: 0000 DDD1 TT MMMRRR [B.L]
			status = _decode_bit(instr, bus);
			break;
		}

		// ORI/ANDI/SUBI/ADDI/EORI/CMPI: 0000 TTT0 SS MMMRRR [BWL]
		status = _decode_imm(instr, bus);
	} break;
	case _OPGROUP_MOVEBYTE:
	case _OPGROUP_MOVELONG:
	case _OPGROUP_MOVEWORD:
		// MOVE:  00SS RRRMMM MMMRRR [BWL]
		// MOVEA: 00SS RRR001 MMMRRR [.WL]
		status = _decode_move_movea(instr, bus);
		break;
	case _OPGROUP_MISC: {
		u8 subgroup = _OP_SUBGROUP(opcode);
		u8 subtype = rbt_bits(opcode, 8, 6);

		// CHK: 0100 DDD 110 MMMRRR [.W.]
		// LEA: 0100 AAA 111 MMMRRR [..L]
		if (subtype == 0b110 || subtype == 0b111) {
			status = _decode_chk_lea(instr, bus);
			break;
		}

		// MOVE fr SR:  0100 000 011 MMMRRR [.W.]
		// MOVE fr CCR: 0100 001 011 MMMRRR [.W.]
		// MOVE to CCR: 0100 010 011 MMMRRR [.W.]
		// MOVE to SR:  0100 011 011 MMMRRR [.W.]
		if ((!RBT_BIT(opcode, 11) && subtype == 0b011)) {
			status = _decode_move_reg(instr, bus);
			break;
		}

		// MOVEM: 0100 1d001s MMMRRR [.WL]
		//        Register List Mask
		if (RBT_BIT(opcode, 11) && rbt_bits(opcode, 9, 7) == 0b001
			&& _OP_EA_MODE(opcode) != 0b000) {
			status = _decode_movem(instr, bus);
			break;
		}

		// EXT:   0100 100 ooo 000DDD [.WL]
		// NBCD:  0100 100 000 MMMRRR [B..]
		// SWAP:  0100 100 001 000RRR [.W.]
		// BKPT:  0100 100 001 001NNN [...] (M68010+)
		// PEA:   0100 100 001 MMMRRR [..L]
		if (rbt_bits(opcode, 11, 9) == 0b100) {
			status = _decode_ext_nbcd_swap_bkpt_pea(instr, bus);
			break;
		}

		// ILLEGAL: 0100 1010 11111100 [...]
		// TAS:     0100 1010 11MMMRRR [B..]
		// TST:     0100 1010 SSMMMRRR [BWL]
		if (subgroup == 0b1010) {
			status = _decode_illegal_tas_tst(instr, bus);
			break;
		}

		// JSR:      0100 1110 10 MMMRRR [...]
		// JMP:      0100 1110 11 MMMRRR [...]
		// TRAP:     0100 1110 0100 VVVV [...]
		// UNLK:     0100 1110 0101 1RRR [...]
		// LINK:     0100 1110 0101 0RRR [.W.]
		//           Offset
		// MOVE USP: 0100 1110 0110 dRRR [..L]
		// RESET:    0100 1110 0111 0000 [...]
		// NOP:      0100 1110 0111 0001 [...]
		// STOP:     0100 1110 0111 0010 [...]
		//           Immediate
		// RTE:      0100 1110 0111 0011 [...]
		// RTD:      0100 1110 0111 0100 [...] (M68010+)
		//           Offset
		// RTS:      0100 1110 0111 0101 [...]
		// TRAPV:    0100 1110 0111 0110 [...]
		// RTR:      0100 1110 0111 0111 [...]
		// MOVEC:    0100 1110 0111 101d [..L] (M68010+)
		//           ARRR CTRL_REGISTER
		if (subgroup == 0b1110) {
			status = _decode_misc(instr, bus);
			break;
		}

		// NEGX/CLR/NEG/NOT: 0100 TTT0 SS MMMRRR [BWL]
		status = _decode_negx_clr_not(instr, bus);
	} break;
	case _OPGROUP_ADDQSUBQ:
		// ADDQ: 0101 QQQ0 SS MMMRRR [BWL]
		// SUBQ: 0101 QQQ1 SS MMMRRR [BWL]
		// Scc:  0101 cccc 11 MMMRRR [B..]
		// DBcc: 0101 cccc 11 001RRR [.W.]
		status = _decode_addq_subq(instr, bus);
		break;
	case _OPGROUP_BRANCH:
		// BRA: 0110 0000 dddddddd [BW.]
		// BSR: 0110 0001 dddddddd [BW.]
		// Bcc: 0110 cccc dddddddd [BW.]
		status = _decode_branch(instr, bus);
		break;
	case _OPGROUP_MOVEQ: {
		// MOVEQ: 0111 RRR0 QQQQQQQQ [..L]
		u16 opcode = instr->words[0];

		if (RBT_BIT(opcode, 8)) {
			_push_error(
				RBT_ERR_DECODE_ILLEGAL, "MOVEQ: Unknown encoding at 0x%06x",
				instr->start_pc
			);
			status = RBT_ERR_DECODE_ILLEGAL;
			break;
		}

		u8 quick = _OP_MOVEQ_QUICK(opcode);
		u8 dreg = _OP_REG(opcode);

		instr->mnemonic = RBT_OP_MOVEQ;
		instr->size = RBT_SIZE_LONG;

		instr->src.mode = RBT_EA_IMMEDIATE;
		instr->src.size = RBT_SIZE_NONE;
		instr->src.imm = quick;

		instr->dst.mode = RBT_EA_DIRECT_DATA;
		instr->dst.reg = dreg;

		status = RBT_ERR_SUCCESS;
	} break;
	case _OPGROUP_ORDIV:
		// DIVU: 1000 RRR0 11 MMMRRR [.W.]
		// DIVS: 1000 RRR1 11 MMMRRR [.W.]
		// SBCD: 1000 RRR 10000 mRRR [B..]
		// OR:   1000 RRRd SS MMMRRR [BWL]
		status = _decode_ordiv(instr, bus);
		break;
	case _OPGROUP_SUBSUBX:
		// SUB:  1001 DDDd SS MMMRRR [BWL]
		// SUBX: 1001 RRR1 SS 00mRRR [BWL]
		// SUBA: 1001 AAAs 11 MMMRRR [.WL]
		status = _decode_subsubx(instr, bus);
		break;
	case _OPGROUP_LINEA:
		instr->mnemonic = RBT_OP_LINEA;
		instr->size = RBT_SIZE_NONE;
		break;
	case _OPGROUP_CMPEOR:
		// EOR:  1011 DDD1 SS MMMRRR [BWL]
		// CMPM: 1011 AAA1 SS 001RRR [BWL]
		// CMP:  1011 DDD0 SS MMMRRR [BWL]
		// CMPA: 1011 AAAS 11 MMMRRR [.WL]
		status = _decode_cmp_eor(instr, bus);
		break;
	case _OPGROUP_ANDMUL:
		// MULU: 1100 DDD0 11 MMMRRR [.W.]
		// MULS: 1100 DDD1 11 MMMRRR [.W.]
		// ABCD: 1100 RRR 10000 mRRR [B..]
		// EXG:  1100 RRR1 ooooo RRR [..L]
		// AND:  1100 DDDd SS MMMRRR [BWL]
		status = _decode_and_mul(instr, bus);
		break;
	case _OPGROUP_ADDADDX:
		// ADD:  1101 DDDd SS MMMRRR [BWL]
		// ADDX: 1101 RRR1 SS 00mRRR [BWL]
		// ADDA: 1101 AAAs 11 MMMRRR [.WL]
		status = _decode_addaddx(instr, bus);
		break;
	case _OPGROUP_SHIFT:
		// ASd:  1110 000d 11 MMMRRR [.W.]
		// LSd:  1110 001d 11 MMMRRR [.W.]
		// ROXd: 1110 010d 11 MMMRRR [.W.]
		// ROd:  1110 011d 11 MMMRRR [.W.]
		// ASd:  1110 rrrd SS m00DDD [BWL]
		// LSd:  1110 rrrd SS m01DDD [BWL]
		// ROXd: 1110 rrrd SS m10DDD [BWL]
		// ROd:  1110 rrrd SS m11DDD [BWL]
		status = _decode_shift(instr, bus);
		break;
	case _OPGROUP_LINEF:
		instr->mnemonic = RBT_OP_LINEF;
		instr->size = RBT_SIZE_NONE;
		break;
	default: unreachable();
	}

	instr->word_count += _store_operand_as_words(
		&instr->aux, &instr->words[instr->word_count]
	);
	instr->word_count += _store_operand_as_words(
		&instr->src, &instr->words[instr->word_count]
	);
	instr->word_count += _store_operand_as_words(
		&instr->dst, &instr->words[instr->word_count]
	);

	instr->len = instr->word_count * 2; // length is stored as bytes
	return status;
}
