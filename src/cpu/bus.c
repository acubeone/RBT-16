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

#include "rbt/cpu/bus.h"

#include "bus_internal.h"
#include "error.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const u32 _ram_module_sizes[] = {
	[RBT_RAM_NONE] = 0,
	[RBT_RAM_256KB] = 256 * 1024,
	[RBT_RAM_512KB] = 512 * 1024,
	[RBT_RAM_1MB] = 1024 * 1024,
};

static inline bool _is_address_in_range(u32 addr, u32 start, u32 size) {
	return addr >= start && addr < start + size;
}

static inline u32 _get_ram_index(RBT_RamDevice *ram, u32 addr) {
	assert(ram->slot_size[0] != 0 && "Slot 0 must be populated");

	u32 slot = addr >> 20;
	u32 subaddr = addr & 0x0f'ffff;

	// If slot is unpopulated, mirror into slot 0
	if (ram->slot_size[slot] == 0) {
		slot = 0;
		subaddr = addr % ram->slot_size[0];
	}

	return ram->slot_offset[slot] + (subaddr % ram->slot_size[slot]);
}

static inline RBT_ErrorCode _ram_read_byte(void *device, u32 addr, u8 *byte) {
	RBT_RamDevice *ram = (RBT_RamDevice *)device;
	if (!ram->data) {
		return RBT_ERR_MEM_BUS_ERROR;
	}

	// RAM region (Wrap around on unused ram slots)
	*byte = ram->data[_get_ram_index(ram, addr)];
	return RBT_ERR_SUCCESS;
}

static inline RBT_ErrorCode _ram_write_byte(void *device, u32 addr, u8 byte) {
	RBT_RamDevice *ram = (RBT_RamDevice *)device;
	if (!ram->data) {
		return RBT_ERR_MEM_BUS_ERROR;
	}

	// RAM region (Wrap around at unused ram slots)
	ram->data[_get_ram_index(ram, addr)] = byte;
	return RBT_ERR_SUCCESS;
}

static inline RBT_ErrorCode _rom_read_byte(void *device, u32 addr, u8 *byte) {
	u8 *rom = (u8 *)device;

	// ROM region
	u32 offset = (addr - _BUS_ROM_ADDR) % _BUS_ROM_SIZE;
	*byte = rom[offset];
	return RBT_ERR_SUCCESS;
}

static inline RBT_ErrorCode _rom_read_word(void *device, u32 addr, u16 *word) {
	u8 *rom = (u8 *)device;

	u32 offset = (addr - _BUS_ROM_ADDR) % _BUS_ROM_SIZE;

	u16 data = rom[offset] << 8;
	if (offset + 1 < _BUS_ROM_SIZE) {
		data |= rom[offset + 1];
	}

	*word = data;
	return RBT_ERR_SUCCESS;
}

static inline RBT_ErrorCode _rom_write_byte(void *device, u32 addr, u8 byte) {
	(void)device;
	(void)byte;

	_push_warn("Write attempt on ROM at: 0x%06x", addr);
	return RBT_ERR_MEM_READONLY;
}

static RBT_IODevice *_query_iodevice_range(RBT_MemoryBus *bus, u32 addr, u32 *offset) {
	assert(offset);

	for (i32 i = 0; i < _RBT_BUSDEV_COUNT; i += 1) {
		RBT_IODevice *dev = &bus->mmio_devices[i];
		if (dev->size != 0 && _is_address_in_range(addr, dev->addr, dev->size)) {
			*offset = addr - dev->addr;
			return dev;
		}
	}

	return nullptr;
}

RBT_ErrorCode _bus_fetch_imm(
	RBT_MemoryBus *bus, RBT_OperandSize size, u32 addr, u32 *out
) {
	switch (size) {
	case RBT_SIZE_BYTE:
	case RBT_SIZE_WORD: {
		u16 word;
		RBT_ErrorCode err = rbt_bus_read_word(bus, addr, &word);
		*out = size == RBT_SIZE_BYTE ? (word & 0xff) : word;
		return err;
	}
	case RBT_SIZE_LONG: return rbt_bus_read_long(bus, addr, out);
	default:			return RBT_ERR_INVALID_ARGS;
	}

	unreachable();
}

[[nodiscard]] RBT_MemoryBus *rbt_create_bus(const RBT_BusConfig *cfg) {
	assert(cfg);

	if (cfg->ram_slots[0] == RBT_RAM_NONE) {
		_push_fatal(RBT_ERR_INVALID_ARGS, "RAM slot 0 must be populated");
		return nullptr;
	}

	RBT_MemoryBus *bus = malloc(sizeof(RBT_MemoryBus));
	if (!bus) {
		_push_fatal(RBT_ERR_SYS_OUT_OF_MEMORY, "Failed to allocate RBT_MemoryBus");
		goto error;
	}
	memset(bus, 0, sizeof(RBT_MemoryBus));

	u32 offset = 0;
	for (i32 i = 0; i < _BUS_RAM_SLOTS_COUNT; i += 1) {
		bus->ram.slot_size[i] = _ram_module_sizes[(u32)cfg->ram_slots[i]];
		bus->ram.slot_offset[i] = offset;
		offset += bus->ram.slot_size[i];
	}

	bus->ram.size = offset;
	bus->ram.data = malloc(bus->ram.size);
	if (!bus->ram.data) {
		_push_fatal(RBT_ERR_SYS_OUT_OF_MEMORY, "Failed to allocate RAM");
		goto error;
	}

	bus->rom = malloc(_BUS_ROM_SIZE);
	if (!bus->rom) {
		_push_fatal(RBT_ERR_SYS_OUT_OF_MEMORY, "Failed to allocate ROM memory area");
		goto error;
	}
	memset(bus->rom, 0, _BUS_ROM_SIZE);

	// Register IO Devices
	bus->mmio_devices[RBT_BUSDEV_RAM] = (RBT_IODevice) {
		.addr = _BUS_RAM_ADDR,
		.size = _BUS_RAM_SIZE,
		.device = &bus->ram,
		.read_byte = _ram_read_byte,
		.read_word = nullptr,
		.write_byte = _ram_write_byte,
		.write_word = nullptr,
	};

	bus->mmio_devices[RBT_BUSDEV_ROM] = (RBT_IODevice) {
		.addr = _BUS_ROM_ADDR,
		.size = _BUS_ROM_SIZE * 2, // map out the 512KB of ROM + MIRROR
		.device = bus->rom,
		.read_byte = _rom_read_byte,
		.read_word = _rom_read_word,
		.write_byte = _rom_write_byte,
		.write_word = nullptr,
	};

	return bus;

error:
	if (bus)
		rbt_destroy_bus(bus);

	return nullptr;
}

void rbt_destroy_bus(RBT_MemoryBus *bus) {
	if (!bus)
		return;

	if (bus->ram.data)
		free(bus->ram.data);
	if (bus->rom)
		free(bus->rom);

	free(bus);
}

void rbt_bus_reset(RBT_MemoryBus *bus) {
	if (!bus)
		return;

	memset(bus->ram.data, 0, bus->ram.size);
}

void rbt_bus_attach_iodevice(
	RBT_MemoryBus *bus, RBT_BusDevice busdev, const RBT_IODevice *device
) {
	assert(bus);
	assert(device);
	assert(busdev < _RBT_BUSDEV_COUNT);

	if (busdev == RBT_BUSDEV_RAM || busdev == RBT_BUSDEV_ROM) {
		_push_warn("Cannot override RAM/ROM devices!");
		return;
	}

	bus->mmio_devices[busdev] = *device;
}

RBT_ErrorCode rbt_bus_init(RBT_MemoryBus *bus, usize size, const u8 *rom) {
	assert(bus);

	if (size == 0 || !rom) {
		return RBT_ERR_INVALID_ARGS;
	}

	if (size > _BUS_ROM_SIZE) {
		_push_warn("ROM truncated: size %zu exceeds max %u", size, _BUS_ROM_SIZE);
		size = _BUS_ROM_SIZE;
	}
	memcpy(bus->rom, rom, size);

	// Mirror vector table into RAM
	usize vec_table_size = 1024; // 256 vectors * 4 bytes
	memcpy(bus->ram.data, bus->rom, vec_table_size);

	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode rbt_bus_init_from_file(RBT_MemoryBus *bus, const char *filename) {
	assert(bus && bus->rom);
	assert(filename);

	FILE *file = fopen(filename, "rb");
	if (!file) {
		_push_error(RBT_ERR_SYS_IO, "Failed to open ROM file at: %s", filename);
		return RBT_ERR_SYS_IO;
	}

	usize size = 0;
	while (size < _BUS_ROM_SIZE) {
		u8 buf[4096];
		usize bytes_read = fread(buf, 1, 4096, file);
		if (bytes_read == 0) {
			break;
		}

		usize remaining = _BUS_ROM_SIZE - size;
		if (bytes_read > remaining)
			bytes_read = remaining;

		memcpy(bus->rom + size, buf, bytes_read);
		size += bytes_read;
	}

	if (ferror(file)) {
		_push_fatal(RBT_ERR_SYS_IO, "Unable to read ROM file: %s", filename);
		fclose(file);
		return RBT_ERR_SYS_IO;
	}

	fclose(file);

	// Mirror vector table into RAM
	usize vec_table_size = 1024; // 256 vectors * 4 bytes
	memcpy(bus->ram.data, bus->rom, vec_table_size);

	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode rbt_bus_read_byte(RBT_MemoryBus *bus, u32 addr, u8 *out) {
	assert(bus);
	assert(out);

	addr &= 0x00ffffff;

	if (_is_address_in_range(addr, _BUS_RESERVED_DTACK_ADDR, _BUS_RESERVED_DTACK_SIZE)) {
		return RBT_ERR_SUCCESS;
	}

	if (_is_address_in_range(addr, _BUS_RESERVED_BERR_ADDR, _BUS_RESERVED_BERR_SIZE)) {
		_push_error(
			RBT_ERR_MEM_BUS_ERROR, "Tried to access invalid address at: 0x%06x", addr
		);
		return RBT_ERR_MEM_BUS_ERROR;
	}

	u32 offset;
	RBT_IODevice *io = _query_iodevice_range(bus, addr, &offset);
	if (!io || !io->read_byte) {
		const RBT_ErrorEntry *last = rbt_query_last_error();
		if (last && last->code == RBT_ERR_MEM_BUS_ERROR)
			return RBT_ERR_MEM_BUS_ERROR;

		_push_warn("Memory isn't mapped at: 0x%06x", addr);
		return RBT_ERR_MEM_UNMAPPED;
	}

	return io->read_byte(io->device, offset, out);
}

RBT_ErrorCode rbt_bus_read_word(RBT_MemoryBus *bus, u32 addr, u16 *out) {
	assert(bus);
	assert(out);

	addr &= 0x00ffffff;

	if (_is_address_in_range(addr, _BUS_RESERVED_DTACK_ADDR, _BUS_RESERVED_DTACK_SIZE)) {
		return RBT_ERR_SUCCESS;
	}

	if (_is_address_in_range(addr, _BUS_RESERVED_BERR_ADDR, _BUS_RESERVED_BERR_SIZE)) {
		_push_error(
			RBT_ERR_MEM_BUS_ERROR, "Tried to access invalid address at: 0x%06x", addr
		);
		return RBT_ERR_MEM_BUS_ERROR;
	}

	// The M68000/MC68008/MC68010 does not support unaligned access
	if (addr & 1) {
		_push_warn("Unaligned memory access at: 0x%06x", addr);
		return RBT_ERR_MEM_ADDR_ERROR;
	}

	u32 offset;
	RBT_IODevice *io = _query_iodevice_range(bus, addr, &offset);
	if (!io) {
		const RBT_ErrorEntry *last = rbt_query_last_error();
		if (last && last->code == RBT_ERR_MEM_BUS_ERROR)
			return RBT_ERR_MEM_BUS_ERROR;

		_push_warn("Memory isn't mapped at: 0x%06x", addr);
		return RBT_ERR_MEM_UNMAPPED;
	}

	if (io->read_word)
		return io->read_word(io->device, offset, out);

	if (!io->read_byte) {
		_push_error(RBT_ERR_MEM_UNMAPPED, "Memory isn't mapped at: 0x%06x", addr);
		return RBT_ERR_MEM_UNMAPPED;
	}

	RBT_ErrorCode err;
	u8 hi;
	u8 lo;

	err = io->read_byte(io->device, offset + 0, &hi);
	if (err)
		return err;

	err = io->read_byte(io->device, offset + 1, &lo);
	if (err)
		return err;

	*out = (hi << 8) | lo;
	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode rbt_bus_read_long(RBT_MemoryBus *bus, u32 addr, u32 *out) {
	assert(bus);
	assert(out);

	u16 high_word;
	u16 low_word;

	RBT_ErrorCode err = rbt_bus_read_word(bus, addr, &high_word);
	if (err)
		return err;

	err = rbt_bus_read_word(bus, addr + 2, &low_word);
	if (err)
		return err;

	*out = ((u32)high_word << 16) | low_word;
	return RBT_ERR_SUCCESS;
}

RBT_ErrorCode rbt_bus_write_byte(RBT_MemoryBus *bus, u32 addr, u8 byte) {
	assert(bus);

	addr &= 0x00ffffff;

	if (_is_address_in_range(addr, _BUS_RESERVED_DTACK_ADDR, _BUS_RESERVED_DTACK_SIZE)) {
		return RBT_ERR_SUCCESS;
	}

	if (_is_address_in_range(addr, _BUS_RESERVED_BERR_ADDR, _BUS_RESERVED_BERR_SIZE)) {
		_push_error(
			RBT_ERR_MEM_BUS_ERROR, "Tried to access invalid address at: 0x%06x", addr
		);
		return RBT_ERR_MEM_BUS_ERROR;
	}

	u32 offset;
	RBT_IODevice *io = _query_iodevice_range(bus, addr, &offset);
	if (!io || !io->write_byte) {
		const RBT_ErrorEntry *last = rbt_query_last_error();
		if (last && last->code == RBT_ERR_MEM_BUS_ERROR)
			return RBT_ERR_MEM_BUS_ERROR;

		_push_warn("Memory isn't mapped at: 0x%06x", addr);
		return RBT_ERR_MEM_UNMAPPED;
	}

	return io->write_byte(io->device, offset, byte);
}

RBT_ErrorCode rbt_bus_write_word(RBT_MemoryBus *bus, u32 addr, u16 word) {
	assert(bus);

	addr &= 0x00ffffff;

	if (_is_address_in_range(addr, _BUS_RESERVED_DTACK_ADDR, _BUS_RESERVED_DTACK_SIZE)) {
		return RBT_ERR_SUCCESS;
	}

	if (_is_address_in_range(addr, _BUS_RESERVED_BERR_ADDR, _BUS_RESERVED_BERR_SIZE)) {
		_push_error(
			RBT_ERR_MEM_BUS_ERROR, "Tried to access invalid address at: 0x%06x", addr
		);
		return RBT_ERR_MEM_BUS_ERROR;
	}

	// The M68000/MC68008/MC68010 doesn't support unaligned access
	if (addr & 1) {
		_push_warn("Unaligned memory access at: 0x%06x", addr);
		return RBT_ERR_MEM_ADDR_ERROR;
	}

	u32 offset;
	RBT_IODevice *io = _query_iodevice_range(bus, addr, &offset);
	if (!io) {
		const RBT_ErrorEntry *last = rbt_query_last_error();
		if (last && last->code == RBT_ERR_MEM_BUS_ERROR)
			return RBT_ERR_MEM_BUS_ERROR;

		_push_warn("Memory isn't mapped at: 0x%06x", addr);
		return RBT_ERR_MEM_UNMAPPED;
	}

	if (io->write_word)
		return io->write_word(io->device, offset, word);

	if (!io->write_byte) {
		_push_error(RBT_ERR_MEM_UNMAPPED, "Memory isn't mapped at: 0x%06x", addr);
		return RBT_ERR_MEM_UNMAPPED;
	}

	RBT_ErrorCode err = io->write_byte(io->device, offset + 0, (word >> 8) & 0xff);
	if (err)
		return err;

	return io->write_byte(io->device, offset + 1, word & 0xff);
}

RBT_ErrorCode rbt_bus_write_long(RBT_MemoryBus *bus, u32 addr, u32 long_) {
	assert(bus);

	RBT_ErrorCode err = rbt_bus_write_word(bus, addr, (long_ >> 16) & 0xffff);
	if (err)
		return err;

	return rbt_bus_write_word(bus, addr + 2, long_ & 0xffff);
}

RBT_ErrorCode rbt_bus_load(RBT_MemoryBus *bus, RBT_OperandSize size, u32 addr, u32 *out) {
	RBT_ErrorCode err;

	switch (size) {
	case RBT_SIZE_BYTE: {
		u8 byte;
		err = rbt_bus_read_byte(bus, addr, &byte);
		if (!err)
			*out = byte;
		return err;
	};
	case RBT_SIZE_WORD: {
		u16 word;
		err = rbt_bus_read_word(bus, addr, &word);
		if (!err)
			*out = word;
		return err;
	};
	case RBT_SIZE_LONG: return rbt_bus_read_long(bus, addr, out);
	default:			return RBT_ERR_INVALID_ARGS;
	}

	unreachable();
}

RBT_ErrorCode rbt_bus_store(
	RBT_MemoryBus *bus, RBT_OperandSize size, u32 addr, u32 data
) {
	switch (size) {
	case RBT_SIZE_BYTE: return rbt_bus_write_byte(bus, addr, data & 0x00ff);
	case RBT_SIZE_WORD: return rbt_bus_write_word(bus, addr, data & 0xffff);
	case RBT_SIZE_LONG: return rbt_bus_write_long(bus, addr, data);
	default:			return RBT_ERR_INVALID_ARGS;
	}

	unreachable();
}
