//===--------------------------------------------------------------------------------------------===
// editor.c - CLI editor implementation
// This source is part of TermUtils
//
// Created on 2019-07-24 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "editor.h"
#include "shims.h"
#include <term/colors.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct EditorLine {
    int offset;
    int count;
} EditorLine;

typedef struct {
    int x, y;
} Coords;

typedef struct Editor {
    const char* prompt;
    int promptLength;
    
    Coords cursor;
    Coords offset;
    
    int lineCount;
    int lineCapacity;
    EditorLine* lines;
    
    char* buffer;
    int count;
    int capacity;
} Editor;

static Editor E;

static inline int min(int a, int b) { return a < b ? a : b; }

static inline void up(int n) {
    termUp(n);
    E.cursor.y -= n;
}

static inline void down(int n) {
    termDown(n);
    E.cursor.y += n;
}

static inline void right(int n) {
    termRight(n);
    E.cursor.x += n;
}

static inline void left(int n) {
    if(!E.cursor.x) return;
    termLeft(n);
    E.cursor.x -= n;
}

static void ensureLines(int count) {
    if(count <= E.lineCapacity) return;
    while(E.lineCapacity < count)
        E.lineCapacity = E.lineCapacity ? E.lineCapacity * 2 : 32;
    E.lines = realloc(E.lines, E.lineCapacity * sizeof(EditorLine));
}

static void ensureBuffer(int count) {
    count += 1; // To account for the trailing \0
    if(count <= E.capacity) return;
    while(E.capacity < count)
        E.capacity = E.capacity ? E.capacity * 2 : 256;
    E.buffer = realloc(E.buffer, E.capacity * sizeof(char));
}

static inline void insert(int offset, char c) {
    assert(offset <= E.count && "Invalid insertion offset");
    ensureBuffer(E.count + 1);
    int moveCount = E.count - offset;
    memmove(&E.buffer[offset+1], &E.buffer[offset], moveCount * sizeof(char));
    E.buffer[offset] = c;
    E.count += 1;
    E.buffer[E.count] = '\0';
}

static inline void erase(int offset) {
    assert(offset <= E.count && "Invalid insertion offset");
    int moveCount = E.count - (offset + 1);
    memmove(&E.buffer[offset], &E.buffer[offset+1], moveCount * sizeof(char));
    E.count -= 1;
    E.buffer[E.count] = '\0';
}

static void editorBackspace() {
    // TODO: implementation
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;
    
    EditorLine* line = &E.lines[y];
    int offset = line->offset + (x - 1);
    
    if(offset < 0) return;
    
    char tbd = E.buffer[offset];
    
    if(tbd == '\n') {
        assert(y > 0 && "If we're deleting \\n, we shouldn't be on the first line");
        assert(x == 0 && "We should be at the start of the line");
        erase(offset);
        
        EditorLine* prev = &E.lines[y-1];
        right(prev->count);
        up(1);
        prev->count += line->count;
        
        E.lineCount -= 1;
        for(int i = y; i < E.lineCount; ++i) {
            E.lines[i] = E.lines[i+1];
        }
        
    } else {
        erase(offset);
        line->count -= 1;
        left(1);
    }
    for(int i = (E.offset.y + E.cursor.y)+1; i < E.lineCount; ++i) E.lines[i].offset -= 1;
}

static void editorInsert(char c) {
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;
    
    EditorLine* line = &E.lines[y];
    int offset = line->offset + x;
    insert(offset, c);
    right(1);
    
    // Then we need to move lines. The only things that change are offsets
    // (and count for the current line)
    line->count += 1;
    
    for(int i = y+1; i < E.lineCount; ++i) E.lines[i].offset += 1;
}

// TODO: Don't memmove here -- we need to keep the oldcount to, well, the old one
static void editorNewline() {
    ensureLines(E.lineCount + 1);
    // TODO: implementation
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;
    
    EditorLine* line = &E.lines[y];
    int offset = line->offset + x;
    insert(offset, '\n');
    
    
    EditorLine next = (EditorLine){.offset=line->offset + x + 1, .count=line->count - x};
    line->count = x;
    
    for(int i = E.lineCount; i > y+1; --i) {
        E.lines[i].offset = E.lines[i-1].offset + 1;
        E.lines[i].count = E.lines[i-1].count;
    }
    E.lines[y+1] = next;
    E.lineCount += 1;
    
    E.cursor.x = 0;
    E.cursor.y += 1;
    putchar('\n');
}

// MARK: - "public" API

void termEditorInit(const char* prompt) {
    E.cursor = (Coords){0, 0};
    E.offset = (Coords){0, 0};
    
    E.prompt = prompt;
    E.promptLength = strlen(prompt);
    
    E.lineCount = 1;
    E.lineCapacity = 0;
    ensureLines(1);
    
    E.count = 0;
    E.capacity = 0;
    E.buffer = NULL;
    
    E.lines[0] = (EditorLine){.count = 0, .offset = 0};
}

void termEditorDeinit() {
    if(E.buffer) free(E.buffer);
    if(E.lines) free(E.lines);
    E.lines = NULL;
    E.lineCount = E.lineCapacity = 0;
    E.buffer = NULL;
    E.count = E.capacity = 0;
    E.prompt = "";
    E.promptLength = 0;
    E.cursor = (Coords){0, 0};
    E.offset = (Coords){0, 0};
}

char* termEditorFlush() {
    char* data = E.buffer;
    E.buffer = NULL;
    E.count = 0;
    E.capacity = 0;
    return data;
}

static void scrollLeft(int over) {
    int dist = 0;
    while(dist < over) dist += 20;
    dist = min(dist, E.offset.x);
    
    E.offset.x -= dist;
    right(dist);
}

static void scrollRight(int over) {
    int dist = 0;
    while(dist < over) dist += 20;
    
    E.offset.x += dist;
    left(dist);
}

static void keepInViewX() {
    int minX = 2;
    int maxX = tcols() - (E.promptLength + 2);
    
    if(E.cursor.x > maxX) {
        scrollRight(E.cursor.x - maxX);
    } else if(E.offset.x && E.cursor.x < minX) {
        scrollLeft(minX - E.cursor.x);
    }
}

static void termEditorCLS() {
    int ny = min(trows(), 10);
    
    Coords screen = (Coords){
        E.promptLength + E.cursor.x,
        E.cursor.y - E.offset.y
    };
    
    Coords current = (Coords){0, 0};
    
    termLeft(screen.x);
    termUp(screen.y);
    
    for(int i = E.offset.y; i < min(E.lineCount + 1, ny); ++i) {
        termClearLine();
        putchar('\n');
        current.y += 1;
    }
    
    termLeft(current.x);
    termUp(current.y);
    fflush(stdout);
}

// TODO: this isn't really the nicest way to do things, we could probably memcpy a lot of this.
void termEditorReplace(const char* data) {
    termEditorClear();
    while(*data) {
        char c = *data++;
        if(c == '\n') editorNewline();
        else editorInsert(c);
    }
}

void termEditorClear() {
    termEditorCLS();
    E.cursor.x = 0;
    E.cursor.y = 0;
    E.offset.x = 0;
    E.offset.y = 0;
    
    for(int i = 0; i < E.lineCount; ++i) {
        E.lines[i].count = 0;
    }
    E.count = 0;
    E.lineCount = 1;
}

void termEditorRender() {
    
    int nx = tcols();
    int ny = trows();
    
    printf("\033[H");
    termClear();
    fflush(stdout);
    
    Coords screen = (Coords){
        E.cursor.x + 4,
        E.cursor.y + 1 // For the prompt line at the top
    };
    
    termColorBG(stdout, kTermBlack);
    termColorFG(stdout, kTermBlue);
    termBold(stdout, true);
    printf("%-*s\n", nx, E.prompt);
    termReset(stdout);
    
    // the simple bit: we print the lines!
    for(int i = E.offset.y; i < min(E.lineCount, ny-1); ++i) {
        
        // termColorFG(stdout, kTermBlue);
        // if(i) printf("%*s", E.promptLength, "... ");
        // else printf("%s", E.prompt);
        // termColorFG(stdout, kTermDefault);
        
        EditorLine line = E.lines[i];
        
        termColorFG(stdout, kTermBlack);
        printf("%3d ", i+1);
        termColorFG(stdout, kTermDefault);
        
        printf("%.*s", min(line.count, nx - 4),
               &E.buffer[line.offset+E.offset.x]);
        putchar('\n');
    }
    
    printf("\033[H");
    termRight(screen.x);
    termDown(screen.y);
    fflush(stdout);
}

void termEditorLeft() {
    if(E.cursor.x <= 0) return;
    left(1);
}

void termEditorRight() {
    if(E.cursor.x + E.offset.x >= E.lines[E.cursor.y].count) return;
    right(1);
}

void termEditorUp() {
    up(1);
    
    int max = E.lines[E.cursor.y].count;
    if(E.cursor.x <= max) return;
    left(E.cursor.x - max);
}

void termEditorDown() {
    down(1);
    
    int max = E.lines[E.cursor.y].count;
    if(E.cursor.x <= max) return;
    left(E.cursor.x - max);
}

static EditorStatus processEscape() {
    if(getch() != 91) return kTermEditorOK;
    switch(getch()) {
        
    case 65:
        if(E.cursor.y + E.offset.y == 0) return kTermEditorTop;
        termEditorUp();
        return kTermEditorOK;
        
    case 66:
        if(E.cursor.y + E.offset.y == E.lineCount - 1) return kTermEditorBottom;
        termEditorDown();
        return kTermEditorOK;
        
    // right arrow
    case 67:
        termEditorRight();
        return kTermEditorOK;
        
    // left arrow
    case 68:
        termEditorLeft();
        return kTermEditorOK;
    }
    return kTermEditorOK;
}

EditorStatus termEditorUpdate(char c) {
    EditorStatus status = kTermEditorOK;
    
    switch(c) {
    case '\n':
        editorNewline();
        status = kTermEditorReturn;
        break;
        
    case 0x7f:
        editorBackspace();
        status = kTermEditorOK;
        break;
        
    case 0x04:
        return kTermEditorDone;
        
    case 0x1b:
        status = processEscape();
        break;
        
    default:
        editorInsert(c);
        status = kTermEditorOK;
        break;
    }
    keepInViewX();
    return status;
}

