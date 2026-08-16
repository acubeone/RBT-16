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
#include "rbt/basic_types.h"
#include "rbt/cpu/bus.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"

#include <unity.h>

// ----------------------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------------------

static RBT_MemoryBus *bus;

// Each test loads its own bytes into ROM and decodes from _BUS_ROM_ADDR.
// Tests are fully independent — no shared mutable pc.
static void _load(const u8 *bytes, usize len) {
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_init(bus, len, bytes));
}

static RBT_Instruction _decode(void) {
	RBT_Instruction instr;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, _decode_instruction(bus, _BUS_ROM_ADDR, &instr));
	return instr;
}

void setUp(void) {
	RBT_BusConfig cfg = {
		.ram_slots = { RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE },
	};
	bus = rbt_create_bus(&cfg);
	TEST_ASSERT_NOT_NULL(bus);
}

void tearDown(void) {
	rbt_destroy_bus(bus);
	rbt_err_flush();
}

// ----------------------------------------------------------------------------
// Group 0000 — Bit/MOVEP/Immediate
// ----------------------------------------------------------------------------

// BTST #5, (A0) — static bit test, byte, indirect
// 0000 1000 00 010 000 | 0x0005
void test_decode_btst_static_indirect(void) {
	_load((u8[]) { 0x08, 0x10, 0x00, 0x05 }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_BTST, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_BYTE, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(5, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_INDIRECT, i.dst.mode);
	TEST_ASSERT_EQUAL(0, i.dst.indirect); // A0

	TEST_ASSERT_EQUAL(2, i.word_count);
	TEST_ASSERT_EQUAL(4, i.len);
}

// BTST D3, (#5, A1) — dynamic bit test, byte, indirect displacement
// 0000 011 1 01 101 001 | 0x0005
void test_decode_btst_dynamic_ind_disp(void) {
	_load((u8[]) { 0x07, 0x29, 0x00, 0x05 }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_BTST, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_BYTE, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.src.mode);
	TEST_ASSERT_EQUAL(3, i.src.reg); // D3

	TEST_ASSERT_EQUAL(RBT_EA_INDIRECT_DISPLACEMENT, i.dst.mode);
	TEST_ASSERT_EQUAL(1, i.dst.ind_disp.areg); // A1
	TEST_ASSERT_EQUAL(5, i.dst.ind_disp.disp);

	TEST_ASSERT_EQUAL(2, i.word_count);
	TEST_ASSERT_EQUAL(4, i.len);
}

// BCHG #0, D4 — static bit change, long (Dn target)
// 0000 1000 01 000 100 | 0x0000
void test_decode_bchg_static_dn(void) {
	_load((u8[]) { 0x08, 0x44, 0x00, 0x00 }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_BCHG, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_LONG, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(0, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(4, i.dst.reg); // D4

	TEST_ASSERT_EQUAL(2, i.word_count);
}

// BSET #5, (0x0009).w — static bit set, absolute short
// 0000 1000 11 111 000 | 0x0005 | 0x0009
void test_decode_bset_static_abs_short(void) {
	_load((u8[]) { 0x08, 0xf8, 0x00, 0x05, 0x00, 0x09 }, 6);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_BSET, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_BYTE, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(5, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_ABSOLUTE_SHORT, i.dst.mode);
	TEST_ASSERT_EQUAL(9, i.dst.absolute_short);

	TEST_ASSERT_EQUAL(3, i.word_count);
	TEST_ASSERT_EQUAL(6, i.len);
}

// ANDI.w #0x9abc, (0x2442).w
// 0000 0010 01 111 000 | 0x9abc | 0x2442
void test_decode_andi_w_abs_short(void) {
	_load((u8[]) { 0x02, 0x78, 0x9a, 0xbc, 0x24, 0x42 }, 6);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_ANDI, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_WORD, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(0x9abc, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_ABSOLUTE_SHORT, i.dst.mode);
	TEST_ASSERT_EQUAL(0x2442, i.dst.absolute_short);

	TEST_ASSERT_EQUAL(3, i.word_count);
	TEST_ASSERT_EQUAL(6, i.len);
}

// ORI.b #0xff, %ccr
// 0000 0000 00 111 100 | 0x00ff
void test_decode_ori_b_ccr(void) {
	_load((u8[]) { 0x00, 0x3c, 0x00, 0xff }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_ORI, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_BYTE, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(0xff, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_REGISTER_CCR, i.dst.mode);

	TEST_ASSERT_EQUAL(2, i.word_count);
}

// CMPI.l #0xdeadbeef, D2
// 0000 1100 10 000 010 | 0xdead | 0xbeef
void test_decode_cmpi_l_dn(void) {
	_load((u8[]) { 0x0c, 0x82, 0xde, 0xad, 0xbe, 0xef }, 6);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_CMPI, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_LONG, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(0xdeadbeef, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.reg); // D2

	TEST_ASSERT_EQUAL(3, i.word_count);
}

// ----------------------------------------------------------------------------
// Group 00ss — MOVE/MOVEA
// ----------------------------------------------------------------------------

// MOVE.l #0xdeadbeef, D4
// 0010 100 000 111 100 | 0xdead | 0xbeef
void test_decode_move_l_imm_dn(void) {
	_load((u8[]) { 0x28, 0x3c, 0xde, 0xad, 0xbe, 0xef }, 6);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_MOVE, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_LONG, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(0xdeadbeef, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(4, i.dst.reg); // D4

	TEST_ASSERT_EQUAL(3, i.word_count);
	TEST_ASSERT_EQUAL(6, i.len);
}

// MOVEA.w #0xffff, A2
// 0011 010 001 111 100 | 0xffff
void test_decode_movea_w_imm(void) {
	_load((u8[]) { 0x34, 0x7c, 0xff, 0xff }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_MOVEA, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_WORD, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(0xffff, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_ADDR, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.reg); // A2

	TEST_ASSERT_EQUAL(2, i.word_count);
}

// MOVE.b D4, (A2, D4.w) — indexed indirect destination
// 0001 010 110 000 100  (size=byte, dst=mode6/reg2, src=mode0/reg4)
void test_decode_move_b_dn_ind_idx(void) {
	_load((u8[]) { 0x15, 0x84, 0x40, 0x00 }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_MOVE, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_BYTE, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.src.mode);
	TEST_ASSERT_EQUAL(4, i.src.reg); // D4

	TEST_ASSERT_EQUAL(RBT_EA_INDIRECT_INDEXED, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.ind_idx.areg);	 // A2
	TEST_ASSERT_EQUAL(4, i.dst.ind_idx.ix.xreg); // D4
	TEST_ASSERT_FALSE(i.dst.ind_idx.ix.is_addr);
	TEST_ASSERT_FALSE(i.dst.ind_idx.ix.is_long);

	TEST_ASSERT_EQUAL(2, i.word_count);
}

// ----------------------------------------------------------------------------
// Group 0100 — Miscellaneous
// ----------------------------------------------------------------------------

// NOP
// 0100 1110 0111 0001
void test_decode_nop(void) {
	_load((u8[]) { 0x4e, 0x71 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_NOP, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_NONE, i.size);
	TEST_ASSERT_EQUAL(1, i.word_count);
	TEST_ASSERT_EQUAL(2, i.len);
}

// RTS
// 0100 1110 0111 0101
void test_decode_rts(void) {
	_load((u8[]) { 0x4e, 0x75 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_RTS, i.mnemonic);
	TEST_ASSERT_EQUAL(1, i.word_count);
}

// LEA.l (0xdeadbeef).l, A2
// 0100 100 111 111 001 | 0xdead | 0xbeef
void test_decode_lea_abs_long(void) {
	_load((u8[]) { 0x45, 0xf9, 0xde, 0xad, 0xbe, 0xef }, 6);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_LEA, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_LONG, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_ABSOLUTE_LONG, i.src.mode);
	TEST_ASSERT_EQUAL(0xdeadbeef, i.src.absolute_long);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_ADDR, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.reg); // A2

	TEST_ASSERT_EQUAL(3, i.word_count);
}

// CLR.w D4
// 0100 0010 01 000 100
void test_decode_clr_w_dn(void) {
	_load((u8[]) { 0x42, 0x44 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_CLR, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_WORD, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(4, i.dst.reg);

	TEST_ASSERT_EQUAL(1, i.word_count);
}

// MOVEQ #0x7f, D4
// 0111 100 0 0111 1111
void test_decode_moveq(void) {
	_load((u8[]) { 0x78, 0x7f }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_MOVEQ, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_LONG, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(0x7f, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(4, i.dst.reg); // D4

	TEST_ASSERT_EQUAL(1, i.word_count);
	TEST_ASSERT_EQUAL(2, i.len);
}

// LINK A2, #32767
// 0100 1110 0101 0010 | 0x7fff
void test_decode_link(void) {
	_load((u8[]) { 0x4e, 0x52, 0x7f, 0xff }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_LINK, i.mnemonic);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_ADDR, i.src.mode);
	TEST_ASSERT_EQUAL(2, i.src.reg); // A2

	TEST_ASSERT_EQUAL(RBT_EA_DISPLACEMENT, i.dst.mode);
	TEST_ASSERT_EQUAL(32767, i.dst.disp);

	TEST_ASSERT_EQUAL(2, i.word_count);
}

// ----------------------------------------------------------------------------
// Group 0101 — ADDQ/SUBQ/Scc/DBcc
// ----------------------------------------------------------------------------

// ADDQ.l #7, D4
// 0101 111 0 10 000 100
void test_decode_addq_l_dn(void) {
	_load((u8[]) { 0x5e, 0x84 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_ADDQ, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_LONG, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(7, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(4, i.dst.reg);

	TEST_ASSERT_EQUAL(1, i.word_count);
}

// SEQ D4
// 0101 0111 11 000 100
void test_decode_seq_dn(void) {
	_load((u8[]) { 0x57, 0xc4 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_Scc, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_BYTE, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.aux.mode);
	TEST_ASSERT_EQUAL(RBT_COND_EQ, i.aux.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(4, i.dst.reg);

	TEST_ASSERT_EQUAL(1, i.word_count);
}

// DBF D4, -2  (self-referential branch, offset = -2)
// 0101 0001 11 001 100 | 0xfffe
void test_decode_dbf(void) {
	_load((u8[]) { 0x51, 0xcc, 0xff, 0xfe }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_DBcc, i.mnemonic);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.aux.mode);
	TEST_ASSERT_EQUAL(RBT_COND_F, i.aux.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.src.mode);
	TEST_ASSERT_EQUAL(4, i.src.reg); // D4

	TEST_ASSERT_EQUAL(RBT_EA_DISPLACEMENT, i.dst.mode);
	TEST_ASSERT_EQUAL(-2, i.dst.disp);

	TEST_ASSERT_EQUAL(2, i.word_count);
}

// ----------------------------------------------------------------------------
// Group 0110 — Bcc/BSR/BRA
// ----------------------------------------------------------------------------

// BRA.b -2
// 0110 0000 1111 1110
void test_decode_bra_b(void) {
	_load((u8[]) { 0x60, 0xfe }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_BRA, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_BYTE, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_DISPLACEMENT, i.dst.mode);
	TEST_ASSERT_EQUAL(-2, i.dst.disp);

	TEST_ASSERT_EQUAL(1, i.word_count);
	TEST_ASSERT_EQUAL(2, i.len);
}

// BEQ.w -2
// 0110 0111 0000 0000 | 0xfffe
void test_decode_beq_w(void) {
	_load((u8[]) { 0x67, 0x00, 0xff, 0xfe }, 4);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_Bcc, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_WORD, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.aux.mode);
	TEST_ASSERT_EQUAL(RBT_COND_EQ, i.aux.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DISPLACEMENT, i.dst.mode);
	TEST_ASSERT_EQUAL(-2, i.dst.disp);

	TEST_ASSERT_EQUAL(2, i.word_count);
}

// ----------------------------------------------------------------------------
// Group 1001 — SUB/SUBX/SUBA
// ----------------------------------------------------------------------------

// SUB.w D4, (A2)
// 1001 100 1 01 010 010
void test_decode_sub_w_dn_indirect(void) {
	_load((u8[]) { 0x99, 0x52 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_SUB, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_WORD, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.src.mode);
	TEST_ASSERT_EQUAL(4, i.src.reg); // D4

	TEST_ASSERT_EQUAL(RBT_EA_INDIRECT, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.indirect); // A2

	TEST_ASSERT_EQUAL(1, i.word_count);
}

// SUBX.l -(A4), -(A2)
// 1001 100 1 10 00 1 100
void test_decode_subx_l_predec(void) {
	_load((u8[]) { 0x95, 0x8c }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_SUBX, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_LONG, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_INDIRECT_PREDEC, i.src.mode);
	TEST_ASSERT_EQUAL(4, i.src.indirect); // A4

	TEST_ASSERT_EQUAL(RBT_EA_INDIRECT_PREDEC, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.indirect); // A2

	TEST_ASSERT_EQUAL(1, i.word_count);
}

// ----------------------------------------------------------------------------
// Group 1110 — Shift/Rotate
// ----------------------------------------------------------------------------

// ASL.w #4, D2
// 1110 100 1 01 1 00 010
void test_decode_asl_w_imm(void) {
	_load((u8[]) { 0xe9, 0x42 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_ASL, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_WORD, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_IMMEDIATE, i.src.mode);
	TEST_ASSERT_EQUAL(4, i.src.imm);

	TEST_ASSERT_EQUAL(RBT_EA_DIRECT_DATA, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.reg); // D2

	TEST_ASSERT_EQUAL(1, i.word_count);
}

// ROR.w (A2) — memory shift
// 1110 0110 11 010 010
void test_decode_ror_w_memory(void) {
	_load((u8[]) { 0xe6, 0xd2 }, 2);
	RBT_Instruction i = _decode();

	TEST_ASSERT_EQUAL(RBT_OP_ROR, i.mnemonic);
	TEST_ASSERT_EQUAL(RBT_SIZE_WORD, i.size);

	TEST_ASSERT_EQUAL(RBT_EA_INDIRECT, i.dst.mode);
	TEST_ASSERT_EQUAL(2, i.dst.indirect); // A2

	TEST_ASSERT_EQUAL(1, i.word_count);
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------

int main(void) {
	UNITY_BEGIN();

	// Group 0000
	RUN_TEST(test_decode_btst_static_indirect);
	RUN_TEST(test_decode_btst_dynamic_ind_disp);
	RUN_TEST(test_decode_bchg_static_dn);
	RUN_TEST(test_decode_bset_static_abs_short);
	RUN_TEST(test_decode_andi_w_abs_short);
	RUN_TEST(test_decode_ori_b_ccr);
	RUN_TEST(test_decode_cmpi_l_dn);

	// Group 00ss
	RUN_TEST(test_decode_move_l_imm_dn);
	RUN_TEST(test_decode_movea_w_imm);
	RUN_TEST(test_decode_move_b_dn_ind_idx);

	// Group 0100
	RUN_TEST(test_decode_nop);
	RUN_TEST(test_decode_rts);
	RUN_TEST(test_decode_lea_abs_long);
	RUN_TEST(test_decode_clr_w_dn);
	RUN_TEST(test_decode_moveq);
	RUN_TEST(test_decode_link);

	// Group 0101
	RUN_TEST(test_decode_addq_l_dn);
	RUN_TEST(test_decode_seq_dn);
	RUN_TEST(test_decode_dbf);

	// Group 0110
	RUN_TEST(test_decode_bra_b);
	RUN_TEST(test_decode_beq_w);

	// Group 1001
	RUN_TEST(test_decode_sub_w_dn_indirect);
	RUN_TEST(test_decode_subx_l_predec);

	// Group 1110
	RUN_TEST(test_decode_asl_w_imm);
	RUN_TEST(test_decode_ror_w_memory);

	return UNITY_END();
}
