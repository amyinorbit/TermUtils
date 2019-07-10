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
    [kTermBlack] = "\033[30m",
    [kTermRed] = "\033[31m",
    [kTermGreen] = "\033[32m",
    [kTermYellow] = "\033[33m",
    [kTermBlue] = "\033[34m",
    [kTermMagenta] = "\033[35m",
    [kTermCyan] = "\033[36m",
    [kTermWhite] = "\033[37m",
    [kTermDefault] = "\033[39m",
    [kTermInvalidColor]  = "",
};

static const char* _bgColors[] = {
    [kTermBlack] = "\033[40m",
    [kTermRed] = "\033[41m",
    [kTermGreen] = "\033[42m",
    [kTermYellow] = "\033[43m",
    [kTermBlue] = "\033[44m",
    [kTermMagenta] = "\033[45m",
    [kTermCyan] = "\033[46m",
    [kTermWhite] = "\033[47m",
    [kTermDefault] = "\033[49m",
    [kTermInvalidColor]  = "",
};


bool termHasColors(FILE* term) {
    return SUPPORTS_COLOR(term);
}

void termBold(FILE* term, bool bold) {
    if(!termHasColors(term)) return;
    if(bold)
        fprintf(term, "\033[1m");
    else
        fprintf(term, "\033[22m");
}

void termUnderline(FILE* term, bool underline) {
    if(!termHasColors(term)) return;
    if(underline)
        fprintf(term, "\033[4m");
    else
        fprintf(term, "\033[24m");
}

void termColorFG(FILE* term, TermColor color) {
    if(!termHasColors(term)) return;
    assert(color >= kTermBlack && color < kTermInvalidColor);
    fprintf(term, "%s", _fgColors[color]);
}

void termColorBG(FILE* term, TermColor color) {
    if(!termHasColors(term)) return;
    assert(color >= kTermBlack && color < kTermInvalidColor);
    fprintf(term, "%s", _bgColors[color]);
}

void termReset(FILE* term) {
    if(!termHasColors(term)) return;
    fprintf(term, "\033[0m");
}