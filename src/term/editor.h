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
#include <term/hexes.h>

void termEditorInit(const char* prompt);
void termEditorDeinit();

// Prints a message in the console half of the editor (bottom lines?)

void termEditorStatus(const char* title);
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
void termEditorInsert(char c);
HexesKey termEditorUpdate();


#endif
