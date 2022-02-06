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

static term_filter_t level__ = TERM_WARN;

void term_set_filter(term_filter_t minimum) {
    level__ = minimum;
}

static inline void print_preamble(const char* program, const char* what, term_color_t color) {
    term_style_reset(stderr);
    fprintf(stderr, "%s: ", program);
    term_set_bold(stderr, true);
    term_set_fg(stderr, color);
    fprintf(stderr, "%s:", what);
    term_style_reset(stderr);
    fprintf(stderr, " ");
}

/// Reports an error to [stderr] with the given format string.
/// If [code] is not 0, exit(code) will be called.
void term_error(const char* program, int code, const char* format, ...) {
    print_preamble(program, "error", TERM_RED);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    if(code) exit(code);
}

void term_warn(const char* program, const char* format, ...) {
    if(level__ > TERM_WARN) return;
    print_preamble(program, "warning", TERM_MAGENTA);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

void term_info(const char* program, const char* format, ...) {
    if(level__ > TERM_INFO) return;
    print_preamble(program, "info", TERM_DEFAULT);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
}

void term_print_usage(FILE* out, const char* program, const char** uses, int count) {
    term_style_reset(out);
    term_set_bold(out, true);
    fprintf(out, "Usage: ");
    term_style_reset(out);
    for(int i = 0; i < count; ++i) {
        if(i > 0) fprintf(out, "  or   ");
        fprintf(out, "%s %s\n", program, uses[i]);
    }
}

void term_print_contact(FILE* out, const char* program, const char* email, const char* website) {
    term_style_reset(out);
    if(email) fprintf(out, "Report bugs to: %s\n", email);
    if(website) fprintf(out, "%s home page: <%s>\n", program, website);
}
