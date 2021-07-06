//===--------------------------------------------------------------------------------------------===
// colors.h - color and formatting support for
// This source is part of TermUtils
//
// Created on 2019-07-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef termutils_colors_h
#define termutils_colors_h
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    TERM_BLACK,
    TERM_RED,
    TERM_GREEN,
    TERM_YELLOW,
    TERM_BLUE,
    TERM_MAGENTA,
    TERM_CYAN,
    TERM_WHITE,
    TERM_DEFAULT,
    TERM_BRIGHT_BLACK,
    TERM_BRIGHT_RED,
    TERM_BRIGHT_GREEN,
    TERM_BRIGHT_YELLOW,
    TERM_BRIGHT_BLUE,
    TERM_BRIGHT_MAGENTA,
    TERM_BRIGHT_CYAN,
    TERM_BRIGHT_WHITE,
    TERM_INVALID_COLOR,
} term_color_t;


bool term_has_colors(FILE* term);
void term_set_bold(FILE* term, bool bold);
void term_set_underline(FILE* term, bool underline);

void term_set_fg(FILE* term, term_color_t color);
void term_set_bg(FILE* term, term_color_t color);
void term_reverse(FILE* term);

void term_style_reset(FILE* term);

#endif
