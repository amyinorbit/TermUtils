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

static TermArgResult fail(TermArgParser* parser, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(parser->error, TU_MAX_ERROR_SIZE, format, args);
    va_end(args);
    fputs("\n", stderr);
    return (TermArgResult){kTermArgError, NULL};
}

static inline TermArgResult ok(int name, const char* value) {
    return (TermArgResult){name, value};
}

static inline TermArgResult bail(TermArgStatus status) {
    return (TermArgResult){status, NULL};
}

static inline const char* current(TermArgParser* parser) {
    return parser->start < parser->end ? *parser->start : NULL;
}

static inline const char* eat(TermArgParser* parser) {
    parser->offset = 0;
    return parser->start < parser->end ? *(parser->start++) : NULL;
}

static const TermParam* findShort(char name, const TermParam* params, int count) {
    for(int i = 0; i < count; ++i) {
        if(!isalnum(params[i].name)) continue;
        if(params[i].name == name) return &params[i];
    }
    return NULL;
}

static const TermParam* findLong(const char* name, const TermParam* params, int count) {
    for(int i = 0; i < count; ++i) {
        if(!params[i].longName) continue;
        if(strcmp(params[i].longName, name) == 0) return &params[i];
    }
    return NULL;
}

static inline bool isPositional(const char* arg, size_t length) {
    return length == 1 || (arg[0] != '-');
}

static inline bool isShort(const char* arg, size_t length) {
    return length >= 2 && arg[0] == '-' && isalnum(arg[1]);
}

static inline bool isLong(const char* arg, size_t length) {
    return length >= 3 && arg[0] == '-' && arg[1] == '-' && isalnum(arg[2]);
}

static inline int id(const TermParam* param) {
    return isalnum(param->name) ? param->name : param->id;
}

static TermArgResult finishValue(TermArgParser* parser, const TermParam* param) {
    assert(param->kind == kTermArgValue);

    const char* value = eat(parser);
    if(!value) return fail(parser, "argument '-%c' requires a value", param->name);
    
    return ok(id(param), value);
}

static TermArgResult finishValueLong(TermArgParser* parser, const char* arg, const TermParam* param) {
    assert(param->kind == kTermArgValue);

    const char* value = eat(parser);
    if(!value) return fail(parser, "argument '--%s' requires a value", arg);
    
    return ok(id(param), value);
}


static TermArgResult shortArg(TermArgParser* parser, const TermParam* options, int count) {
    const char* list = current(parser) + 1;
    const char* listEnd = list + strlen(list);
    bool canHaveValue = strlen(list) == 1;
    
    char flag = *(list + parser->offset++);
    if(flag == 'h') return bail(kTermArgHelp);
    
    const TermParam* param = findShort(flag, options, count);
    if(!param) return fail(parser, "unknown argument: '-%c'", flag);
    
    if(param->kind == kTermArgOption) {
        if((list + parser->offset) >= listEnd) eat(parser);
        return ok(id(param), NULL);
    }
    
    if(!canHaveValue) return fail(parser, "argument '-%c' requires a value", param->name);
    return finishValue(parser, param);
}

static TermArgResult longArg(TermArgParser* parser, const TermParam* options, int count) {
    const char* arg = eat(parser) + 2; // skip the 
    if(strcmp(arg, "help") == 0) return bail(kTermArgHelp);
    if(strcmp(arg, "version") == 0) return bail(kTermArgVersion);
    
    const TermParam* param = findLong(arg, options, count);
    if(!param) return fail(parser, "unknown argument: '--%s'", arg);
    
    if(param->kind == kTermArgValue) return finishValueLong(parser, arg, param);
    return ok(id(param), NULL);
}

static TermArgResult positionalArg(TermArgParser* parser) {
    return ok(kTermArgPositional, eat(parser));
}


TermArgResult termArgParse(TermArgParser* parser, const TermParam* options, int count) {
    const char* arg = current(parser);
    if(!arg) return bail(kTermArgDone);
    size_t length = strlen(arg);
    
    if(strcmp(arg, "--") == 0) {
        parser->inOptions = false;
        eat(parser);
        arg = current(parser);
        if(!arg) return bail(kTermArgDone);
        length = strlen(arg);
    }
    
    if(!parser->inOptions || isPositional(arg, length)) return positionalArg(parser);
    if(isShort(arg, length)) return shortArg(parser, options, count);
    if(isLong(arg, length)) return longArg(parser, options, count);
    return fail(parser, "'%s' is not a valid argument", arg);
}
