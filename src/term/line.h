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

typedef enum line_action_e {
    LINE_STAY,
    LINE_DONE,
    LINE_RETURN,
    LINE_MOVE,
    LINE_REFRESH,
    LINE_CANCEL,
} line_action_t;

/// Command structure used for callbacks to control line_t*.
typedef struct line_cmd_s {
    line_action_t action;
    int param;
} line_cmd_t;

/// Used to override defaults
typedef struct line_functions_s {
    void (*print_prompt)(const char*);
} line_functions_t;

typedef struct line_t line_t;
typedef line_cmd_t (*line_binding_t)(line_t*, int key);

// TODO: pass in FILE* ptrs for input, output, error streams.
line_t* line_new(const line_functions_t* functions);
void line_destroy(line_t* line);

void line_run_cmd(line_action_t action, void* param);

void line_set_prompt(line_t* line, const char* prompt);
char* line_get(line_t* line);

void line_history_load(line_t* line, const char* path);
void line_history_write(line_t* line, const char* path);
void line_history_add(line_t* line, const char* entry);

#endif
