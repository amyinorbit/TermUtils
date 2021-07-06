#include <stdio.h>
#include <term/arg.h>
#include <term/colors.h>
#include <term/printing.h>
#include <term/editor.h>
#include <term/line.h>
#include <stdlib.h>

void prompt(const char* PS) {
    term_set_fg(stdout, TERM_BLUE);
    printf("%s > ", PS);
    term_set_fg(stdout, TERM_DEFAULT);
}

int main(int argc, const char** argv) {

    term_set_filter(TERM_INFO);
    
    line_functions_t functions;
    functions.print_prompt = prompt;
    
    line_t* line = line_new(&functions);
    line_history_load(line, ".demo_history");
    
    line_set_prompt(line, "repl");
    
    char* input = NULL;
    while((input = line_get(line))) {
        term_info("demo", "received input: %s", input);
        free(input);
    }
    
    line_history_write(line, ".demo_history");
    line_destroy(line);
}
