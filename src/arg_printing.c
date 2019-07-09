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

void termPrintHelp(FILE* out, const TUArgParser* parser) {
    printf("Options\n");

    for(int i = 0; i < parser->count; ++i) {
        TUParam param = parser->params[i];
        if(param.kind == TU_ARG_POS) continue;
        if(param.shortName)
            fprintf(out, " -%c,", param.shortName);
        else
            fprintf(out, "    ");

        if(param.name && param.kind == TU_ARG_VALUE) {
            fprintf(out, " --%s ", param.name);
            fprintf(out, "%-*s", (int)(21 - strlen(param.name)), param.valueDesc);
        } else if(param.name) {
            fprintf(out, " --%-22s", param.name);
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
