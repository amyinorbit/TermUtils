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

typedef enum {
    KEY_CTRL_C      = 0x03,
    KEY_CTRL_D      = 0x04,
    KEY_CTRL_F      = 0x06,
    KEY_CTRL_H      = 0x08,
    KEY_TAB         = 0x09,
    KEY_CTRL_L      = 0x0c,
    KEY_RETURN      = 0x0d,
    KEY_CTRL_Q      = 0x11,
    KEY_CTRL_S      = 0x13,
    KEY_ESC         = 0x1b,
    KEY_BACKSPACE   = 0x7f,
    
    KEY_ARROW_LEFT  = 500,
    KEY_ARROW_RIGHT,
    KEY_ARROW_UP,
    KEY_ARROW_DOWN,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
} EditorKey;

void termEditorInit(const char* prompt);
void termEditorDeinit();

// Prints a message in the console half of the editor (bottom lines?)

void termEditorSetTitle(const char* title);
void termEditorOut(const char* fmt, ...);

char* termEditorFlush();
const char* termEditorBuffer(int* length);

void termEditorLeft();
void termEditorRight();
void termEditorUp();
void termEditorDown();

void termEditorReplace(const char* data);
void termEditorClear();
void termEditorRender();
EditorKey termEditorUpdate();


#endif