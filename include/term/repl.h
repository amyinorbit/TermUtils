//===--------------------------------------------------------------------------------------------===
// repl.h - REPL interface utilities
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef termutils_repl_h
#define termutils_repl_h
#include <stdbool.h>
#include <stdio.h>

#ifndef TERM_MAX_HISTORY
#define TERM_MAX_HISTORY 32
#endif

typedef struct TermREPL {
    char* history[TERM_MAX_HISTORY];
} TermREPL;

void termREPLInit(TermREPL* repl);
void termREPLDeinit(TermREPL* repl);

const char* termREPL(const char* prompt, TermREPL* repl);

#endif