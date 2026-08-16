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
#include "rbt/error_codes.h"

enum {
	RBT_ERR_STACK_MAX = 64,
};

#define _push_info(...)                                                        \
	_err_push(                                                                 \
		RBT_SEVERITY_INFO, RBT_ERR_SUCCESS, __func__, __FILE_NAME__, __LINE__, \
		__VA_ARGS__                                                            \
	)
#define _push_warn(...)                                                        \
	_err_push(                                                                 \
		RBT_SEVERITY_WARN, RBT_ERR_SUCCESS, __func__, __FILE_NAME__, __LINE__, \
		__VA_ARGS__                                                            \
	)
#define _push_error(code, ...)                                                          \
	_err_push(RBT_SEVERITY_ERROR, code, __func__, __FILE_NAME__, __LINE__, __VA_ARGS__)
#define _push_fatal(code, ...)                                                          \
	_err_push(RBT_SEVERITY_FATAL, code, __func__, __FILE_NAME__, __LINE__, __VA_ARGS__)

void _err_push(
	RBT_ErrorSeverity severity,
	RBT_ErrorCode code,
	const char *func,
	const char *file,
	u32 line,
	const char *fmt,
	...
);
