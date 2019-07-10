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

typedef enum {kTermInfo, kTermWarn, kTermError} TermLevel;

void termFilter(TermLevel minimum);

/// Reports an error to [stderr] with the given format string.
/// If [code] is not 0, exit(code) will be called.
void termError(const char* program, int code, const char* format, ...);
void termWarn(const char* program, const char* format, ...);
void termInfo(const char* program, const char* format, ...);


void termPrintUsage(FILE* out, const char* program, const char** uses, int count);
void termPrintBugreports(FILE* out, const char* program, const char* email, const char* website);

#endif