//===--------------------------------------------------------------------------------------------===
// editor.c - CLI editor implementation
// This source is part of TermUtils
//
// Created on 2019-07-24 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/editor.h>
#include <term/colors.h>
#include <term/hexes.h>
#include "string_buf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>

typedef struct EditorLine {
    int offset;
    int count;
} EditorLine;

typedef struct Coords {
    int x, y;
} Coords;

typedef struct Token {
    int line;
    int column;
    int length;
} Token;

typedef struct Editor {
    const char* title;

    Coords cursor;
    Coords offset;

    int lineCount;
    int lineCapacity;
    EditorLine* lines;

    Token highlight;
    
    string_buf_t buffer;

    const char* status;
    char* message;
} Editor;
static Editor E;

static inline int min(int a, int b) { return a < b ? a : b; }

static void ensureLines(int count) {
    if(count <= E.lineCapacity) return;
    while(E.lineCapacity < count)
        E.lineCapacity = E.lineCapacity ? E.lineCapacity * 2 : 32;
    E.lines = realloc(E.lines, E.lineCapacity * sizeof(EditorLine));
}

static void editorDEL() {
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;

    EditorLine* line = &E.lines[y];
    int offset = line->offset + x;

    if(offset >= E.buffer.count) return;

    char tbd = E.buffer.data[offset];

    if(tbd == '\n') {
        assert(y < E.lineCount - 1 && "If we're deleting \\n, we shouldn't be on the first line");
        assert(x == line->count && "We should be at the end of the line");
        string_buf_erase(&E.buffer, offset, 1);

        EditorLine* next = &E.lines[y+1];
        line->count += next->count;

        E.lineCount -= 1;
        for(int i = y+1; i < E.lineCount; ++i) {
            E.lines[i] = E.lines[i+1];
        }
    } else {
        string_buf_erase(&E.buffer, offset, 1);
        line->count -= 1;
    }
    for(int i = (E.offset.y + E.cursor.y)+1; i < E.lineCount; ++i) E.lines[i].offset -= 1;
}

static void editorBackspace() {
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;

    EditorLine* line = &E.lines[y];
    int offset = line->offset + (x - 1);

    if(offset < 0) return;

    char tbd = E.buffer.data[offset];

    if(tbd == '\n') {
        assert(y > 0 && "If we're deleting \\n, we shouldn't be on the first line");
        assert(x == 0 && "We should be at the start of the line");
        string_buf_erase(&E.buffer, offset, 1);

        EditorLine* prev = &E.lines[y-1];
        E.cursor.x += prev->count;
        E.cursor.y -= 1;
        prev->count += line->count;

        E.lineCount -= 1;
        for(int i = y; i < E.lineCount; ++i) {
            E.lines[i] = E.lines[i+1];
        }

    } else {
        string_buf_erase(&E.buffer, offset, 1);
        line->count -= 1;
        E.cursor.x -= 1;
    }
    for(int i = (E.offset.y + E.cursor.y)+1; i < E.lineCount; ++i) E.lines[i].offset -= 1;
}

static void editorInsert(char c) {
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;

    EditorLine* line = &E.lines[y];
    int offset = line->offset + x;
    string_buf_insert(&E.buffer, offset, c);
    E.cursor.x += 1;

    // Then we need to move lines. The only things that change are offsets
    // (and count for the current line)
    line->count += 1;
    for(int i = y+1; i < E.lineCount; ++i) E.lines[i].offset += 1;
}

static void editorNewline() {
    ensureLines(E.lineCount + 1);
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;

    EditorLine* line = &E.lines[y];
    int offset = line->offset + x;
    string_buf_insert(&E.buffer, offset, '\n');

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
    E.offset.x = 0;
}

// MARK: - "public" API

void termEditorInit(const char* title) {
    E.cursor = (Coords){0, 0};
    E.offset = (Coords){0, 0};

    E.title = title;

    E.lineCount = 1;
    E.lineCapacity = 0;
    ensureLines(1);

    string_buf_init(&E.buffer);

    E.lines[0] = (EditorLine){.count = 0, .offset = 0};

    E.status = "";
    E.message = malloc(1024);
    E.message[0] = '\0';
    E.highlight = (Token){-1, -1, -1};

    hexes_raw_start();
    hexes_set_alternate(true);
}

void termEditorDeinit() {
    hexes_raw_stop();
    hexes_set_alternate(false);
    string_buf_fini(&E.buffer);
    
    if(E.lines) free(E.lines);
    if(E.message) free(E.message);
    E.lines = NULL;
    E.lineCount = E.lineCapacity = 0;
    
    E.title = "";
    E.message = NULL;
    E.cursor = (Coords){0, 0};
    E.offset = (Coords){0, 0};
}

char* termEditorFlush() {
    char* data = E.buffer.data;
    E.buffer.data = NULL;
    E.buffer.count = 0;
    E.buffer.capacity = 0;
    return data;
}

const char* termEditorBuffer(int* length) {
    if(length) *length = E.buffer.count;
    return E.buffer.data;
}

static void keepInView() {
    int nx = 0, ny = 0;
    assert(hexes_get_size(&nx, &ny) == 0);
    nx -= 5; // To account for the line number space
    ny -= 3; // To account for the status bar

    if(E.cursor.x > nx) {
        E.offset.x += (E.cursor.x - nx);
        E.cursor.x = nx;
    } else if(E.cursor.x < 0) {
        int dist = min(-E.cursor.x, E.offset.x);
        E.offset.x -= dist;
        E.cursor.x = 0;
    }

    if(E.cursor.y > ny) {
        E.offset.y += (E.cursor.y - ny);
        E.cursor.y = ny;
    } else if(E.cursor.y < 0) {
        int dist = min(-E.cursor.y, E.offset.y);
        E.offset.y -= dist;
        E.cursor.y = 0;
    }
}

// TODO: this isn't really the nicest way to do things, we could probably memcpy a lot of this.
void termEditorReplace(const char* data) {
    termEditorClear();
    while(*data) {
        char c = *data++;
        if(c == '\n') editorNewline();
        else editorInsert(c);
    }
    E.cursor.x = 0;
    E.cursor.y = 0;
    E.offset.x = 0;
    E.offset.y = 0;
    keepInView();
}

void termEditorClear() {
    E.cursor.x = 0;
    E.cursor.y = 0;
    E.offset.x = 0;
    E.offset.y = 0;

    for(int i = 0; i < E.lineCount; ++i) {
        E.lines[i].count = 0;
    }
    E.buffer.count = 0;
    E.lineCount = 1;
}

static void renderTitle(int nx, int ny) {

    int c = E.cursor.x + E.offset.x + 1, r= E.cursor.y + E.offset.y + 1;

    char locBuffer[16];
    int locLength = snprintf(locBuffer, 16, " (%d, %d)  ", c, r);

    term_set_fg(stdout, TERM_BLUE);
    hexes_cursor_go(0, ny-2);
    term_reverse(stdout);
    int titleLength = printf("  %s | ", E.title);


    int statusLength = E.status ? min(nx - (titleLength + locLength), (int)strlen(E.status)) : 0;
    int locPad = nx - (titleLength + statusLength);
    printf("%.*s%*s", statusLength, E.status ? E.status : "", locPad, locBuffer);
    term_style_reset(stdout);
}

static void renderMessage(int nx, int ny) {
    int length = strlen(E.message);
    if(!length) return;
    hexes_cursor_go(0, ny-1);
    term_set_bold(stdout, true);
    printf("> %.*s", min(nx - 2, strlen(E.message)), E.message);
    term_style_reset(stdout);
}

static bool renderLineHead(int l) {
    bool done = false;
    term_set_fg(stdout, TERM_BLUE);
    if(l < E.lineCount) {
        printf("%3d ", l + 1);
        done = false;
    } else {
        printf("  ~ ");
        done = true;
    }
    term_set_fg(stdout, TERM_DEFAULT);
    return done;
}

static void renderLine(int i, int nx, int ny) {
    hexes_cursor_go(0, i);
    hexes_clear_line();
    int index = i + E.offset.y;
    if(renderLineHead(index)) return;
    EditorLine line = E.lines[index];

    if(!line.count || line.count <= E.offset.x) return;

    int startHL = (E.highlight.column - 1);
    int endHL = (E.highlight.column + E.highlight.length - 1);

    for(int i = 0; i < nx - 4; ++i) {
        int idx = i + E.offset.x;
        if(idx == line.count) break;
        if(idx == startHL && index == E.highlight.line-1) {
            term_set_underline(stdout, true);
            term_set_bold(stdout, true);
            term_set_fg(stdout, TERM_RED);
        }
        putchar(E.buffer.data[line.offset + idx]);
        if(idx == endHL && index == E.highlight.line-1) {
            term_style_reset(stdout);
        }
    }

    term_style_reset(stdout);

}

void termEditorRender() {
    int nx = 0, ny = 0;
    assert(hexes_get_size(&nx, &ny) == 0);
    hexes_cursor_go(0, 0);

    Coords screen = (Coords){
        E.cursor.x + 4,
        E.cursor.y
    };

    for(int i = 0; i < ny-2; ++i) renderLine(i, nx, ny);
    renderTitle(nx, ny);
    renderMessage(nx, ny);

    hexes_cursor_go(screen.x, screen.y);
    fflush(stdout);
}

void termEditorLeft() {
    E.cursor.x -= 1;
}

void termEditorRight() {
    if(E.cursor.x + E.offset.x >= E.lines[E.cursor.y + E.offset.y].count) return;
    E.cursor.x += 1;
}

void termEditorUp() {
    E.cursor.y -= 1;
    int max = E.lines[E.cursor.y + E.offset.y].count;
    if(E.cursor.x <= max) return;
    E.cursor.x = max;
}

void termEditorDown() {
    E.cursor.y += 1;
    int max = E.lines[E.cursor.y + E.offset.y].count;
    if(E.cursor.x <= max) return;
    E.cursor.x = max;
}

void termEditorStatus(const char* status) {
    E.status = status;
}

void termEditorOut(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(E.message, 1024, fmt, args);
    va_end(args);
}

void termEditorInsert(char c) {
    editorInsert(c);
}

HexesKey termEditorUpdate() {
    int c = hexes_get_key_raw();
    switch(c) {
    case KEY_RETURN:
        editorNewline();
        break;

    case KEY_DELETE:
        editorDEL();
        break;

    case KEY_BACKSPACE:
        editorBackspace();
        break;

    case KEY_ARROW_UP:
        if(E.cursor.y + E.offset.y > 0)
            termEditorUp();
        break;

    case KEY_ARROW_DOWN:
        if(E.cursor.y + E.offset.y < E.lineCount - 1)
            termEditorDown();
        break;

    case KEY_ARROW_RIGHT:
        termEditorRight();
        break;

    case KEY_ARROW_LEFT:
        termEditorLeft();
        break;

    case KEY_CTRL_D:
    case KEY_TAB:
    case KEY_CTRL_C:
    case KEY_CTRL_S:
    case KEY_CTRL_Q:
        break;

    default:
        editorInsert(c);
        break;
    }
    keepInView();
    return c;
}
