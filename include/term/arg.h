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
    kTermArgHelp = 'h',
    kTermArgDone = -1,
    kTermArgError = -2,
    kTermArgPositional = -3,
    kTermArgVersion = -4,
} TermArgStatus;

typedef enum { kTermArgOption, kTermArgValue,  } TermParamKind;

static const char termShortNone = '\0';
static const char* termLongNone = NULL;

typedef struct {
    char name; /// The short option name (for example -h) or termShortNone.
    int id; /// The id used to return this argument. If this is termShortNone, will use name.
    const char* longName; /// A long option name (for example --help) or termLongNone.
    TermParamKind kind;
    const char* description;
} TermParam;

typedef union {
    int intValue;
    float floatValue;
    const char* stringValue;
} TermArgValue;

typedef struct {
    int name;
    const char* value;
} TermArgResult;

typedef struct {
    const char** start;
    const char** end;
    
    int offset;
    bool inOptions;
    
    char error[TU_MAX_ERROR_SIZE];
} TermArgParser;

void termArgParserInit(TermArgParser* parser, int argc, const char** argv);
TermArgResult termArgParse(TermArgParser* parser, const TermParam* options, int count);
void termPrintHelp(FILE* out, const TermParam* options, int count);

#endif