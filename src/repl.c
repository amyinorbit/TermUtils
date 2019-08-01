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

#ifdef _WIN32
#else
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

typedef enum REPLAction {
    REPL_SUBMIT,
    REPL_DONE,
    REPL_CLEAR,
    REPL_DONOTHING,
} REPLAction;

typedef REPLAction (*REPLCallback)(int);

typedef struct {
    int key;
    REPLCallback function;
} REPLCommand;

struct {
    int cursor;
    int offsets[32];
    String buffer;
} Line;

static const char* currentPrompt = "";

static REPLAction replLeft(int key) {
    if(!Line.cursor) return REPL_DONOTHING;
    putchar('\b');
    Line.cursor -= 1;
    return REPL_DONOTHING;
}

static REPLAction replRight(int key) {
    if(Line.cursor >= Line.buffer.count) return REPL_DONOTHING;
    putchar(Line.buffer.data[Line.cursor]);
    Line.cursor += 1;
    return REPL_DONOTHING;
}

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

// MARK: - Printing functions

void showPrompt() {
    printf("%s> ", currentPrompt);
}

void replPut(int c) {
    putchar(c);
}

void replPuts(const char* str) {
    while(*str)
        replPut(*(str++));
}

// MARK: - Terminal Handling

void startREPL() {
    hexesStartRawMode();
    stringInit(&Line.buffer);
}

void stopREPL() {
    hexesStopRawMode();
    stringDeinit(&Line.buffer);
}

// MARK: - Insertion and deletion

static void lineCap() {
    replPuts(&Line.buffer.data[Line.cursor]);
    replPuts("\e[0K");
    int move = (Line.buffer.count - Line.cursor);
    while(move--) replPut('\b');
}

static void replErase(int move) {
    if(Line.cursor - move >= Line.buffer.count || Line.cursor - move < 0) return;
    while(move--) replLeft(0);
    stringErase(&Line.buffer, Line.cursor, 1);
    lineCap();
}

static inline REPLAction replBackspace(int key) {
    replErase(1);
    return REPL_DONOTHING;
}

static inline REPLAction replDelete(int key) {
    replErase(0);
    return REPL_DONOTHING;
}

static void replRestart() {
    Line.cursor = 0;
    Line.buffer.count = 0;
    if(Line.buffer.capacity) Line.buffer.data[0] = '\0';
    showPrompt();
}

static REPLAction replClear(int key) {
    replPuts("^C\n\r");
    Line.cursor = 0;
    Line.buffer.count = 0;
    if(Line.buffer.capacity) Line.buffer.data[0] = '\0';
    showPrompt();
    return REPL_CLEAR;
}

static REPLAction replReturn(int key) {
    replPuts("\n\r");
    return REPL_SUBMIT;
}

static REPLAction replEnd(int key) {
    replPuts("\n\r");
    return REPL_DONE;
}

#ifndef CTRL
#define CTRL(c) ((c)&037)
#endif

static const REPLCommand dispatch[] = {
    {CTRL('d'), replEnd},
    {CTRL('c'), replClear},
    {CTRL('h'), replBackspace},
    {CTRL('m'), replReturn},
    {KEY_BACKSPACE, replBackspace},
    {KEY_DELETE, replDelete},
    {KEY_ARROW_LEFT, replLeft},
    {KEY_ARROW_RIGHT, replRight},
    // {CTRL('d'), }
};

static REPLAction defaultCMD(int key) {
    stringInsert(&Line.buffer, Line.cursor, key & 0x00ff);
    putchar(key & 0x00ff);
    Line.cursor += 1;
    return REPL_DONOTHING;
}

const REPLCallback findCommand(int c) {
    static const int count = sizeof(dispatch) / sizeof(dispatch[0]);
    for(int i = 0; i < count; ++i) {
        if(dispatch[i].key == c) return dispatch[i].function;
    }
    return defaultCMD;
}

char* termREPL(TermREPL* repl, const char* prompt) {
    currentPrompt = prompt;
    Line.cursor = 0;
    startREPL();
    showPrompt();
    
    char* result = NULL;
    for(;;) {
        int key = hexesGetKeyRaw();
        REPLAction action = findCommand(key)(key);
        
        switch(action) {
        case REPL_SUBMIT:
            result = stringTake(&Line.buffer);
            goto done;
            
        case REPL_DONE:
            result = NULL;
            goto done;
            
        case REPL_CLEAR:
        case REPL_DONOTHING:
            break;
        }
    }
done:
    stopREPL();
    fflush(stdout);
    return result;
    // stopREPL();
}
