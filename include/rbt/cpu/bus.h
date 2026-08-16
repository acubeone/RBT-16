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
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"

typedef enum RBT_BusDevice {
	RBT_BUSDEV_RAM,
	RBT_BUSDEV_ROM,
	RBT_BUSDEV_VDP,
	RBT_BUSDEV_IO,
	RBT_BUSDEV_SD,
	RBT_BUSDEV_SYS,
	RBT_BUSDEV_EXT0,
	RBT_BUSDEV_EXT1,
	RBT_BUSDEV_EXT2,
	RBT_BUSDEV_EXT3,
	_RBT_BUSDEV_COUNT,
} RBT_BusDevice;

typedef enum RBT_RamModuleSize {
	RBT_RAM_NONE,
	RBT_RAM_256KB,
	RBT_RAM_512KB,
	RBT_RAM_1MB,
} RBT_RamModuleSize;

typedef RBT_ErrorCode (*RBT_IOReadByteCallback)(void *device, u32 addr, u8 *byte);
typedef RBT_ErrorCode (*RBT_IOReadWordCallback)(void *device, u32 addr, u16 *word);
typedef RBT_ErrorCode (*RBT_IOWriteByteCallback)(void *device, u32 addr, u8 byte);
typedef RBT_ErrorCode (*RBT_IOWriteWordCallback)(void *device, u32 addr, u16 word);

typedef struct RBT_IODevice {
	u32 addr;
	u32 size;

	void *device;
	RBT_IOReadByteCallback read_byte;
	RBT_IOReadWordCallback read_word;
	RBT_IOWriteByteCallback write_byte;
	RBT_IOWriteWordCallback write_word;
} RBT_IODevice;

typedef struct RBT_BusConfig {
	RBT_RamModuleSize ram_slots[4];
} RBT_BusConfig;

typedef struct RBT_MemoryBus RBT_MemoryBus;

[[nodiscard]] RBT_MemoryBus *rbt_create_bus(const RBT_BusConfig *cfg);
void rbt_destroy_bus(RBT_MemoryBus *bus);
void rbt_bus_reset(RBT_MemoryBus *bus);

void rbt_bus_attach_iodevice(
	RBT_MemoryBus *bus, RBT_BusDevice dev, const RBT_IODevice *device
);
RBT_ErrorCode rbt_bus_init(RBT_MemoryBus *bus, usize size, const u8 *rom);
RBT_ErrorCode rbt_bus_init_from_file(RBT_MemoryBus *bus, const char *filename);

RBT_ErrorCode rbt_bus_read_byte(RBT_MemoryBus *bus, u32 addr, u8 *out);
RBT_ErrorCode rbt_bus_read_word(RBT_MemoryBus *bus, u32 addr, u16 *out);
RBT_ErrorCode rbt_bus_read_long(RBT_MemoryBus *bus, u32 addr, u32 *out);

RBT_ErrorCode rbt_bus_write_byte(RBT_MemoryBus *bus, u32 addr, u8 byte);
RBT_ErrorCode rbt_bus_write_word(RBT_MemoryBus *bus, u32 addr, u16 word);
RBT_ErrorCode rbt_bus_write_long(RBT_MemoryBus *bus, u32 addr, u32 long_);

RBT_ErrorCode rbt_bus_load(RBT_MemoryBus *bus, RBT_OperandSize size, u32 addr, u32 *out);
RBT_ErrorCode rbt_bus_store(RBT_MemoryBus *bus, RBT_OperandSize size, u32 addr, u32 data);
