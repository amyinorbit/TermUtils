//===--------------------------------------------------------------------------------------------===
// arg_printing.c - Implements help-printing functions for the argument parser
// This source is part of TermUtils
//
// Created on 2019-07-09 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <string.h>
#include <stdint.h>
#include <term/arg.h>
#include <term/colors.h>

static inline void pad(FILE* out, uint8_t count) {
    fprintf(out, "%*s", (int)count, "");
}

static const char* next_line(const char* str) {
    while(*str) {
        if(*str++ == '\n') return str;
    }
    return NULL;
}

// This is probably inefficient and slow, but then again once we're printing the help message
// we're probably not counting on speed
static void print_aligned(FILE* out, const char* str, uint8_t start, uint8_t justify) {
    
    if(start >= justify) {
        fputc('\n', out);
        start = 0;
    }
    if(start < justify)
        pad(out, justify-start);
    
    const char* line = str;
    while(line) {
        size_t length = strlen(line);
        const char* next = next_line(line);
        if(next) length = next - line;
        
        if(line != str) pad(out, justify);
        fprintf(out, "%.*s", (int)length, line);
        
        line = next;
    }
}


static void print_param(FILE* out, const term_param_t* param, uint8_t start) {
    int col = 0;
    if(param->name)
        col += fprintf(out, " -%c,", param->name);
    else
        col += fprintf(out, "    ");
    
    if(param->long_name)
        col += fprintf(out, " --%s", param->long_name);

    if(param->description) {
        print_aligned(out, param->description, col, start);
        fputc('\n', out);
    }
}

// static const term_param_t version = {0, 0, "version", 0, "print version number"};
static const term_param_t help = {'h', 0, "help", 0, "print this help message"};

void term_print_help(FILE* out, const term_param_t* params, int count) {
    term_style_reset(out);
    term_set_bold(out, true);
    printf("Options\n");
    term_style_reset(out);

    for(int i = 0; i < count; ++i) {
        print_param(out, params + i, 25);
    }
    
    // print_param(out, &version, 25);
    print_param(out, &help, 25);
}
