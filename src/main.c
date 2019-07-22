#include <stdio.h>
#include <term/arg.h>
#include <term/colors.h>
#include <term/printing.h>
#include <term/repl.h>

int main(int argc, const char** argv) {
    
    termError("demo", 0, "an error occured");
    termWarn("demo", "this is a warning");
    termInfo("demo", "this info won't show because of the diag level");
    termFilter(kTermInfo);
    termInfo("demo", "this is some information");
    
    puts("\n\n");
    
    TermArgParser parser;
    termArgParserInit(&parser, argc, argv);
    
    TermREPL repl;
    termREPLInit(&repl);
    
    const char* line = NULL;
    
    while((line = termREPL("repl> ", &repl))) {
        termInfo("demo", "received repl input: %s\n", line);
    }
    
    termREPLDeinit(&repl);
}