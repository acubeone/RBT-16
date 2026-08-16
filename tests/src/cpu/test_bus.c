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
#include "rbt/basic_types.h"
#include "rbt/cpu/bus.h"
#include "rbt/error_codes.h"
#include "unity_internals.h"

#include <stdio.h>
#include <unity.h>

static RBT_MemoryBus *_make_bus(
	RBT_RamModuleSize s0, RBT_RamModuleSize s1, RBT_RamModuleSize s2, RBT_RamModuleSize s3
) {
	RBT_BusConfig cfg = { .ram_slots = { s0, s1, s2, s3 } };
	return rbt_create_bus(&cfg);
}

void setUp(void) { }
void tearDown(void) { }

static void test_create_slot0_unpopulated_fails(void) {
	TEST_ASSERT_NULL(_make_bus(RBT_RAM_NONE, RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE));
}

static void test_create_only_slot0(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);
	TEST_ASSERT_NOT_NULL(bus);
	TEST_ASSERT_NOT_NULL(bus->ram.data);
	TEST_ASSERT_NOT_NULL(bus->rom);
	TEST_ASSERT_EQUAL(256 * 1024, bus->ram.size);
	rbt_destroy_bus(bus);
}

static void test_create_all_slots(void) {
	RBT_MemoryBus *bus = _make_bus(RBT_RAM_1MB, RBT_RAM_1MB, RBT_RAM_1MB, RBT_RAM_1MB);
	TEST_ASSERT_NOT_NULL(bus);
	TEST_ASSERT_EQUAL(4 * 1024 * 1024, bus->ram.size);
	rbt_destroy_bus(bus);
}

static void test_create_mixed_slots(void) {
	// 256KB + unpopulated + 512KB + unpopulated = 768KB allocated
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_512KB, RBT_RAM_NONE
	);
	TEST_ASSERT_NOT_NULL(bus);
	TEST_ASSERT_EQUAL((256 + 512) * 1024, bus->ram.size);
	rbt_destroy_bus(bus);
}

static void test_ram_read_write_byte(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_write_byte(bus, 0x000100, 0xab));
	u8 out;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, 0x000100, &out));
	TEST_ASSERT_EQUAL_UINT8(0xab, out);

	rbt_destroy_bus(bus);
}

static void test_ram_read_write_word(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_write_word(bus, 0x000200, 0xdead));
	u16 out;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_word(bus, 0x000200, &out));
	TEST_ASSERT_EQUAL_UINT16(0xdead, out);

	rbt_destroy_bus(bus);
}

static void test_ram_read_write_long(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_write_long(bus, 0x000400, 0xdeadbeef));
	u32 out;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_long(bus, 0x000400, &out));
	TEST_ASSERT_EQUAL_UINT32(0xdeadbeef, out);

	rbt_destroy_bus(bus);
}

static void test_ram_slot0_wraps_at_module_size(void) {
	// 256KB module in 1MB window — address 256KB should alias to address 0
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	rbt_bus_write_byte(bus, 0x000000, 0x42);

	u8 out;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, 256 * 1024, &out));
	TEST_ASSERT_EQUAL_UINT8(0x42, out); // wrapped back to offset 0

	rbt_destroy_bus(bus);
}

static void test_ram_slot0_wraps_512kb_module(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_512KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	rbt_bus_write_byte(bus, 0x000010, 0x77);

	u8 out;
	// 512KB + 0x10 should wrap to 0x10
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, (512 * 1024) + 0x10, &out));
	TEST_ASSERT_EQUAL_UINT8(0x77, out);

	rbt_destroy_bus(bus);
}

static void test_ram_unpopulated_slot_mirrors_slot0(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	rbt_bus_write_byte(bus, 0x000020, 0x55);

	u8 out;
	// Slot 1 (0x10'0000) is unpopulated — mirrors into slot 0
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, 0x10'0020, &out));
	TEST_ASSERT_EQUAL_UINT8(0x55, out);

	rbt_destroy_bus(bus);
}

static void test_ram_populated_slot_does_not_mirror(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE
	);

	rbt_bus_write_byte(bus, 0x000020, 0x11);
	rbt_bus_write_byte(bus, 0x10'0020, 0x22);

	u8 out;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, 0x000020, &out));
	TEST_ASSERT_EQUAL_UINT8(0x11, out);

	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, 0x10'0020, &out));
	TEST_ASSERT_EQUAL_UINT8(0x22, out);

	rbt_destroy_bus(bus);
}

static void test_rom_init_and_read(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	u8 rom[8] = { 0x00, 0x00, 0x20, 0x00, 0x00, 0xf0, 0x00, 0x00 };
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_init(bus, sizeof(rom), rom));

	u8 out;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, _BUS_ROM_ADDR + 2, &out));
	TEST_ASSERT_EQUAL_UINT8(0x20, out);

	rbt_destroy_bus(bus);
}

static void test_rom_mirror(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	u8 rom[4] = { 0xde, 0xad, 0xbe, 0xef };
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_init(bus, sizeof(rom), rom));

	u8 out;
	// Primary window
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, _BUS_ROM_ADDR, &out));
	TEST_ASSERT_EQUAL_UINT8(0xde, out);

	// Mirror window (0xf4'0000)
	TEST_ASSERT_EQUAL(
		RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, _BUS_ROM_MIRROR_ADDR, &out)
	);
	TEST_ASSERT_EQUAL_UINT8(0xde, out);

	TEST_ASSERT_EQUAL(
		RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, _BUS_ROM_MIRROR_ADDR + 3, &out)
	);
	TEST_ASSERT_EQUAL_UINT8(0xef, out);

	rbt_destroy_bus(bus);
}

static void test_rom_write_readonly(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	TEST_ASSERT_EQUAL(RBT_ERR_MEM_READONLY, rbt_bus_write_byte(bus, _BUS_ROM_ADDR, 0xff));
	TEST_ASSERT_EQUAL(
		RBT_ERR_MEM_READONLY, rbt_bus_write_byte(bus, _BUS_ROM_MIRROR_ADDR, 0xff)
	);

	rbt_destroy_bus(bus);
}

static void test_rom_init_from_file(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	FILE *f = fopen("test.rom", "wb");
	TEST_ASSERT_NOT_NULL(f);
	u8 data[] = { 0x11, 0x22, 0x33, 0x44 };
	fwrite(data, 1, sizeof(data), f);
	fclose(f);

	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_init_from_file(bus, "test.rom"));

	u8 out;
	TEST_ASSERT_EQUAL(RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, _BUS_ROM_ADDR + 1, &out));
	TEST_ASSERT_EQUAL_UINT8(0x22, out);

	remove("test.rom");
	rbt_destroy_bus(bus);
}

static void test_unaligned_word_access(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	u16 dummy = 0;
	TEST_ASSERT_EQUAL(RBT_ERR_MEM_ADDR_ERROR, rbt_bus_read_word(bus, 0x000001, &dummy));
	TEST_ASSERT_EQUAL(RBT_ERR_MEM_ADDR_ERROR, rbt_bus_write_word(bus, 0x000003, 0xffff));

	rbt_destroy_bus(bus);
}

static void test_berr_region(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	u8 out;
	TEST_ASSERT_EQUAL(
		RBT_ERR_MEM_BUS_ERROR, rbt_bus_read_byte(bus, _BUS_RESERVED_BERR_ADDR, &out)
	);
	TEST_ASSERT_EQUAL(
		RBT_ERR_MEM_BUS_ERROR, rbt_bus_write_byte(bus, _BUS_RESERVED_BERR_ADDR, 0xff)
	);

	rbt_destroy_bus(bus);
}

static void test_dtack_region(void) {
	// DTACK region does nothing — reads/writes silently succeed
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);

	u8 out;
	TEST_ASSERT_EQUAL(
		RBT_ERR_SUCCESS, rbt_bus_read_byte(bus, _BUS_RESERVED_DTACK_ADDR, &out)
	);
	TEST_ASSERT_EQUAL(
		RBT_ERR_SUCCESS, rbt_bus_write_byte(bus, _BUS_RESERVED_DTACK_ADDR, 0xff)
	);

	rbt_destroy_bus(bus);
}

static void test_disabled_ext_card_berr(void) {
	RBT_MemoryBus *bus = _make_bus(
		RBT_RAM_256KB, RBT_RAM_NONE, RBT_RAM_NONE, RBT_RAM_NONE
	);
	// No ext card attached — access should BERR

	u8 out;
	TEST_ASSERT_EQUAL(
		RBT_ERR_MEM_BUS_ERROR, rbt_bus_read_byte(bus, _BUS_EXT0_ADDR, &out)
	);
	TEST_ASSERT_EQUAL(
		RBT_ERR_MEM_BUS_ERROR, rbt_bus_write_byte(bus, _BUS_EXT0_ADDR, 0xff)
	);

	rbt_destroy_bus(bus);
}

int main(void) {
	UNITY_BEGIN();

	RUN_TEST(test_create_slot0_unpopulated_fails);
	RUN_TEST(test_create_only_slot0);
	RUN_TEST(test_create_all_slots);
	RUN_TEST(test_create_mixed_slots);

	RUN_TEST(test_ram_read_write_byte);
	RUN_TEST(test_ram_read_write_word);
	RUN_TEST(test_ram_read_write_long);

	RUN_TEST(test_ram_slot0_wraps_at_module_size);
	RUN_TEST(test_ram_slot0_wraps_512kb_module);

	RUN_TEST(test_ram_unpopulated_slot_mirrors_slot0);
	RUN_TEST(test_ram_populated_slot_does_not_mirror);

	RUN_TEST(test_rom_init_and_read);
	RUN_TEST(test_rom_mirror);
	RUN_TEST(test_rom_write_readonly);
	RUN_TEST(test_rom_init_from_file);

	RUN_TEST(test_unaligned_word_access);
	RUN_TEST(test_berr_region);
	RUN_TEST(test_dtack_region);
	RUN_TEST(test_disabled_ext_card_berr);

	return UNITY_END();
}
