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
    kTermBlack,
    kTermRed,
    kTermGreen,
    kTermYellow,
    kTermBlue,
    kTermMagenta,
    kTermCyan,
    kTermWhite,
    kTermDefault,
    kTermInvalidColor,
} TermColor;


bool termHasColors(FILE* term);
void termBold(FILE* term, bool bold);
void termUnderline(FILE* term, bool underline);

void termColorFG(FILE* term, TermColor color);
void termColorBG(FILE* term, TermColor color);

void termReset(FILE* term);

#endif
