#include <stdio.h>
#include <term/arg.h>
#include <term/colors.h>
#include <term/printing.h>
#include <term/repl.h>

int main(int argc, const char** argv) {
    termFilter(kTermInfo);
    
    TermREPL repl;
    termREPLInit(&repl, "repl> ", kTermCyan);
    const char* line = NULL;
    
    while((line = termREPL(&repl))) {
        termInfo("demo", "received repl input: %s\n", line);
    }
    
    termREPLDeinit(&repl);
}