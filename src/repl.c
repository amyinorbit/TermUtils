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

struct {
    int cursor;
    int offsets[32];
    String buffer;
} Line;

static const char* currentPrompt = "";

static void replLeft() {
    if(!Line.cursor) return;
    putchar('\b');
    Line.cursor -= 1;
}

static void replRight() {
    if(Line.cursor >= Line.buffer.count) return;
    putchar(Line.buffer.data[Line.cursor]);
    Line.cursor += 1;
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
    replPut('\0');
}

// MARK: - Terminal Handling

bool setTTY(bool reset) {
#ifdef _WIN32
    return false;
#else
    static struct termios saved;
    
    if(!reset) {
        struct termios raw;
        if (!isatty(STDIN_FILENO)) return false;
        // atexit(editorAtExit);
        if (tcgetattr(STDIN_FILENO,&saved) == -1) return false;

        raw = saved;
        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
        raw.c_oflag &= ~(OPOST);
        raw.c_cflag |= (CS8);
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) return false;
        
    } else {
        if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&saved) < 0) return false;
    }
    return true;
#endif
}

void startREPL() {
    // rl_ttyset(0);
    // setTTY(false);
    stringInit(&Line.buffer);
}

void stopREPL() {
    // rl_ttyset(1);
    // setTTY(true);
    stringDeinit(&Line.buffer);
}

void replGet() {
    
}

// MARK: - Insertion and deletion

static void lineCap() {
    replPuts(&Line.buffer.data[Line.cursor]);
    replPuts("\e[0K");
    int move = (Line.buffer.count - Line.cursor);
    while(move--) replPut('\b');
}

static void replBackspace() {
    if(!Line.cursor) return;
    replLeft();
    stringErase(&Line.buffer, Line.cursor, 1);
    lineCap();
}

static void replErase(int move) {
    if(Line.cursor - move >= Line.buffer.count || Line.cursor - move < 0) return;
    while(move--) replLeft();
    stringErase(&Line.buffer, Line.cursor, 1);
    lineCap();
}

char* termREPL(TermREPL* repl, const char* prompt) {
    currentPrompt = prompt;
    Line.cursor = 0;
    startREPL();
    showPrompt();
    for(;;) {
        int key = hexesGetKey();
        switch(key) {
            
        case KEY_ARROW_UP:
        case KEY_ARROW_DOWN:
            break;
            
            
        case KEY_ARROW_LEFT:
            replLeft();
            break;
            
        case KEY_ARROW_RIGHT:
            replRight();
            break;
            
        case KEY_BACKSPACE:
            replErase(1);
            break;
            
        case KEY_DELETE:
            replErase(0);
            break;
            
        case KEY_RETURN:
        case '\n':
            break;
            
        case KEY_CTRL_C:
            goto stop;
            break;
            
        case KEY_CTRL_D:
            return NULL;
            break;
            
            
            
        default:
            stringInsert(&Line.buffer, Line.cursor, key & 0x00ff);
            putchar(key & 0x00ff);
            Line.cursor += 1;
            break;
        }
    }
stop:
    return Line.buffer.data;
    // stopREPL();
}
