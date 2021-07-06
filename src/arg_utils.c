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

void term_arg_parser_init(term_arg_parser_t* parser, int argc, const char** argv) {
    assert(parser && "cannot initialise a null parser");
    parser->start = argv + 1;
    parser->end = argv + argc;
    parser->offset = 0;
    parser->inOptions = true;
}
