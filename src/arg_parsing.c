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
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <term/arg.h>

static term_arg_result_t fail(term_arg_parser_t* parser, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(parser->error, TU_MAX_ERROR_SIZE, format, args);
    va_end(args);
    return (term_arg_result_t){TERM_ARG_ERROR, NULL};
}

static inline term_arg_result_t ok(int name, const char* value) {
    return (term_arg_result_t){name, value};
}

static inline term_arg_result_t bail(term_arg_status_t status) {
    return (term_arg_result_t){status, NULL};
}

static inline const char* current(term_arg_parser_t* parser) {
    return parser->start < parser->end ? *parser->start : NULL;
}

static inline const char* eat(term_arg_parser_t* parser) {
    parser->offset = 0;
    return parser->start < parser->end ? *(parser->start++) : NULL;
}

static const term_param_t* findShort(char name, const term_param_t* params, int count) {
    for(int i = 0; i < count; ++i) {
        if(!isalnum(params[i].name)) continue;
        if(params[i].name == name) return &params[i];
    }
    return NULL;
}

static const term_param_t* findLong(const char* name, const term_param_t* params, int count) {
    for(int i = 0; i < count; ++i) {
        if(!params[i].long_name) continue;
        if(strcmp(params[i].long_name, name) == 0) return &params[i];
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

static inline int id(const term_param_t* param) {
    return isalnum(param->name) ? param->name : param->id;
}

static term_arg_result_t finishValue(term_arg_parser_t* parser, const term_param_t* param) {
    assert(param->kind == TERM_ARG_VALUE);

    const char* value = eat(parser);
    if(!value) return fail(parser, "argument '-%c' requires a value", param->name);
    
    return ok(id(param), value);
}

static term_arg_result_t finishValueLong(term_arg_parser_t* parser, const char* arg, const term_param_t* param) {
    assert(param->kind == TERM_ARG_VALUE);

    const char* value = eat(parser);
    if(!value) return fail(parser, "argument '--%s' requires a value", arg);
    
    return ok(id(param), value);
}


static term_arg_result_t shortArg(term_arg_parser_t* parser, const term_param_t* options, int count) {
    const char* list = current(parser) + 1;
    const char* listEnd = list + strlen(list);
    bool canHaveValue = strlen(list) == 1;
    
    char flag = *(list + parser->offset++);
    if(flag == 'h') return bail(TERM_ARG_HELP);
    
    const term_param_t* param = findShort(flag, options, count);
    if(!param) return fail(parser, "unknown argument: '-%c'", flag);
    
    if(param->kind == TERM_ARG_OPTION) {
        if((list + parser->offset) >= listEnd) eat(parser);
        return ok(id(param), NULL);
    }
    
    if(!canHaveValue) return fail(parser, "argument '-%c' requires a value", param->name);
    return finishValue(parser, param);
}

static term_arg_result_t longArg(term_arg_parser_t* parser, const term_param_t* options, int count) {
    const char* arg = eat(parser) + 2; // skip the 
    if(strcmp(arg, "help") == 0) return bail(TERM_ARG_HELP);
    if(strcmp(arg, "version") == 0) return bail(TERM_ARG_VERSION);
    
    const term_param_t* param = findLong(arg, options, count);
    if(!param) return fail(parser, "unknown argument: '--%s'", arg);
    
    if(param->kind == TERM_ARG_VALUE) return finishValueLong(parser, arg, param);
    return ok(id(param), NULL);
}

static term_arg_result_t positionalArg(term_arg_parser_t* parser) {
    return ok(TERM_ARG_POSITIONAL, eat(parser));
}


term_arg_result_t term_arg_parse(term_arg_parser_t* parser, const term_param_t* options, int count) {
    const char* arg = current(parser);
    if(!arg) return bail(TERM_ARG_DONE);
    size_t length = strlen(arg);
    
    if(strcmp(arg, "--") == 0) {
        parser->inOptions = false;
        eat(parser);
        arg = current(parser);
        if(!arg) return bail(TERM_ARG_DONE);
        length = strlen(arg);
    }
    
    if(!parser->inOptions || isPositional(arg, length)) return positionalArg(parser);
    if(isShort(arg, length)) return shortArg(parser, options, count);
    if(isLong(arg, length)) return longArg(parser, options, count);
    return fail(parser, "'%s' is not a valid argument", arg);
}
