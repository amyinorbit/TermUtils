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

void termEditorInit(const char* prompt);
void termEditorDeinit();

char* termEditorFlush();

void termEditorLeft();
void termEditorRight();
void termEditorUp();
void termEditorDown();

void termEditorReplace(const char* data);
void termEditorClear();
void termEditorRender();
EditorStatus termEditorUpdate(char c);


#endif