//===--------------------------------------------------------------------------------------------===
// editor.h - multi-line-aware CLI editor. UNIX-based, but could work on windows (?)
// This source is part of TermUtils
//
// Created on 2019-07-24 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef term_editor_h
#define term_editor_h

#include <stdbool.h>

#ifndef TERM_EDITOR_MAX_LINES
#define TERM_EDITOR_MAX_LINES 64
#endif

typedef enum {
    kTermEditorDone,
    kTermEditorOK,
    kTermEditorTop,
    kTermEditorBottom,
    kTermEditorReturn,
} EditorStatus;

typedef struct EditorLine {
    int count;
    int capacity;
    char* data;
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
    EditorLine lines[TERM_EDITOR_MAX_LINES];
} Editor;

void termEditorInit(Editor* e, const char* prompt);
void termEditorDeinit(Editor* e);

char* termEditorFlush(Editor* e);

void termEditorLeft(Editor* e);
void termEditorRight(Editor* e);
void termEditorUp(Editor* e);
void termEditorDown(Editor* e);

void termEditorReplace(Editor* e, const char* data);
void termEditorClear(Editor* e);
void termEditorRender(Editor* e);
EditorStatus termEditorUpdate(Editor* e, char c);


#endif