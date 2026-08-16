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
#include "rbt/cpu/cpu.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"

bool _indexext_from_word(u16 ext, RBT_IndexExtension *ix);
u16 _indexext_to_word(const RBT_IndexExtension *ix);

// return next_pc, or UINT32_MAX as an error
u32 _ea_decode(
	u8 mode,
	u8 reg,
	RBT_OperandSize size,
	RBT_MemoryBus *bus,
	u32 pc,
	RBT_EffectiveAddress *ea
);

u32 _ea_compute_address(const RBT_EffectiveAddress *ea, RBT_Cpu *cpu);

RBT_ErrorCode _ea_read(
	const RBT_EffectiveAddress *ea, RBT_OperandSize size, RBT_Cpu *cpu, u32 *out
);
RBT_ErrorCode _ea_write(
	const RBT_EffectiveAddress *ea, RBT_OperandSize size, RBT_Cpu *cpu, u32 in
);
