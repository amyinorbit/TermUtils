#include <stdio.h>
#include <term/arg.h>

static const TermParam options[] = {
    {'v', 0, "debug-vm", kTermArgOption, "print debug information (like -x -s)"},
    {'n', 0, "syntax-only", kTermArgOption, "stop compilation before code generation"},
    {0, 'O', "optimize", kTermArgValue, "specify a code optimization level"}
};

int main(int argc, const char** argv) {
    TermArgParser parser;
    termArgParserInit(&parser, argc, argv);
    
    
    for(;;) {
        TermArgResult r = termArgParse(&parser, options, 3);
        switch(r.name) {
            case kTermArgDone: return 0;
            
            case kTermArgHelp:
            termPrintHelp(stdout, options, 3);
            return 0;
            break;
            
            case kTermArgVersion:
            printf("version message\n");
            break;
            
            case kTermArgError:
            printf("error: %s\n", parser.error);
            return -1;
            
            case kTermArgPositional:
            printf("{%s}", r.value);
            break;
            
            case 'O':
            printf("optimizing at level %s\n", r.value);
            break;
            
            case 'v':
            printf("printing verbose diagnostics\n");
            break;
            
            default: {
                char c = r.name;
                printf("-%c\n", c);
            } break;
        }
    }
    
}