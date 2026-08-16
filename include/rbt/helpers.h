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

#if defined(__GNUC__) || defined(__clang__)
#	define rbt_bswap_u16(x) __builtin_bswap16((x))
#	define rbt_bswap_u32(x) __builtin_bswap32((x))
#	define rbt_bswap_u64(x) __builtin_bswap64((x))
#elif defined(_MSC_VER)
#	include <stdlib.h>
#	define rbt_bswap_u16(x) _byteswap_ushort((x))
#	define rbt_bswap_u32(x) _byteswap_ulong((x))
#	define rbt_bswap_u64(x) _byteswap_uint64((x))
#endif

#define RBT_BIT(v, bit) (((v) >> (bit)) & 1u)

[[nodiscard]] u32 rbt_bits(u32 v, i32 hi, i32 lo);
[[nodiscard]] u32 rbt_truncate(RBT_OperandSize size, u32 value);
[[nodiscard]] u32 rbt_store_sized(RBT_OperandSize size, u32 data, u32 value);
[[nodiscard]] i32 rbt_sign_extend(RBT_OperandSize from, u32 value);
