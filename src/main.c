#include <stdio.h>
#include <term/arg.h>
#include <term/colors.h>
#include <term/printing.h>
#include <term/repl.h>
#include <term/editor.h>
#include <stdlib.h>
#include "shims.h"


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
    
    TermREPL repl;
    termREPLInit(&repl);
    char* data = NULL;
    
    while((data = termREPL(&repl, "repl"))) {
        termInfo("demo", "received repl input: %s", data);
        free(data);
    }
    
    termREPLDeinit(&repl);
}