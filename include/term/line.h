//===--------------------------------------------------------------------------------------------===
// line.h - Line editing library
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef termutils_line_h
#define termutils_line_h
#include <stdbool.h>
#include <stdio.h>
#include <term/colors.h>

#define CTL(c)      ((c) & 037)
#define IS_CTL(c)   ((c) && (c) < ' ')
#define DE_CTL(c)   ((c) + '@')

typedef enum LineAction {
    kLineStay,
    kLineDone,
    kLineReturn,
    kLineMove,
    kLineRefresh,
    kLineCancel,
} LineAction;

/// Command structure used for callbacks to control Line*.
typedef struct LineCMD {
    LineAction action;
    int param;
} LineCMD;

/// Used to override defaults
typedef struct {
    void (*printPrompt)(const char*);
} LineFunctions;

typedef struct Line Line;
typedef LineCMD (*LineBinding)(Line*, int key);

// TODO: pass in FILE* ptrs for input, output, error streams.
Line* lineNew(const LineFunctions* functions);
void lineDealloc(Line* line);

void lineRunCommand(LineAction action, void* param);

void lineSetPrompt(Line* line, const char* prompt);
char* lineGet(Line* line);

void lineHistoryLoad(Line* line, const char* path);
void lineHistoryWrite(Line* line, const char* path);
void lineHistoryAdd(Line* line, const char* entry);

#endif
