//===--------------------------------------------------------------------------------------------===
// printing.c - Implements error reporting
// This source is part of TermUtils
//
// Created on 2019-07-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/printing.h>
#include <term/colors.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static TermLevel level__ = kTermWarn;

void termFilter(TermLevel minimum) {
    level__ = minimum;
}

static inline void printPreamble(const char* program, const char* what, TermColor color) {
    termReset(stderr);
    fprintf(stderr, "%s: ", program);
    termBold(stderr, true);
    termColorFG(stderr, color);
    fprintf(stderr, "%s:", what);
    termReset(stderr);
    fprintf(stderr, " ");
}

/// Reports an error to [stderr] with the given format string.
/// If [code] is not 0, exit(code) will be called.
void termError(const char* program, int code, const char* format, ...) {
    printPreamble(program, "error", kTermRed);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    if(code) exit(code);
}

void termWarn(const char* program, const char* format, ...) {
    if(level__ > kTermWarn) return;
    printPreamble(program, "warning", kTermMagenta);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

void termInfo(const char* program, const char* format, ...) {
    if(level__ > kTermInfo) return;
    printPreamble(program, "info", kTermDefault);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

void termPrintUsage(FILE* out, const char* program, const char** uses, int count) {
    termReset(out);
    termBold(out, true);
    fprintf(out, "Usage: ");
    termReset(out);
    for(int i = 0; i < count; ++i) {
        if(i > 0) fprintf(out, "  or   ");
        fprintf(out, "%s %s\n", program, uses[i]);
    }
}

void termPrintBugreports(FILE* out, const char* program, const char* email, const char* website) {
    termReset(out);
    if(email) fprintf(out, "Report bugs to: %s\n", email);
    if(website) fprintf(out, "%s home page: <%s>\n", program, website);
}
