//===--------------------------------------------------------------------------------------------===
// repl.c - REPL interface utilities
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/repl.h>
#include <term/hexes.h>
#include "string.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>


struct {
    int cursor;
    String buffer;
} Line;

void termREPLInit(TermREPL* repl) {
    assert(repl && "cannot initialise a null REPL");
    repl->historyCount = 0;
    for(int i = 0; i < TERM_MAX_HISTORY; ++i) {
        repl->history[i] = NULL;
    }
    
}

void termREPLDeinit(TermREPL* repl) {
    for(char** str = repl->history; str != repl->history + TERM_MAX_HISTORY; ++str) {
        if(!*str) break;
        free(*str);
        *str = NULL;
    }
}

static char* strip(char* data) {
    char* end = data;
    while(*end) end += 1;
    if(*end != '\n' && *end != ' ' && *end != '\0') return data;
    
    for(; end > data; --end) {
        if(end[-1] != '\n' && end[-1] != ' ') break;
    }
    *end = '\0';
    return data;
}

char* termREPL(TermREPL* repl, const char* prompt) {
    for(;;) {
        int key = hexesGetCh();
        switch(key) {
        case KEY_ARROW_LEFT:
            if(Line.cursor < Line.buffer.count) {
                hexesCursorLeft(1);
                Line.cursor += 1;
            }
            break;
            
        case KEY_ARROW_RIGHT:
            if(Line.cursor < Line.buffer.count) {
                hexesCursorRight(1);
                Line.cursor += 1;
            }
            break;
            
        default:
            stringInsert(&Line.buffer, Line.cursor, key & 0x00ff);
            Line.cursor += 1;
            break;
        }
    }
}
