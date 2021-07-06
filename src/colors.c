//===--------------------------------------------------------------------------------------------===
// colors.c - color and formatting support for
// This source is part of TermUtils
//
// Created on 2019-07-10 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/colors.h>
#include <assert.h>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined (__MINGW32__)
#include <unistd.h>
#define SUPPORTS_COLOR(file) (isatty(fileno(file)))
#else
#define SUPPORTS_COLOR(file) (false)
#endif

static const char* _fgColors[] = {
    [TERM_BLACK] = "\033[30m",
    [TERM_RED] = "\033[31m",
    [TERM_GREEN] = "\033[32m",
    [TERM_YELLOW] = "\033[33m",
    [TERM_BLUE] = "\033[34m",
    [TERM_MAGENTA] = "\033[35m",
    [TERM_CYAN] = "\033[36m",
    [TERM_WHITE] = "\033[37m",
    [TERM_DEFAULT] = "\033[39m",
    [TERM_BRIGHT_BLACK] = "\033[30;1m",
    [TERM_BRIGHT_RED] = "\033[31;1m",
    [TERM_BRIGHT_GREEN] = "\033[32;1m",
    [TERM_BRIGHT_YELLOW] = "\033[33;1m",
    [TERM_BRIGHT_BLUE] = "\033[34;1m",
    [TERM_BRIGHT_MAGENTA] = "\033[35;1m",
    [TERM_BRIGHT_CYAN] = "\033[36;1m",
    [TERM_BRIGHT_WHITE] = "\033[37m",
    [TERM_INVALID_COLOR]  = "",
};

static const char* _bgColors[] = {
    [TERM_BLACK] = "\033[40m",
    [TERM_RED] = "\033[41m",
    [TERM_GREEN] = "\033[42m",
    [TERM_YELLOW] = "\033[43m",
    [TERM_BLUE] = "\033[44m",
    [TERM_MAGENTA] = "\033[45m",
    [TERM_CYAN] = "\033[46m",
    [TERM_WHITE] = "\033[47m",
    [TERM_DEFAULT] = "\033[49m",
    [TERM_BRIGHT_BLACK] = "\033[40;1m",
    [TERM_BRIGHT_RED] = "\033[41;1m",
    [TERM_BRIGHT_GREEN] = "\033[42;1m",
    [TERM_BRIGHT_YELLOW] = "\033[43;1m",
    [TERM_BRIGHT_BLUE] = "\033[44;1m",
    [TERM_BRIGHT_MAGENTA] = "\033[45;1m",
    [TERM_BRIGHT_CYAN] = "\033[46;1m",
    [TERM_BRIGHT_WHITE] = "\033[47;1m",
    [TERM_INVALID_COLOR]  = "",
};


bool term_has_colors(FILE* term) {
    return SUPPORTS_COLOR(term);
}

void term_set_bold(FILE* term, bool bold) {
    if(!term_has_colors(term)) return;
    if(bold)
        fprintf(term, "\033[1m");
    else
        fprintf(term, "\033[22m");
}

void term_set_underline(FILE* term, bool underline) {
    if(!term_has_colors(term)) return;
    if(underline)
        fprintf(term, "\033[4m");
    else
        fprintf(term, "\033[24m");
}

void term_set_fg(FILE* term, term_color_t color) {
    if(!term_has_colors(term)) return;
    assert(color >= TERM_BLACK && color < TERM_INVALID_COLOR);
    fprintf(term, "%s", _fgColors[color]);
}

void term_set_bg(FILE* term, term_color_t color) {
    if(!term_has_colors(term)) return;
    assert(color >= TERM_BLACK && color < TERM_INVALID_COLOR);
    fprintf(term, "%s", _bgColors[color]);
}

void term_reverse(FILE* term) {
    if(!term_has_colors(term)) return;
    fprintf(term, "\033[7m");
}

void term_style_reset(FILE* term) {
    if(!term_has_colors(term)) return;
    fprintf(term, "\033[0m");
}
