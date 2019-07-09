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
    TU_ARGRESULT_OK = 0,
    TU_ARGRESULT_HELP = -1,
    TU_ARGRESULT_VERSION = -2,
    TU_ARGRESULT_NOMEM = -3,
    TU_ARGRESULT_ERROR = -4,
} TUResult;

typedef enum { TU_ARG_VALUE, TU_ARG_OPTION, TU_ARG_POS } TUParamKind;

typedef struct {
    TUParamKind kind;
    char shortName;
    const char* name;
    const char* description;
    const char* valueDesc;
} TUParam;

typedef struct {
    char name;
    TUParamKind kind;
    union {
        bool flag;
        const char* value;
    } as;
} TUArg;


typedef struct {
    const char** start;
    const char** end;
    bool inOptions;

    int count;
    TUParam params[TU_PARAMS_MAX];

    char error[TU_MAX_ERROR_SIZE];
} TUArgParser;

void termArgParserInit(TUArgParser* parser, int argc, const char** argv);
void termArgAddParam(TUArgParser* parser, TUParam param);

void termArgAddOption(TUArgParser* parser, char name, const char* longName, const char* desc);
void termArgAddOptionLong(TUArgParser* parser, const char* longName, const char* desc);
void termArgAddValueLong(TUArgParser* parser, const char* longName, const char* desc,
                         const char* value);

int termArgParse(TUArgParser* parser, TUArg* args, int count);

void termPrintHelp(FILE* out, const TUArgParser* parser);

#endif