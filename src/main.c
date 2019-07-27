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
    
    Editor editor;
    termEditorInit(&editor);
    
    do {
        termEditorRender(&editor);
    } while(termEditorUpdate(&editor, getch()) != kTermEditorDone);
    
    char* data = termEditorFlush(&editor);
    printf("Editor data: \n---\n%s\n---\n", data);
    termEditorDeinit(&editor);
}