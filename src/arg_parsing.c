//===--------------------------------------------------------------------------------------------===
// arg_parsing.c - Implements the argument parser
// This source is part of TermUtils
//
// Created on 2019-07-09 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <term/arg.h>

typedef struct {
    TUResult status;
    TUArg* out;
} Result;

static Result fail(TUArgParser* parser, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(parser->error, TU_MAX_ERROR_SIZE, format, args);
    va_end(args);
    fputs("\n", stderr);
    return (Result){TU_ARGRESULT_ERROR, NULL};
}

static inline Result ok(TUArg* out) {
    return (Result){TU_ARGRESULT_OK, out};
}

static inline Result bail(TUResult status, TUArg* out) {
    return (Result){status, out};
}

static inline const char* eat(TUArgParser* parser) {
    return parser->start < parser->end ? *(parser->start++) : NULL;
}

static const TUParam* argsGetShort(const TUArgParser* parser, char name) {
    for(int i = 0; i < parser->count; ++i) {
        if(!parser->params[i].shortName) continue;
        if(parser->params[i].shortName == name) return &parser->params[i];
    }
    return NULL;
}

static const TUParam* argsGetLong(const TUArgParser* parser, const char* name) {
    for(int i = 0; i < parser->count; ++i) {
        if(!parser->params[i].name) continue;
        if(strcmp(parser->params[i].name, name) == 0) return &parser->params[i];
    }
    return NULL;
}

// static bool produce(TUArgParser* parser)

static inline bool isPositional(const char* arg, size_t length) {
    return length == 1 || (arg[0] != '-');
}

static inline bool isShort(const char* arg, size_t length) {
    return length >= 2 && arg[0] == '-' && isalpha(arg[1]);
}

static inline bool isLong(const char* arg, size_t length) {
    return length >= 3 && arg[0] == '-' && arg[1] == '-' && isalpha(arg[2]);
}

static Result finishValue(TUArgParser* parser, const TUParam* param, TUArg* out, const TUArg* end) {
    assert(param->kind == TU_ARG_VALUE);

    const char* value = eat(parser);
    if(!value) return fail(parser, "argument '-%c' requires a value", param->shortName);
    if(out == end) return bail(TU_ARGRESULT_NOMEM, out);

    out->name = param->shortName;
    out->kind = TU_ARG_VALUE;
    out->as.value = value;
    return ok(++out);
}

static Result shortArg(TUArgParser* parser, const char* arg, TUArg* out, const TUArg* end) {
    const char* argEnd = arg + strlen(arg);
    bool canHaveValue = strlen(arg) == 1;

    while(arg != argEnd) {
        char flag = *(arg++);
        if(flag == 'h') return bail(TU_ARGRESULT_HELP, out);

        const TUParam* param = argsGetShort(parser, flag);
        if(!param) return fail(parser, "unknown argument: '-%c'", flag);
        if(param->kind == TU_ARG_VALUE && !canHaveValue)
            return fail(parser, "argument '-%c' requires a value", param->shortName);
        if(param->kind == TU_ARG_VALUE) return finishValue(parser, param, out, end);

        if(out == end) return bail(TU_ARGRESULT_NOMEM, out);
        out->name = flag;
        out->kind = TU_ARG_OPTION;
        out++;
    }
    return ok(out);
}

static Result longArg(TUArgParser* parser, const char* arg, TUArg* out, const TUArg* end) {
    if(strcmp(arg, "help") == 0) return bail(TU_ARGRESULT_HELP, out);
    if(strcmp(arg, "version") == 0) return bail(TU_ARGRESULT_VERSION, out);

    const TUParam* param = argsGetLong(parser, arg);
    if(!param) return fail(parser, "unknown argument: '--%s'", arg);

    if(param->kind == TU_ARG_VALUE) return finishValue(parser, param, out, end);

    assert(param->kind == TU_ARG_OPTION);
    if(out == end) return bail(TU_ARGRESULT_NOMEM, out);
    out->name = param->shortName;
    out->kind = TU_ARG_OPTION;
    return ok(++out);
}

static Result positionalArg(TUArgParser* parser, const char* arg, TUArg* out, const TUArg* end) {
    if(out == end) return bail(TU_ARGRESULT_NOMEM, out);
    out->kind = TU_ARG_POS;
    out->as.value = arg;
    return ok(++out);
}

int termArgParse(TUArgParser* parser, TUArg* args, int count) {
    TUArg* out = args;
    const TUArg* end = args + count;
    while(parser->start != parser->end) {
        const char* arg = eat(parser);
        size_t length = strlen(arg);

        // printf("arg: %s > ", arg);

        if(strcmp(arg, "--") == 0) {
            parser->inOptions = false;
            continue;
        }

        Result r;
        if(isPositional(arg, length))
            r = positionalArg(parser, arg + 1, out, end);
        else if(isShort(arg, length))
            r = shortArg(parser, arg + 1, out, end);
        else if(isLong(arg, length))
            r = longArg(parser, arg + 2, out, end);

        if(r.status < 0) return r.status;
        out = r.out;
    }
    return out - args;
}
