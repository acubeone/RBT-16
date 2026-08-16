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

#include "cpu/effective_address.h"

#include "cpu/bus_internal.h"
#include "cpu/cpu_internal.h"
#include "error.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/bus.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"
#include "rbt/helpers.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

bool _indexext_from_word(u16 ext, RBT_IndexExtension *ix) {
	assert(ix);

	// Brief Extension Word
	//  F  | E D C |  B  |  A 9  | 8 | 7 6 5 4 3 2 1 0
	// A/D |  REG  | W/L | SCALE | 0 |  DISPLACEMENT

	if (rbt_bits(ext, 10, 9) != 0) {
		_push_error(RBT_ERR_DECODE_INVALID_EA, "Extension word's scale bit is set");
		return false;
	}

	ix->is_addr = RBT_BIT(ext, 15);
	ix->is_long = RBT_BIT(ext, 11);
	ix->xreg = rbt_bits(ext, 14, 12);
	ix->disp = rbt_sign_extend(RBT_SIZE_BYTE, ext & 0xff);

	return true;
}

u16 _indexext_to_word(const RBT_IndexExtension *ix) {
	assert(ix);

	u16 word = 0;

	if (ix->is_addr)
		word |= 1 << 15;
	if (ix->is_long)
		word |= 1 << 11;

	word |= ((u16)ix->xreg) << 12;
	word |= ix->disp & 0xff;

	return word;
}

u32 _ea_decode(
	u8 mode,
	u8 reg,
	RBT_OperandSize size,
	RBT_MemoryBus *bus,
	u32 pc,
	RBT_EffectiveAddress *ea
) {
	assert(bus);
	assert(ea);

	memset(ea, 0, sizeof(RBT_EffectiveAddress));
	ea->start_pc = pc;
	ea->size = size;

	mode &= 0x07;
	reg &= 0x07;

	u32 bytes = 0;
	switch (mode) {
	case 0b000: // Dn
		ea->mode = RBT_EA_DIRECT_DATA;
		ea->reg = reg;
		break;
	case 0b001: // An
		ea->mode = RBT_EA_DIRECT_ADDR;
		ea->reg = reg;
		break;
	case 0b010: // (An)
		ea->mode = RBT_EA_INDIRECT;
		ea->indirect = reg;
		break;
	case 0b011: // (An)+
		ea->mode = RBT_EA_INDIRECT_POSTINC;
		ea->indirect = reg;
		break;
	case 0b100: // -(An)
		ea->mode = RBT_EA_INDIRECT_PREDEC;
		ea->indirect = reg;
		break;
	case 0b101: { // (d16, An)

		u16 disp;
		if (rbt_bus_read_word(bus, pc, &disp)) {
			goto decoding_error;
		}
		bytes = 2;

		ea->mode = RBT_EA_INDIRECT_DISPLACEMENT;
		ea->ind_disp.areg = reg;
		ea->ind_disp.disp = rbt_sign_extend(RBT_SIZE_WORD, disp);
	} break;
	case 0b110: { // (d8, Xi, An)
		u16 ext;
		if (rbt_bus_read_word(bus, pc, &ext)) {
			goto decoding_error;
		}
		bytes = 2;

		ea->mode = RBT_EA_INDIRECT_INDEXED;
		ea->ind_idx.areg = reg;
		if (!_indexext_from_word(ext, &ea->ind_idx.ix)) {
			goto decoding_error;
		}
	} break;
	case 0b111:
		switch (reg) {
		case 0b000: { // (xxx).w
			u16 abs;
			if (rbt_bus_read_word(bus, pc, &abs)) {
				goto decoding_error;
			}
			bytes = 2;

			ea->mode = RBT_EA_ABSOLUTE_SHORT;
			ea->absolute_short = rbt_sign_extend(RBT_SIZE_WORD, abs);
		} break;
		case 0b001: { // (xxx).l
			u32 abs;
			if (rbt_bus_read_long(bus, pc, &abs)) {
				goto decoding_error;
			}
			bytes = 4;

			ea->mode = RBT_EA_ABSOLUTE_LONG;
			ea->absolute_long = abs;
		} break;
		case 0b010: { // (d16, PC)
			u16 disp;
			if (rbt_bus_read_word(bus, pc, &disp)) {
				goto decoding_error;
			}
			bytes = 2;

			ea->mode = RBT_EA_PC_DISPLACEMENT;
			ea->pc_disp = rbt_sign_extend(RBT_SIZE_WORD, disp);
		} break;
		case 0b011: { // (d8, Xi, PC)
			u16 ext;
			if (rbt_bus_read_word(bus, pc, &ext)) {
				goto decoding_error;
			}
			bytes = 2;

			ea->mode = RBT_EA_PC_INDEXED;
			if (!_indexext_from_word(ext, &ea->pc_idx)) {
				goto decoding_error;
			}
		} break;
		case 0b100: { // #imm
			ea->mode = RBT_EA_IMMEDIATE;

			if (_bus_fetch_imm(bus, size, pc, &ea->imm)) {
				goto decoding_error;
			}

			bytes = (size == RBT_SIZE_LONG) ? 4 : 2;
		} break;
		default: goto decoding_error;
		}
	}

	return ea->start_pc + bytes;

decoding_error:
	_push_error(
		RBT_ERR_DECODE_INVALID_EA, "Failed to decode effective address at: 0x%06x", pc
	);
	return UINT32_MAX;
}

u32 _ea_compute_address(const RBT_EffectiveAddress *ea, RBT_Cpu *cpu) {
	switch (ea->mode) {
	case RBT_EA_INDIRECT: //
		return cpu->state.gpr.addr[ea->indirect];
	case RBT_EA_INDIRECT_DISPLACEMENT:
		return cpu->state.gpr.addr[ea->ind_disp.areg] + ea->ind_disp.disp;
	case RBT_EA_INDIRECT_INDEXED: {
		const RBT_IndexExtension *ix = &ea->ind_idx.ix;
		u32 xreg;
		RBT_OperandSize ix_size = ix->is_long ? RBT_SIZE_LONG : RBT_SIZE_WORD;
		if (ix->is_addr)
			xreg = cpu->state.gpr.addr[ix->xreg];
		else
			xreg = cpu->state.gpr.data[ix->xreg];

		return cpu->state.gpr.addr[ea->ind_idx.areg] //
			 + ix->disp								 //
			 + rbt_sign_extend(ix_size, xreg);
	}
	case RBT_EA_ABSOLUTE_SHORT: //
		return rbt_sign_extend(RBT_SIZE_WORD, ea->absolute_short);
	case RBT_EA_ABSOLUTE_LONG: //
		return ea->absolute_long;
	case RBT_EA_PC_DISPLACEMENT: //
		return ea->start_pc + ea->pc_disp;
	case RBT_EA_PC_INDEXED: {
		const RBT_IndexExtension *ix = &ea->pc_idx;
		u32 xreg;
		RBT_OperandSize ix_size = ix->is_long ? RBT_SIZE_LONG : RBT_SIZE_WORD;
		if (ix->is_addr)
			xreg = cpu->state.gpr.addr[ix->xreg];
		else
			xreg = cpu->state.gpr.data[ix->xreg];

		return ea->start_pc //
			 + ix->disp		//
			 + rbt_sign_extend(ix_size, xreg);
	}
	default: //
		return 0;
	}

	unreachable();
}

RBT_ErrorCode _ea_read(
	const RBT_EffectiveAddress *ea, RBT_OperandSize size, RBT_Cpu *cpu, u32 *out
) {
	assert(ea);
	assert(cpu);
	assert(out);

	switch (ea->mode) {
	case RBT_EA_DIRECT_DATA: //
		*out = rbt_truncate(size, cpu->state.gpr.data[ea->reg]);
		break;
	case RBT_EA_DIRECT_ADDR: //
		*out = cpu->state.gpr.addr[ea->reg];
		break;
	case RBT_EA_INDIRECT_POSTINC: {
		u32 addr = cpu->state.gpr.addr[ea->indirect];

		u32 value;
		RBT_ErrorCode err = rbt_bus_load(cpu->bus, size, addr, &value);
		if (err)
			return err;

		*out = value;

		if (ea->indirect == 7 && size == RBT_SIZE_BYTE)
			cpu->state.gpr.addr[ea->indirect] += 2;
		else
			cpu->state.gpr.addr[ea->indirect] += (u32)size;
	} break;
	case RBT_EA_INDIRECT_PREDEC: {
		if (ea->indirect == 7 && size == RBT_SIZE_BYTE)
			cpu->state.gpr.addr[ea->indirect] -= 2;
		else
			cpu->state.gpr.addr[ea->indirect] -= (u32)size;

		u32 addr = cpu->state.gpr.addr[ea->indirect];
		u32 value;
		RBT_ErrorCode err = rbt_bus_load(cpu->bus, size, addr, &value);
		if (err)
			return err;

		*out = value;
	} break;
	case RBT_EA_INDIRECT:
	case RBT_EA_INDIRECT_DISPLACEMENT:
	case RBT_EA_INDIRECT_INDEXED:
	case RBT_EA_ABSOLUTE_SHORT:
	case RBT_EA_ABSOLUTE_LONG:
	case RBT_EA_PC_DISPLACEMENT:
	case RBT_EA_PC_INDEXED:			   {
		u32 addr = _ea_compute_address(ea, cpu);
		RBT_ErrorCode err = rbt_bus_load(cpu->bus, size, addr, out);
		if (err)
			return err;
	} break;
	case RBT_EA_IMMEDIATE: //
		*out = ea->imm;
		break;
	case RBT_EA_DISPLACEMENT: //
		*out = (u32)ea->disp;
		break;
	case RBT_EA_REGISTER_SR: //
		*out = _pack_status_register(&cpu->state.sr);
		break;
	case RBT_EA_REGISTER_CCR: //
		// Only read lower byte from status register
		*out = _pack_status_register(&cpu->state.sr) & 0xff;
		break;
	case RBT_EA_REGISTER_USP: //
		*out = cpu->state.usp;
		break;
	case RBT_EA_REGISTER_DFC: //
		*out = cpu->state.dfc;
		break;
	case RBT_EA_REGISTER_SFC: //
		*out = cpu->state.sfc;
		break;
	case RBT_EA_REGISTER_VBR: //
		*out = cpu->state.vbr;
		break;
	default: //
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode _ea_write(
	const RBT_EffectiveAddress *ea, RBT_OperandSize size, RBT_Cpu *cpu, u32 in
) {
	assert(ea);
	assert(cpu);

	switch (ea->mode) {
	case RBT_EA_DIRECT_DATA: {
		u32 *dreg = &cpu->state.gpr.data[ea->reg];
		*dreg = rbt_store_sized(size, *dreg, in);
	} break;
	case RBT_EA_DIRECT_ADDR: //
		cpu->state.gpr.addr[ea->reg] = in;
		break;
	case RBT_EA_INDIRECT_POSTINC: {
		u32 addr = cpu->state.gpr.addr[ea->indirect];

		RBT_ErrorCode err = rbt_bus_store(cpu->bus, size, addr, in);
		if (err)
			return err;

		if (ea->indirect == 7 && size == RBT_SIZE_BYTE)
			cpu->state.gpr.addr[ea->indirect] += 2;
		else
			cpu->state.gpr.addr[ea->indirect] += (u32)size;
	} break;
	case RBT_EA_INDIRECT_PREDEC: {
		if (ea->indirect == 7 && size == RBT_SIZE_BYTE)
			cpu->state.gpr.addr[ea->indirect] -= 2;
		else
			cpu->state.gpr.addr[ea->indirect] -= (u32)size;

		u32 addr = cpu->state.gpr.addr[ea->indirect];
		RBT_ErrorCode err = rbt_bus_store(cpu->bus, size, addr, in);
		if (err)
			return err;
	} break;
	case RBT_EA_INDIRECT:
	case RBT_EA_INDIRECT_DISPLACEMENT:
	case RBT_EA_INDIRECT_INDEXED:	   {
		u32 addr = _ea_compute_address(ea, cpu);
		RBT_ErrorCode err = rbt_bus_store(cpu->bus, size, addr, in);
		if (err)
			return err;
	} break;
	case RBT_EA_ABSOLUTE_SHORT:
	case RBT_EA_ABSOLUTE_LONG:
		_push_error(
			RBT_ERR_DECODE_ILLEGAL_EA,
			"EA: Absolute address is not alterable, at: 0x%06x", ea->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL_EA;
	case RBT_EA_PC_DISPLACEMENT:
	case RBT_EA_PC_INDEXED:
		_push_error(
			RBT_ERR_DECODE_ILLEGAL_EA, "EA: PC-relative is not alterable, at: 0x%06x",
			ea->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL_EA;
	case RBT_EA_IMMEDIATE: //
		_push_error(
			RBT_ERR_DECODE_ILLEGAL_EA, "EA: Immediate is not alterable, at: 0x%06x",
			ea->start_pc
		);
		return RBT_ERR_DECODE_ILLEGAL_EA;
	case RBT_EA_DISPLACEMENT: //
		return RBT_ERR_DECODE_ILLEGAL_EA;
	case RBT_EA_REGISTER_SR: //
		_unpack_status_register(&cpu->state.sr, (u16)in);
		break;
	case RBT_EA_REGISTER_CCR: {
		// Keep high byte from status register, only modify lower byte
		u16 sr = _pack_status_register(&cpu->state.sr) & 0xff00;
		u16 ccr = in & 0x00ff;

		_unpack_status_register(&cpu->state.sr, sr | ccr);
	} break;
	case RBT_EA_REGISTER_USP: //
		cpu->state.usp = in;
		break;
	case RBT_EA_REGISTER_DFC: //
		cpu->state.dfc = in;
		break;
	case RBT_EA_REGISTER_SFC: //
		cpu->state.sfc = in;
		break;
	case RBT_EA_REGISTER_VBR: //
		cpu->state.vbr = in;
		break;
	default: //
		return RBT_ERR_DECODE_ILLEGAL_EA;
	}

	return RBT_ERR_SUCCESS;
}
