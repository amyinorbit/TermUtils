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
    termColorFG(stdout, kTermBlack);
    printf("%s > ", PS);
    termColorFG(stdout, kTermDefault);
}

int main(int argc, const char** argv) {
    // termEditorInit("file.c");
    // do {
    //     termEditorRender();
    // } while(termEditorUpdate() != KEY_CTRL_D);
    //
    // char* data = termEditorFlush();
    // termEditorDeinit();
    // printf("Editor data: \n---\n%s\n---\n", data);

    termFilter(kTermInfo);
    
    Line* line = lineNew();
    lineSetPrompt(line, "repl");
    
    char* input = NULL;
    while((input = lineGet(line))) {
        termInfo("demo", "received input: %s", input);
        free(input);
    }
    
    lineDealloc(line);
    

    // TermREPL repl;
    // termREPLInit(&repl);
    // char* data = NULL;
    //
    // while((data = termREPL(&repl, "repl"))) {
    //     termInfo("demo", "received repl input: %s", data);
    //     termREPLRecord(&repl, data);
    //     free(data);
    // }
    //
    // termREPLDeinit(&repl);
}
