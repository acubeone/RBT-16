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

typedef RBT_ErrorCode (*RBT_CpuDebugHook)(void *userdata, const RBT_Instruction *instr);

typedef struct RBT_CpuConfig {
	RBT_CpuModel model;

	RBT_CpuDebugHook hook;
	void *userdata;
} RBT_CpuConfig;

typedef struct RBT_Cpu RBT_Cpu;

[[nodiscard]] RBT_Cpu *rbt_create_cpu(const RBT_CpuConfig *config);
void rbt_destroy_cpu(RBT_Cpu *cpu);

void rbt_cpu_attach_bus(RBT_Cpu *cpu, RBT_MemoryBus *bus);

RBT_ErrorCode rbt_cpu_reset(RBT_Cpu *cpu);
RBT_ErrorCode rbt_cpu_step(RBT_Cpu *cpu, u16 *out_cycles);
