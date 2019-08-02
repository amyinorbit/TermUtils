#include <stdio.h>
#include <term/arg.h>
#include <term/colors.h>
#include <term/printing.h>
#include <term/repl.h>
#include <term/editor.h>
#include <term/line.h>
#include <stdlib.h>
#include "shims.h"

void prompt(const char* PS) {
    termColorFG(stdout, kTermBlue);
    printf("%s > ", PS);
    termColorFG(stdout, kTermDefault);
}

int main(int argc, const char** argv) {

    termFilter(kTermInfo);
    
    LineFunctions functions;
    functions.printPrompt = prompt;
    
    Line* line = lineNew(&functions);
    lineSetPrompt(line, "repl");
    
    char* input = NULL;
    while((input = lineGet(line))) {
        termInfo("demo", "received input: %s", input);
        free(input);
    }
    
    lineDealloc(line);
}
