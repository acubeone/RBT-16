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

#include "error.h"

#include "rbt/basic_types.h"
#include "rbt/error_codes.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef __FILE_NAME__
#	define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

typedef struct RBT_ErrorContext {
	usize stack_top;
	RBT_ErrorEntry stack[RBT_ERR_STACK_MAX];
	FILE *stream;
} RBT_ErrorContext;

static RBT_ErrorSeverity _min_severity = RBT_SEVERITY_INFO;
static _Thread_local struct RBT_ErrorContext _err_ctx = {};

static const char *const _severity_name[] = {
	"INFO",
	"WARN",
	"ERROR",
	"FATAL",
};

static const char *const _severity_color[] = {
	"\x1b[36m",
	"\x1b[33m",
	"\x1b[31m",
	"\x1b[35m",
};

void _err_push(
	RBT_ErrorSeverity severity,
	RBT_ErrorCode code,
	const char *func,
	const char *file,
	u32 line,
	const char *fmt,
	...
) {
	assert(func);
	assert(file);
	assert(fmt);

	if (severity < _min_severity) {
		return; // Skip storing errors below severity
	}

	if (_err_ctx.stack_top >= RBT_ERR_STACK_MAX) {
		rbt_err_flush();
	}

	RBT_ErrorEntry *entry = &_err_ctx.stack[_err_ctx.stack_top];
	_err_ctx.stack_top += 1;

	entry->severity = severity;
	entry->code = code;
	entry->func = func;
	entry->file = file;
	entry->line = line;
	entry->timestamp = time(nullptr);

	va_list args;
	va_start(args, fmt);
	vsnprintf(entry->msg, RBT_ERR_MESSAGE_MAX, fmt, args);
	va_end(args);
}

void rbt_err_flush(void) {
	if (_err_ctx.stack_top == 0) {
		return;
	}

	FILE *out = _err_ctx.stream ? _err_ctx.stream : stderr;

	fprintf(out, "\n[RBT] > Error Stack Dump:\n");
	fprintf(out, "=========================\n");

	for (usize i = 0; i < _err_ctx.stack_top; i += 1) {
		RBT_ErrorEntry *entry = &_err_ctx.stack[i];

		struct tm *ltime = localtime(&entry->timestamp);
		char timestamp_buf[16] = {};
		timestamp_buf[strftime(timestamp_buf, 16, "%T", ltime)] = '\0';

		const char *sev_name = _severity_name[entry->severity];
		const char *sev_color = _severity_color[entry->severity];

		fprintf(
			out, "#%02zu - [%s] %s%s\x1b[0m (%#02x)\n", i,
			timestamp_buf[0] ? timestamp_buf : "no-time", sev_color, sev_name, entry->code
		);
#ifndef NDEBUG
		fprintf(out, "    at %s:%u in %s()\n", entry->file, entry->line, entry->func);
#endif

		fprintf(out, "    %s\n", entry->msg);
	}
	fprintf(out, "    ---\n");

	fflush(out);
	_err_ctx.stack_top = 0;
}

void rbt_set_err_stream(FILE *stream) {
	memset(&_err_ctx, 0, sizeof(struct RBT_ErrorContext));
	_err_ctx.stream = stream;
}

const RBT_ErrorEntry *rbt_query_last_error(void) {
	if (_err_ctx.stack_top == 0)
		return nullptr;

	return &_err_ctx.stack[_err_ctx.stack_top - 1];
}
