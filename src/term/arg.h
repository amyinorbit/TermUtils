//===--------------------------------------------------------------------------------------------===
// arg.h - Command line argument parsing API. Bascially a reentrant getopt_long
// This source is part of TermUtils
//
// Created on 2019-07-09 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef termutils_arg_h
#define termutils_arg_h
#include <stdbool.h>
#include <stdio.h>

#ifndef TU_PARAMS_MAX
#define TU_PARAMS_MAX 32
#endif

#ifndef TU_MAX_ERROR_SIZE
#define TU_MAX_ERROR_SIZE 128
#endif

typedef enum {
    TERM_ARG_HELP = 'h',
    TERM_ARG_DONE = -1,
    TERM_ARG_ERROR = -2,
    TERM_ARG_POSITIONAL = -3,
    TERM_ARG_VERSION = -4,
} term_arg_status_t;

typedef enum { TERM_ARG_OPTION, TERM_ARG_VALUE,  } term_param_kind_t;

static const char termShortNone = '\0';
static const char* termLongNone = NULL;

typedef struct {
    char name; /// The short option name (for example -h) or termShortNone.
    int id; /// The id used to return this argument. If this is termShortNone, will use name.
    const char* long_name; /// A long option name (for example --help) or termLongNone.
    term_param_kind_t kind;
    const char* description;
} term_param_t;

typedef union {
    int int_val;
    float float_val;
    const char* str_val;
} term_arg_value_t;

typedef struct {
    int name;
    const char* value;
} term_arg_result_t;

typedef struct {
    const char** start;
    const char** end;
    
    int offset;
    bool inOptions;
    
    char error[TU_MAX_ERROR_SIZE];
} term_arg_parser_t;

void term_arg_parser_init(term_arg_parser_t* parser, int argc, const char** argv);
term_arg_result_t term_arg_parse(term_arg_parser_t* parser, const term_param_t* options, int count);
void term_print_help(FILE* out, const term_param_t* options, int count);

#endif