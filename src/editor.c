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

static inline int max(int a, int b) { return a > b ? a : b; }
static inline int min(int a, int b) { return a < b ? a : b; }

static void editorInitLine(Editor* e, int i) {
    e->lines[i].count = 0;
    e->lines[i].capacity = 0;
    e->lines[i].data = NULL;
}

static void editorDeinitLine(Editor* e, int i) {
    if(e->lines[i].data) free(e->lines[i].data);
    editorInitLine(e, i);
}

static void editorEnsureLine(EditorLine* line, int count) {
    if(line->capacity > count) return;
    while(count > line->capacity)
        line->capacity = line->capacity ? line->capacity * 2 : 32;
    line->data = realloc(line->data, line->capacity * sizeof(char));
}

static void editorBackspace(Editor* e) {
    
    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;
    
    EditorLine* line = &e->lines[y];
    if(x > 0) {
        memmove(&line->data[x-1], &line->data[x], line->count - x);
        line->count -= 1;
        line->data[line->count] = '\0';
    
        printf("\033[1D");
        e->cursor.x -= 1;
        return;
    }
    
    // Tricky here, we need to potentially do some line merging
    if(y == 0) return;
    
    EditorLine* above = &e->lines[y-1];
    
    // We save the jump the cursor will have to make
    int dx = above->count;
    
    // TODO: we can optimize this pretty well if the line above is empty!
    editorEnsureLine(above, above->count + line->count + 1);
    memcpy(&above->data[above->count], line->data, line->count);
    above->count += line->count;
    above->data[above->count] = '\0';
    
    // Then we deinit that line
    editorDeinitLine(e, y);
    e->lineCount -= 1;
    
    // Then we move lines down
    for(int i = y; i < e->lineCount; ++i) {
        e->lines[i].count = e->lines[i-1].count;
        e->lines[i].capacity = e->lines[i-1].capacity;
        e->lines[i].data = e->lines[i-1].data;
    }
    editorInitLine(e, e->lineCount);
    
    // And finally we move the cursor!
    if(dx) printf("\033[%dC", dx);
    printf("\033[1A");
    
    e->cursor.x += dx;
    e->cursor.y -= 1;
}

static void editorInsert(Editor* e, char c) {
    
    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;
    
    printf("\033[1C");
    e->cursor.x += 1;

    EditorLine* line = &e->lines[y];
    editorEnsureLine(line, line->count + 2);
    if(e->cursor.x != line->count)
        memmove(&line->data[x+1], &line->data[x], line->count - x);
    line->count += 1;
    line->data[x] = c;
    line->data[line->count] = '\0';
}

// TODO: Don't memmove here -- we need to keep the oldcount to, well, the old one
static void editorNewline(Editor* e) {
    if(e->cursor.x) printf("\033[%dD", e->cursor.x);
    //printf("\033[1B");
    putchar('\n');
    
    assert(e->lineCount < TERM_EDITOR_MAX_LINES);
    
    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;
    
    e->cursor.y += 1;
    e->cursor.x = 0;
    e->offset.x = 0;
    
    editorDeinitLine(e, e->lineCount);
    for(int i = e->lineCount; i > y; --i) {
        e->lines[i].data = e->lines[i-1].data;
        e->lines[i].count = e->lines[i-1].count;
        e->lines[i].capacity = e->lines[i-1].capacity;
    }
    
    EditorLine* currentLine = &e->lines[y];
    EditorLine* newLine = &e->lines[y+1];
    
    e->lineCount += 1;
    editorInitLine(e, y+1);
    
    // If we're not at the end of the line, we need to do some splitting
    int remaining = currentLine->count - x;
    if(remaining) {
        editorEnsureLine(newLine, remaining + 1);
        memcpy(newLine->data, currentLine->data + x, remaining);
        newLine->data[remaining] = '\0';
        newLine->count = remaining;
    }

    currentLine->count = x;
    if(currentLine->capacity) currentLine->data[x] = '\0';
}


static inline int byteSize(const Editor* e) {
    int size = 0;
    for(int i = 0; i < e->lineCount; ++i) {
        size += e->lines[i].count + 1;
    }
    return size;
}

void termEditorInit(Editor* e, const char* prompt) {
    e->cursor = (Coords){0, 0};
    e->offset = (Coords){0, 0};
    
    e->prompt = prompt;
    e->promptLength = strlen(prompt);
    
    e->lineCount = 1;
    for(int i = 0; i < TERM_EDITOR_MAX_LINES; ++i)
        editorInitLine(e, i);
}

void termEditorDeinit(Editor* e) {
    for(int i = 0; i < TERM_EDITOR_MAX_LINES; ++i)
        editorDeinitLine(e, i);
}

char* termEditorFlush(Editor* e) {
    int bytes = byteSize(e);
    char* string = malloc((bytes + 1) * sizeof(char));
    int index = 0;
    for(int i = 0; i < e->lineCount; ++i) {
        if(e->lines[i].count) {
            memcpy(string + index, e->lines[i].data, e->lines[i].count);
            index += e->lines[i].count;
        }
        string[index++] = '\n';
    }
    string[bytes] = '\0';
    return string;
}

static void scrollLeft(Editor* e, int over) {
    int dist = 0;
    while(dist < over) dist += 20;
    dist = min(dist, e->offset.x);
    
    e->offset.x -= dist;
    e->cursor.x += dist;
    
    printf("\033[%dC", dist);
}

static void scrollRight(Editor* e, int over) {
    int dist = 0;
    while(dist < over) dist += 20;
    
    e->offset.x += dist;
    e->cursor.x -= dist;
    
    printf("\033[%dD", dist);
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
    
    if(screen.x > 0) printf("\033[%dD", screen.x);
    if(screen.y > 0) printf("\033[%dA", screen.y);
    
    for(int i = e->offset.y; i < min(e->lineCount + 1, ny); ++i) {
        for(int i = 0; i < nx; ++i) putchar(' ');
        putchar('\n');
        current.y += 1;
    }
    
    if(current.x > 0) printf("\033[%dD", current.x);
    if(current.y > 0) printf("\033[%dA", current.y);
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
        termColorFG(stdout, kTermBlue);
        if(i)
            current.x += printf("%-*s", e->promptLength, " ");
        else
            current.x += printf("%s", e->prompt);
        termColorFG(stdout, kTermDefault);
        
        EditorLine line = e->lines[i];
        
        if(line.count) {
            printf("%.*s", min(line.count, nx - e->promptLength), line.data + e->offset.x);
        }
        putchar('\n');
        current.y += 1;
    }

    if(current.x > 0) printf("\033[%dD", current.x);
    if(current.y > 0) printf("\033[%dA", current.y);

    if(screen.x > 0) printf("\033[%dC", screen.x);
    if(screen.y > 0) printf("\033[%dB", screen.y);
}

void termEditorLeft(Editor* e) {
    if(e->cursor.x <= 0) return;
    e->cursor.x -= 1;
    printf("\033[1D");
}

void termEditorRight(Editor* e) {
    if(e->cursor.x + e->offset.x >= e->lines[e->cursor.y].count) return;
    e->cursor.x += 1;
    printf("\033[1C");
}

void termEditorUp(Editor* e) {
    e->cursor.y -= 1;
    printf("\033[1A");
    
    int max = e->lines[e->cursor.y].count;
    if(e->cursor.x <= max) return;
    printf("\033[%dD", e->cursor.x - max);
    e->cursor.x = max;
}

void termEditorDown(Editor* e) {
    e->cursor.y += 1;
    printf("\033[1B");
    
    int max = e->lines[e->cursor.y].count;
    if(e->cursor.x <= max) return;
    printf("\033[%dD", e->cursor.x - max);
    e->cursor.x = max;
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

