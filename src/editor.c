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
#include <ctype.h>

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
    
    char* buffer;
    int count;
    int capacity;
} Editor;
static Editor E;

#ifndef _WIN32
static struct termios savedTerm;
#endif

void startRawMode() {
#ifdef _WIN32
#else
    struct termios raw;

    if (!isatty(STDIN_FILENO)) return;
    // atexit(editorAtExit);
    if (tcgetattr(STDIN_FILENO,&savedTerm) == -1) return;

    raw = savedTerm;  /* modify the original mode */
    /* input modes: no break, no CR to NL, no parity check, no strip char,
     * no start/stop output control. */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    /* output modes - disable post processing */
    raw.c_oflag &= ~(OPOST);
    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);
    /* local modes - echoing off, canonical off, no extended functions,
     * no signal chars (^Z,^C) */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer. */
    // raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
    // raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) return;
#endif
}

void stopRawMode() {
#ifdef _WIN32
#else
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&savedTerm);
#endif
}

static inline int min(int a, int b) { return a < b ? a : b; }
static inline int max(int a, int b) { return a > b ? a : b; }

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

static void editorDEL() {
    int x = E.offset.x + E.cursor.x;
    int y = E.offset.y + E.cursor.y;
    
    EditorLine* line = &E.lines[y];
    int offset = line->offset + x;
    
    if(offset >= E.count) return;
    
    char tbd = E.buffer[offset];
    
    if(tbd == '\n') {
        assert(y < E.lineCount - 1 && "If we're deleting \\n, we shouldn't be on the first line");
        assert(x == line->count && "We should be at the end of the line");
        erase(offset);
        
        EditorLine* next = &E.lines[y+1];
        line->count += next->count;
        
        E.lineCount -= 1;
        for(int i = y+1; i < E.lineCount; ++i) {
            E.lines[i] = E.lines[i+1];
        }
    } else {
        erase(offset);
        line->count -= 1;
    }
    for(int i = (E.offset.y + E.cursor.y)+1; i < E.lineCount; ++i) E.lines[i].offset -= 1;
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
        E.cursor.x += prev->count;
        E.cursor.y -= 1;
        prev->count += line->count;
        
        E.lineCount -= 1;
        for(int i = y; i < E.lineCount; ++i) {
            E.lines[i] = E.lines[i+1];
        }
        
    } else {
        erase(offset);
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
    insert(offset, c);
    E.cursor.x += 1;
    
    // Then we need to move lines. The only things that change are offsets
    // (and count for the current line)
    line->count += 1;
    
    for(int i = y+1; i < E.lineCount; ++i) E.lines[i].offset += 1;
}

// TODO: Don't memmove here -- we need to keep the oldcount to, well, the old one
static void editorNewline() {
    ensureLines(E.lineCount + 1);
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
    
    E.count = 0;
    E.capacity = 0;
    E.buffer = NULL;
    
    E.lines[0] = (EditorLine){.count = 0, .offset = 0};
    startRawMode();
    printf("\033[?1049h");
    
    E.highlight = (Token){-1, -1, -1};
}

void termEditorDeinit() {
    stopRawMode();
    printf("\033[?1049l");
    if(E.buffer) free(E.buffer);
    if(E.lines) free(E.lines);
    E.lines = NULL;
    E.lineCount = E.lineCapacity = 0;
    E.buffer = NULL;
    E.count = E.capacity = 0;
    E.title = "";
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

static void keepInView() {
    int nx = tcols() - 5; // To account for the line number space
    int ny = trows() - 3; // To account for the status bar
    
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
}

void termEditorClear() {
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

static void renderTitle(int nx, int ny) {
    
    int c = E.cursor.x + E.offset.x + 1, r= E.cursor.y + E.offset.y + 1;
    
    char locBuffer[16];
    snprintf(locBuffer, 16, "(%d, %d)", c, r);
    
    termColorFG(stdout, kTermBlue);
    printf("\033[%d;1H", ny - 1);
    printf("\033[7m");
    int titleLength = printf("  %s", E.title);
    printf("%*s  ", (nx - titleLength - 2), locBuffer);
    termReset(stdout);
}

static bool renderLineHead(int l) {
    bool done = false;
    termColorFG(stdout, kTermBlue);
    if(l < E.lineCount) {
        printf("%3d ", l + 1);
        done = false;
    } else {
        printf("  ~ ");
        done = true;
    }
    termColorFG(stdout, kTermDefault);
    return done;
}

static void printLine(int i, int nx, int ny) {
    printf("\033[%d;1H\033[2K", 1 + i);
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
            termUnderline(stdout, true);
            termBold(stdout, true);
            termColorFG(stdout, kTermRed);
            // printf("\033[7m");
        }
        putchar(E.buffer[line.offset + idx]);
        if(idx == endHL && index == E.highlight.line-1) {
            termReset(stdout);
        }
    }
    
    termReset(stdout);
    
}

void termEditorRender() {
    
    int nx = tcols();
    int ny = trows();
    printf("\033[H");
    
    Coords screen = (Coords){
        E.cursor.x + 4,
        E.cursor.y
    };
    
    for(int i = 0; i < ny-2; ++i) printLine(i, nx, ny);
    renderTitle(nx, ny);
    
    printf("\033[%d;%dH", screen.y+1, screen.x+1);
    fflush(stdout);
}

void termEditorLeft() {
    E.cursor.x -= 1;
}

void termEditorRight() {
    if(E.cursor.x + E.offset.x >= E.lines[E.cursor.y].count) return;
    E.cursor.x += 1;
}

void termEditorUp() {
    E.cursor.y -= 1;
    int max = E.lines[E.cursor.y].count;
    if(E.cursor.x <= max) return;
    E.cursor.x = max;
}

void termEditorDown() {
    E.cursor.y += 1;
    int max = E.lines[E.cursor.y].count;
    if(E.cursor.x <= max) return;
    E.cursor.x = max;
}

#define CTRL_KEYPRESS(k) ((k)  & 0x1f)
static int getInput() {
    // Buffer for our input. We need that for extended escapes
    char buf[3];
    
    char c = getch();
    switch(c) {
        
    case CTRL('s'):
        return KEY_CTRL_S;
        
    case KEY_ESC:
        buf[0] = getch();
        buf[1] = getch();
        
        if(buf[0] == '[') {
            // If we have a digit, then we have an extended escape sequence
            if(isnumber(buf[1])) {
                buf[2] = getch();
                if(buf[2] == '~') {
                    switch(buf[1]) {
                        case '3': return KEY_DELETE;
                        case '5': return KEY_PAGE_UP;
                        case '6': return KEY_PAGE_DOWN;
                    }
                }
            } else {
                switch(buf[1]) {
                    case 'A': return KEY_ARROW_UP;
                    case 'B': return KEY_ARROW_DOWN;
                    case 'C': return KEY_ARROW_RIGHT;
                    case 'D': return KEY_ARROW_LEFT;
                }
            }
        }
        break;
    default:
        return c;
    }
    return -1;
}

EditorKey termEditorUpdate() {
    int c = getInput();
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
        return KEY_CTRL_D;
        
    case KEY_CTRL_C:
        break;
        
    case KEY_CTRL_S:
        E.title = "CTRL-S PRESSED";
        break;
        
    default:
        editorInsert(c);
        break;
    }
    keepInView();
    return c;
}

