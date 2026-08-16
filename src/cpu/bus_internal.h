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
#include "rbt/cpu/bus.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"

enum {
	_BUS_RAM_ADDR = 0x00'0000,
	_BUS_RAM_SLOT_WINDOW = 1024 * 1024, // // Each slot occupies 1MB window
	_BUS_RAM_SLOTS_COUNT = 4,
	_BUS_RAM_SIZE = _BUS_RAM_SLOT_WINDOW * _BUS_RAM_SLOTS_COUNT, // 4MB (Max total RAM)

	// ROM mirror: 0xf4'0000-f7'ffff
	_BUS_ROM_ADDR = 0xf0'0000,
	_BUS_ROM_SIZE = 256 * 1024, // 256KB (Kernel ROM)
	_BUS_ROM_MIRROR_ADDR = _BUS_ROM_ADDR + _BUS_ROM_SIZE,

	// MMIO devices: 256-bytes each
	_BUS_MMIO_ADDR = 0xfb'0000,
	_BUS_MMIO_SIZE = 256, // Each MMIO region is 256-bytes wide
	_BUS_MMIO_VDP_ADDR = _BUS_MMIO_ADDR + 0x0000,
	_BUS_MMIO_IO_ADDR = _BUS_MMIO_ADDR + 0x0100,
	_BUS_MMIO_SD_ADDR = _BUS_MMIO_ADDR + 0x0200,
	_BUS_MMIO_SYS_ADDR = _BUS_MMIO_ADDR + 0x0300,

	// Expansion cards: 64KB each
	_BUS_EXT_SIZE = 64 * 1024, // 64KB  (Per Expansion Card)
	_BUS_EXT_SLOTS_COUNT = 4,
	_BUS_EXT0_ADDR = 0xfc'0000,
	_BUS_EXT1_ADDR = 0xfd'0000,
	_BUS_EXT2_ADDR = 0xfe'0000,
	_BUS_EXT3_ADDR = 0xff'0000,

	// Reserved regions
	_BUS_RESERVED_BERR_ADDR = 0x40'0000,
	_BUS_RESERVED_BERR_SIZE = 11 * (1024 * 1024), // ~11MB triggers /BERR
	_BUS_RESERVED_DTACK_ADDR = 0xf8'0000,
	_BUS_RESERVED_DTACK_SIZE = 3 * (64 * 1024), // 192KB does nothing /DTACK
};

typedef struct RBT_RamDevice {
	u8 *data; // 0x00'0000-0x3f'ffff (Max 4MB)
	usize size;

	u32 slot_size[_BUS_RAM_SLOTS_COUNT];
	u32 slot_offset[_BUS_RAM_SLOTS_COUNT];
} RBT_RamDevice;

typedef struct RBT_MemoryBus {
	RBT_IODevice mmio_devices[_RBT_BUSDEV_COUNT];

	RBT_RamDevice ram;
	u8 *rom; // 0xf0'0000-0xf3'ffff (256KB)
} RBT_MemoryBus;

RBT_ErrorCode _bus_fetch_imm(
	RBT_MemoryBus *bus, RBT_OperandSize size, u32 addr, u32 *out
);
