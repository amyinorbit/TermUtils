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
#include <term/arg.h>
#include <term/colors.h>

void termPrintHelp(FILE* out, const TermParam* params, int count) {
    termReset(out);
    termBold(out, true);
    termColorFG(out, kTermBlack);
    printf("Options\n");
    termBold(out, false);

    for(int i = 0; i < count; ++i) {
        TermParam param = params[i];
        if(param.name)
            fprintf(out, " -%c,", param.name);
        else
            fprintf(out, "    ");

        if(param.longName) {
            fprintf(out, " --%-22s", param.longName);
        } else {
            fprintf(out, "                          ");
        }

        if(param.description) {
            fprintf(out, " %-40s", param.description);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "     --%-22s %-40s\n", "version", "print version number");
    fprintf(out, " -%c, --%-22s %-40s\n", 'h', "help", "print this help message");
}
