#include <stdio.h>
#include <term/arg.h>
#include <term/colors.h>
#include <term/printing.h>
#include <term/repl.h>
#include <stdlib.h>
#include "editor.h"
#include "shims.h"


int main(int argc, const char** argv) {
    termFilter(kTermInfo);
    
    TermREPL repl;
    termREPLInit(&repl, "repl > ", kTermCyan);
    char* data = NULL;

    while((data = termREPL(&repl))) {
        termInfo("demo", "received repl input: %s", data);
        free(data);
    }

    termREPLDeinit(&repl);
}