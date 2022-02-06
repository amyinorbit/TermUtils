//===--------------------------------------------------------------------------------------------===
// printing.h - Various utility functions used for printing messages to a terminal
// This source is part of TermUtils
//
// Created on 2019-07-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef termutils_printing_h
#define termutils_printing_h
#include <stdio.h>

typedef enum {TERM_INFO, TERM_WARN, TERM_ERROR} term_filter_t;

void term_set_filter(term_filter_t minimum);

/// Reports an error to [stderr] with the given format string.
/// If [code] is not 0, exit(code) will be called.
void term_error(const char* program, int code, const char* format, ...);
void term_warn(const char* program, const char* format, ...);
void term_info(const char* program, const char* format, ...);


void term_print_usage(FILE* out, const char* program, const char** uses, int count);
void term_print_contact(FILE* out, const char* program, const char* email, const char* website);

#endif
