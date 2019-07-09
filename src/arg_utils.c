//===--------------------------------------------------------------------------------------------===
// arg_utils.c - Arg parser utilites
// This source is part of TermUtils
//
// Created on 2019-07-09 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <assert.h>
#include <term/arg.h>

void termArgParserInit(TUArgParser* parser, int argc, const char** argv) {
    assert(parser && "cannot initialise a null parser");

    parser->start = argv + 1;
    parser->end = argv + argc;
    parser->inOptions = true;
}

static bool paramExists(const TUArgParser* parser, char name) {
    for(int i = 0; i < parser->count; ++i) {
        if(parser->params[i].shortName == name) return true;
    }
    return false;
}

void termArgAddParam(TUArgParser* parser, TUParam param) {
    assert(parser && "cannot initialise a null parser");
    assert(!paramExists(parser, param.shortName));
    parser->params[parser->count++] = param;
}
