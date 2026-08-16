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

typedef enum RBT_OpGroup {
	_OPGROUP_BITMOVEPIMM, // Bit/MOVEP/Immediate
	_OPGROUP_MOVEBYTE,	  // MOVE.b
	_OPGROUP_MOVELONG,	  // MOVE.l
	_OPGROUP_MOVEWORD,	  // MOVE.w
	_OPGROUP_MISC,		  // Miscellaneous
	_OPGROUP_ADDQSUBQ,	  // ADDQ/SUBQ/DBcc
	_OPGROUP_BRANCH,	  // Bcc/BSR/BRA
	_OPGROUP_MOVEQ,		  // MOVEQ
	_OPGROUP_ORDIV,		  // OR/DIV/SBCD
	_OPGROUP_SUBSUBX,	  // SUB/SUBX
	_OPGROUP_LINEA,		  // Reserved/Unassigned
	_OPGROUP_CMPEOR,	  // CMP/EOR
	_OPGROUP_ANDMUL,	  // AND/MUL/ABCD/EXG
	_OPGROUP_ADDADDX,	  // ADD/ADDX
	_OPGROUP_SHIFT,		  // Shift/Rotate
	_OPGROUP_LINEF,		  // Extensions
} RBT_OpGroup;

RBT_ErrorCode _decode_instruction(RBT_MemoryBus *bus, u32 pc, RBT_Instruction *instr);
