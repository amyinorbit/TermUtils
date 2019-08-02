//===--------------------------------------------------------------------------------------------===
// line.c - Line editing library
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/line.h>
#include <term/hexes.h> // Could be moved back to private headers
#include "string.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HistoryEntry {
    struct HistoryEntry* prev;
    struct HistoryEntry* next;
    char line[];
} HistoryEntry;

typedef struct {
    int key;
    LineBinding function;
    LineCMD defaultCmd; // if [function] is null, we return this
} BindingData;

struct Line {
    const char* prompt;
    int cursor;
    
    LineFunctions functions;
    String buffer;
    
    HistoryEntry* head;
    HistoryEntry* current;
};

#define CMD_NOTHING (LineCMD){kLineStay, 0}
#define CMD(action, param) (LineCMD){(action), (param)}

// MARK: - Terminal manipulation.

static void putChar(char c) {
    putchar(c);
}

static void putString(const char* str) {
    while(*str)
        putChar(*str++);
}

static int showChar(Line* line, char c) {
    if(IS_CTL(c)) {
        termColorFG(stdout, kTermBlack);
        putChar('^');
        putChar(DE_CTL(c));
        termColorFG(stdout, kTermDefault);
        return 2;
    }
    putChar(c & 0x7f);
    return 1;
}

static int showString(Line* line, const char* str) {
    int total = 0;
    while(*str)
        total += showChar(line, *str++);
    return total;
}

static void back(Line* line, LineAction mode) {
    if(mode == kLineMove && !line->cursor) return;
    if(IS_CTL(line->buffer.data[line->cursor-1])) {
        putChar('\b');
    }
    if(mode == kLineMove) line->cursor -= 1;
    putChar('\b');
}

static void backN(Line* line, LineAction mode, int n) {
    while(n--)
        back(line, mode);
}

static void forward(Line* line) {
    if(line->cursor >= line->buffer.count) return;
    showChar(line, line->buffer.data[line->cursor]);
    line->cursor += 1;
}

static void forwardN(Line* line, int n) {
    while(n--)
        forward(line);
}

// MARK: - typing handling

// re-print the line after the cursor, to make sure we don't have any remaining stray characters.
static LineCMD finishLine(Line* line) {
    int move = showString(line, &line->buffer.data[line->cursor]);
    putString("\e[0K");
    return CMD(kLineStay, -move);
}

static LineCMD insert(Line* line, int key) {
    stringInsert(&line->buffer, line->cursor, key & 0x00ff);
    showChar(line, key);
    line->cursor += 1;
    if(line->cursor == line->buffer.count) return CMD_NOTHING;
    return finishLine(line);
}

static LineCMD backspace(Line* line, int key) {
    if(!line->buffer.count || !line->cursor) return CMD_NOTHING;
    back(line, kLineMove);
    stringErase(&line->buffer, line->cursor, 1);
    return finishLine(line);
}

static LineCMD delete(Line* line, int key) {
    if(!line->buffer.count || line->cursor >= line->buffer.count) return CMD_NOTHING;
    stringErase(&line->buffer, line->cursor, 1);
    return finishLine(line);
}

static LineCMD clear(Line* line, int key) {
    line->cursor = 0;
    line->buffer.count = 0;
    if(line->buffer.capacity) line->buffer.data[0] = '\0';
    return CMD(kLineCancel, 0);
}

// MARK: - Built-in utils

static void showPrompt(const Line* line) {
    if(line->functions.printPrompt) {
        line->functions.printPrompt(line->prompt);
    } else {
        putString(line->prompt);
        putString("> ");
    }
}

static void reset(Line* line) {
    line->buffer.count = 0;
    if(line->buffer.capacity) line->buffer.data[0] = '\0';
    line->cursor = 0;
}

// MARK: - History Management
// TODO: we should save the current buffer when scrolling through the history

static void freeHistory(Line* line) {
    HistoryEntry* entry = line->head;
    while(entry) {
        HistoryEntry* prev = entry->prev;
        free(entry);
        entry = prev;
    }
}

static LineCMD historyPrev(Line* line, int key) {
    if(!line->current && !line->head) return CMD_NOTHING;
    
    HistoryEntry* saved = line->current;
    line->current = line->current ? line->current->prev : line->head;
    
    if(!line->current) {
        line->current = saved;
        return CMD_NOTHING;
    }
    
    stringSet(&line->buffer, line->current->line);
    return CMD(kLineRefresh, 0);
}

static LineCMD historyNext(Line* line, int key) {
    if(!line->current) return CMD_NOTHING;
    line->current = line->current->next;
    
    if(line->current) {
        stringSet(&line->buffer, line->current->line);
    } else {
        reset(line);
    }
    return CMD(kLineRefresh, 0);
}

// MARK: - Default bindings & binding dispatch
// TODO: this should probably get moved to the Line* object itself, once we add custom bindings

static const BindingData bindings[] = {
    {CTL('d'),          NULL,           CMD(kLineDone, 0)},
    {CTL('c'),          &clear,         CMD_NOTHING},
    {CTL('m'),          NULL,           CMD(kLineReturn, 0)},
    {CTL('l'),          NULL,           CMD(kLineRefresh, 0)},
    
    {KEY_BACKSPACE,     &backspace,     CMD_NOTHING},
    {KEY_DELETE,        &delete,        CMD_NOTHING},
    
    {CTL('b'),          NULL,           CMD(kLineMove, -1)},
    {KEY_ARROW_LEFT,    NULL,           CMD(kLineMove, -1)},
    {CTL('f'),          NULL,           CMD(kLineMove, 1)},
    {KEY_ARROW_RIGHT,   NULL,           CMD(kLineMove, 1)},
    
    // TODO: implement history commands
    {CTL('p'),          &historyPrev,   CMD_NOTHING},
    {KEY_ARROW_UP,      &historyPrev,   CMD_NOTHING},
    {CTL('n'),          &historyNext,   CMD_NOTHING},
    {KEY_ARROW_DOWN,    &historyNext,   CMD_NOTHING},
    
    {0,                 NULL,           CMD_NOTHING},
};

static LineCMD dispatch(Line* line, int key) {
    
    for(int i = 0; bindings[i].key != 0; ++i) {
        if(bindings[i].key != key) continue;
        return bindings[i].function ?
            bindings[i].function(line, key) :
            bindings[i].defaultCmd;
    }
    return insert(line, key);
}

// MARK: - Public Line API

Line* lineNew(const LineFunctions* functions) {
    Line* line = malloc(sizeof(Line));
    assert(line && "line editor allocation failed");
    line->prompt = "";
    line->cursor = 0;
    
    line->functions = *functions;
    stringInit(&line->buffer);
    
    line->head = NULL;
    line->current = NULL;
    
    return line;
}

void lineDealloc(Line* line) {
    assert(line && "cannot deallocate a null line");
    stringDeinit(&line->buffer);
    freeHistory(line);
    free(line);
}

void lineRunCommand(LineAction action, void* param) {
    
}

void lineSetPrompt(Line* line, const char* prompt) {
    assert(line && "cannot set prompt on null line editor");
    assert(prompt && "prompt cannot be null");
    line->prompt = prompt;
}

// TODO: read in, dispatch, etc;
char* lineGet(Line* line) {
    hexesStartRawMode();
    reset(line);
    
    char* result = NULL;
    putString("\r\e[2K");
    showPrompt(line);
    for(;;) {
        int key = hexesGetKeyRaw();
        LineCMD cmd = dispatch(line, key);
        
        switch(cmd.action) {
        case kLineStay:
            if(cmd.param >= 0) break;
            if(cmd.param < 0) backN(line, kLineStay, -cmd.param);
            break;
            
        case kLineDone:
            showChar(line, key);
            result = NULL;
            goto done;
            
        case kLineReturn:
            putString("\n\r");
            stringAppend(&line->buffer, '\n');
            result = stringTake(&line->buffer);
            goto done;
            
        case kLineMove:
            if(cmd.param < 0) backN(line, kLineMove, -cmd.param);
            if(cmd.param > 0) forwardN(line, cmd.param);
            break;
            
        case kLineRefresh:
            putString("\r\e[2K");
            showPrompt(line);
            showString(line, line->buffer.data);
            break;
            
        case kLineCancel:
            showChar(line, key);
            putString("\r\n");
            showPrompt(line);
            break;
            
        }
    }
    
done:
    hexesStopRawMode();
    fflush(stdout);
    if(result) lineHistoryAdd(line, result);
    return result;
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

void lineHistoryAdd(Line* line, const char* data) {
    int length = strlen(data);
    HistoryEntry* entry = malloc(sizeof(HistoryEntry) + (length + 1) * sizeof(char));
    memcpy(entry->line, data, length);
    entry->line[length] = '\0';
    strip(entry->line);
    
    entry->next = NULL;
    if(line->head) {
        entry->prev = line->head;
        line->head->next = entry;
    } else {
        entry->prev = NULL;
    }
    line->head = entry;
}
