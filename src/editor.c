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

static inline int min(int a, int b) { return a < b ? a : b; }

static inline void moveLineStart(Editor* e) {
    if(!e->cursor.x) return;
    printf("\033[%dD", e->cursor.x);
    e->cursor.x = 0;
}

static inline void termUp(int n) { if(n) printf("\033[%dA", n); }
static inline void termDown(int n) { if(n) printf("\033[%dB", n); }
static inline void termRight(int n) { if(n) printf("\033[%dC", n); }
static inline void termLeft(int n) { if(n) printf("\033[%dD", n); }

static inline void moveNextLine(Editor* e) {
    termDown(1);
    e->cursor.y += 1;
    moveLineStart(e);
}

static inline void up(Editor* e, int n) {
    termUp(n);
    e->cursor.y -= n;
}

static inline void down(Editor* e, int n) {
    termDown(n);
    e->cursor.y += n;
}

static inline void right(Editor* e, int n) {
    termRight(n);
    e->cursor.x += n;
}

static inline void left(Editor* e, int n) {
    if(!e->cursor.x) return;
    termLeft(n);
    e->cursor.x -= n;
}

static void ensureBuffer(Editor* e, int count) {
    count += 1; // To account for the trailing \0
    if(count <= e->capacity) return;
    while(e->capacity < count)
        e->capacity = e->capacity ? e->capacity * 2 : 256;
    e->buffer = realloc(e->buffer, e->capacity * sizeof(char));
}

static inline void insert(Editor* e, int offset, char c) {
    assert(offset <= e->count && "Invalid insertion offset");
    ensureBuffer(e, e->count + 1);
    int moveCount = e->count - offset;
    memmove(&e->buffer[offset+1], &e->buffer[offset], moveCount * sizeof(char));
    e->buffer[offset] = c;
    e->count += 1;
    e->buffer[e->count] = '\0';
}

static inline void erase(Editor* e, int offset) {
    assert(offset <= e->count && "Invalid insertion offset");
    int moveCount = e->count - (offset + 1);
    memmove(&e->buffer[offset], &e->buffer[offset+1], moveCount * sizeof(char));
    e->count -= 1;
    e->buffer[e->count] = '\0';
}

static void editorBackspace(Editor* e) {
    // TODO: implementation
    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;
    
    EditorLine* line = &e->lines[y];
    int offset = line->offset + (x - 1);
    
    if(offset < 0) return;
    
    char tbd = e->buffer[offset];
    
    if(tbd == '\n') {
        assert(y > 0 && "If wer're deleting \\n, we shouldn't be on the first line");
        assert(x == 0 && "We should be at the start of the line");
        erase(e, offset);
        
        EditorLine* prev = &e->lines[y-1];
        right(e, prev->count);
        up(e, 1);
        prev->count += line->count;
        
        e->lineCount -= 1;
        for(int i = y; i < e->lineCount; ++i) {
            e->lines[i] = e->lines[i+1];
        }
        
    } else {
        erase(e, offset);
        line->count -= 1;
        left(e, 1);
    }
    for(int i = (e->offset.y + e->cursor.y)+1; i < e->lineCount; ++i) e->lines[i].offset -= 1;
}

static void editorInsert(Editor* e, char c) {
    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;
    
    EditorLine* line = &e->lines[y];
    int offset = line->offset + x;
    insert(e, offset, c);
    right(e, 1);
    
    // Then we need to move lines. The only things that change are offsets
    // (and count for the current line)
    line->count += 1;
    
    for(int i = y+1; i < e->lineCount; ++i) e->lines[i].offset += 1;
}

// TODO: Don't memmove here -- we need to keep the oldcount to, well, the old one
static void editorNewline(Editor* e) {
    assert(e->lineCount + 1 <= TERM_EDITOR_MAX_LINES);
    // TODO: implementation
    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;
    
    EditorLine* line = &e->lines[y];
    int offset = line->offset + x;
    insert(e, offset, '\n');
    
    
    EditorLine next = (EditorLine){.offset=line->offset + x + 1, .count=line->count - x};
    line->count = x;
    
    for(int i = e->lineCount; i > y+1; --i) {
        e->lines[i].offset = e->lines[i-1].offset + 1;
        e->lines[i].count = e->lines[i-1].count;
    }
    e->lines[y+1] = next;
    e->lineCount += 1;
    moveNextLine(e);
}

// MARK: - "public" API

void termEditorInit(Editor* e) {
    e->cursor = (Coords){0, 0};
    e->offset = (Coords){0, 0};
    
    e->prompt = "";
    e->promptLength = 0;
    
    e->lineCount = 1;
    e->count = 0;
    e->capacity = 0;
    e->buffer = NULL;
    
    for(int i = 0; i < TERM_EDITOR_MAX_LINES; ++i) {
        e->lines[i].count = 0;
        e->lines[i].offset = 0;
    }
}

void termEditorDeinit(Editor* e) {
    if(e->buffer) free(e->buffer);
    termEditorInit(e);
}

char* termEditorFlush(Editor* e) {
    char* data = e->buffer;
    e->buffer = NULL;
    e->count = 0;
    e->capacity = 0;
    return data;
}

static void scrollLeft(Editor* e, int over) {
    int dist = 0;
    while(dist < over) dist += 20;
    dist = min(dist, e->offset.x);
    
    e->offset.x -= dist;
    right(e, dist);
}

static void scrollRight(Editor* e, int over) {
    int dist = 0;
    while(dist < over) dist += 20;
    
    e->offset.x += dist;
    left(e, dist);
}

static void keepInViewX(Editor* e) {
    int minX = 2;
    int maxX = tcols() - (e->promptLength + 2);
    
    if(e->cursor.x > maxX) {
        scrollRight(e, e->cursor.x - maxX);
    } else if(e->offset.x && e->cursor.x < minX) {
        scrollLeft(e, minX - e->cursor.x);
    }
    
}

static void termEditorCLS(Editor* e) {
    int nx = tcols();
    int ny = min(trows(), 10);
    
    Coords screen = (Coords){
        e->promptLength + e->cursor.x,
        e->cursor.y - e->offset.y
    };
    
    Coords current = (Coords){0, 0};
    
    termLeft(screen.x);
    termUp(screen.y);
    
    for(int i = e->offset.y; i < min(e->lineCount + 1, ny); ++i) {
        for(int i = 0; i < nx; ++i) putchar(' ');
        putchar('\n');
        current.y += 1;
    }
    
    termLeft(current.x);
    termUp(current.y);
}

// TODO: this isn't really the nicest way to do things, we could probably memcpy a lot of this.
void termEditorReplace(Editor* e, const char* data) {
    termEditorClear(e);
    while(*data) {
        char c = *data++;
        if(c == '\n') editorNewline(e);
        else editorInsert(e, c);
    }
}

void termEditorClear(Editor* e) {
    termEditorCLS(e);
    e->cursor.x = 0;
    e->cursor.y = 0;
    e->offset.x = 0;
    e->offset.y = 0;
    
    for(int i = 0; i < e->lineCount; ++i) {
        e->lines[i].count = 0;
    }
    e->lineCount = 1;
}

void termEditorRender(Editor* e) {
    
    int nx = tcols();
    int ny = min(trows(), 10);
    
    Coords screen = (Coords){
            e->promptLength + e->cursor.x,
            e->cursor.y - e->offset.y
        };
    Coords current = (Coords){0, 0};
    
    termEditorCLS(e);
    
    // the simple bit: we print the lines!
    for(int i = e->offset.y; i < min(e->lineCount, ny); ++i) {
        current.x = 0;
        EditorLine line = e->lines[i];
        if(line.count) {
            printf("%.*s", min(line.count, nx - e->promptLength),
                   &e->buffer[line.offset+e->offset.x]);
        }
        putchar('\n');
        current.y += 1;
    }
    
    termLeft(current.x);
    termUp(current.y);
    
    termRight(screen.x);
    termDown(screen.y);
}

void termEditorLeft(Editor* e) {
    if(e->cursor.x <= 0) return;
    left(e, 1);
}

void termEditorRight(Editor* e) {
    if(e->cursor.x + e->offset.x >= e->lines[e->cursor.y].count) return;
    right(e, 1);
}

void termEditorUp(Editor* e) {
    up(e, 1);
    
    int max = e->lines[e->cursor.y].count;
    if(e->cursor.x <= max) return;
    left(e, e->cursor.x - max);
}

void termEditorDown(Editor* e) {
    down(e, 1);
    
    int max = e->lines[e->cursor.y].count;
    if(e->cursor.x <= max) return;
    left(e, e->cursor.x - max);
}

static EditorStatus processEscape(Editor* e) {
    if(getch() != 91) return kTermEditorOK;
    switch(getch()) {
        
    case 65:
        if(e->cursor.y + e->offset.y == 0) return kTermEditorTop;
        termEditorUp(e);
        return kTermEditorOK;
        
    case 66:
        if(e->cursor.y + e->offset.y == e->lineCount - 1) return kTermEditorBottom;
        termEditorDown(e);
        return kTermEditorOK;
        
    // right arrow
    case 67:
        termEditorRight(e);
        return kTermEditorOK;
        
    // left arrow
    case 68:
        termEditorLeft(e);
        return kTermEditorOK;
    }
    return kTermEditorOK;
}

EditorStatus termEditorUpdate(Editor* e, char c) {
    EditorStatus status = kTermEditorOK;
    
    switch(c) {
    case '\n':
        editorNewline(e);
        status = kTermEditorReturn;
        break;
        
    case 0x7f:
        editorBackspace(e);
        status = kTermEditorOK;
        break;
        
    case 0x04:
        return kTermEditorDone;
        
    case 0x1b:
        status = processEscape(e);
        break;
        
    default:
        editorInsert(e, c);
        status = kTermEditorOK;
        break;
    }
    keepInViewX(e);
    return status;
}

