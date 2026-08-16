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

#include "cpu/timing.h"

#include "rbt/basic_types.h"
#include "rbt/cpu/types.h"

#include <assert.h>

// NOTE: For now, I won't encode cycle accurate fetching. Always assume
// worst case scenario when calculating cycles. There's no need to be
// accurate here, but for the future...
// TODO: Improve cycle timing accuracy.

u16 _calculate_timing(
	const RBT_Instruction *instr, const RBT_TimingCtx *ctx, RBT_CpuModel cpu_model
) {
	assert(instr);
	(void)ctx;
	(void)cpu_model;

	u16 cycles = 4; // TODO: Calculate instruction cycles timing

	return cycles;
}
