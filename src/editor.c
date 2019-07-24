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
#include <term/colors.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef _WIN32
#include <conio.h>

#define getch _getch

#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BUFFER_DEFAULT_CAPACITY 512

static inline int getch(void) {
    struct termios oldt, newt;
	int ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}
#endif

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
    
}

static void editorInsert(Editor* e, char c) {
    putchar(c);
    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;

    EditorLine* line = &e->lines[y];
    editorEnsureLine(line, line->count + 2);
    if(e->cursor.x != line->count)
        memmove(&line->data[x+1], &line->data[x], line->count - x);
    line->count += 1;
    line->data[x] = c;
    // line->data[line->count] = '\0';
    e->cursor.x += 1;
}

static void editorNewline(Editor* e) {
    putchar('\n');
    assert(e->lineCount < TERM_EDITOR_MAX_LINES);

    int x = e->offset.x + e->cursor.x;
    int y = e->offset.y + e->cursor.y;
    
    e->cursor.y += 1;
    e->cursor.x = 0;
    
    if(e->cursor.y != e->lineCount)
        memmove(&e->lines[y+1], &e->lines[y], e->lineCount - y);
    EditorLine* currentLine = &e->lines[y];
    EditorLine* newLine = &e->lines[y+1];
    
    e->lineCount += 1;
    editorInitLine(e, y+1);
    // If we're not at the end of the line, we need to do some splitting
    int remaining = currentLine->count - x;
    if(!remaining) return;
    
    editorInitLine(e, y+1);
    editorEnsureLine(newLine, remaining + 1);
    memcpy(newLine->data, currentLine->data + x, remaining);
    newLine->data[remaining] = '\0';
    newLine->count = remaining;
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
    for(int i = 0; i < TERM_EDITOR_MAX_LINES; ++i) editorInitLine(e, 0);
}

void termEditorDeinit(Editor* e) {
    for(int i = 0; i < TERM_EDITOR_MAX_LINES; ++i) editorDeinitLine(e, 0);
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

void termEditorRender(Editor* e) {
    
    Coords screen = (Coords){
        e->promptLength + (e->cursor.x - e->offset.x),
        e->cursor.y - e->offset.y
    };
    
    Coords current = (Coords){0, 0};
    
    if(screen.x > 0) printf("\033[%dD", screen.x);
    if(screen.y > 0) printf("\033[%dA", screen.y);
    
    for(int i = 0; i < e->lineCount; ++i) {
        current.x = printf("%-*.s", e->promptLength + e->lines[i].count + 2, " ");
        putchar('\n');
        current.y += 1;
    }
    
    if(current.x > 0) printf("\033[%dD", current.x);
    if(current.y > 0) printf("\033[%dA", current.y);
    
    current.x = 0;
    current.y = 0;
    
    // the simple bit: we print the lines!
    for(int i = 0; i < e->lineCount; ++i) {
        current.x = 0;
        termColorFG(stdout, kTermBlue);
        if(i)
            current.x += printf("%-*.s", e->promptLength, "...");
        else
            current.x += printf("%s", e->prompt);
        termColorFG(stdout, kTermDefault);
        
        
        EditorLine line = e->lines[i];
        // printf("line: %d\n", line.count);
        if(line.count) {
            for(int i = 0; i < line.count; ++i) putchar(line.data[i]);
        }
        putchar('\n');
        current.y += 1;
    }
    
    if(current.x > 0) printf("\033[%dD", current.x);
    if(current.y > 0) printf("\033[%dA", current.y);
    
    if(screen.x > 0) printf("\033[%dC", screen.x);
    if(screen.y > 0) printf("\033[%dB", screen.y);
}

bool termEditorUpdate(Editor* e) {
    int c = getch();
    switch(c) {
    case '\n':
        editorNewline(e);
        break;
        
    case 0x04:
        return false;
        break;
        
    default:
        editorInsert(e, c);
        break;
    }
    return true;
}

