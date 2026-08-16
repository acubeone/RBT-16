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

#include "rbt/helpers.h"

#include "error.h"
#include "rbt/basic_types.h"
#include "rbt/cpu/types.h"
#include "rbt/error_codes.h"

#include <assert.h>

[[nodiscard]] u32 rbt_bits(u32 v, i32 hi, i32 lo) {
	assert((hi >= lo) && "rbt_bits: High bit cannot be lower than low bit!");

	u32 width = (hi - lo) + 1; // find how many bits we gonna use
	u32 mask = (width >= 32) ? 0xffffffffu : ((1u << width) - 1u);

	return (v >> lo) & mask;
}

[[nodiscard]] u32 rbt_truncate(RBT_OperandSize size, u32 value) {
	switch (size) {
	case RBT_SIZE_BYTE: return value & 0x000000ff;
	case RBT_SIZE_WORD: return value & 0x0000ffff;
	case RBT_SIZE_LONG: return value;
	case RBT_SIZE_NONE:
		_push_error(RBT_ERR_INVALID_ARGS, "Cannot truncate value size");
		return 0;
	}

	unreachable();
}

[[nodiscard]] u32 rbt_store_sized(RBT_OperandSize size, u32 data, u32 value) {
	switch (size) {
	case RBT_SIZE_BYTE: return (data & 0xffffff00) | (value & 0x000000ff);
	case RBT_SIZE_WORD: return (data & 0xffff0000) | (value & 0x0000ffff);
	case RBT_SIZE_LONG: return value;
	case RBT_SIZE_NONE:
		_push_error(RBT_ERR_INVALID_ARGS, "Cannot store without size");
		return 0;
	}

	unreachable();
}

[[nodiscard]] i32 rbt_sign_extend(RBT_OperandSize from, u32 value) {
	switch (from) {
	case RBT_SIZE_BYTE: return (i8)(value & 0xff);
	case RBT_SIZE_WORD: return (i16)(value & 0xffff);
	case RBT_SIZE_LONG: return value;
	case RBT_SIZE_NONE:
		_push_error(RBT_ERR_INVALID_ARGS, "Cannot sign extend without size");
		return 0;
	}

	unreachable();
}
