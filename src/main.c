#include <stdio.h>
#include <term/arg.h>
#include <term/colors.h>
#include <term/printing.h>

static const TermParam options[] = {
    {'v', 0, "debug-vm", kTermArgOption, "print debug information (like -x -s)"},
    {'n', 0, "syntax-only", kTermArgOption, "stop compilation before code generation.\nThis argument has a lot of text, and probably\nexplains some non-trivial side effects."},
    {0, 'O', "optimize-some-long-option", kTermArgValue, "specify a code optimization level"}
};

static const char* uses[] = {
    "rsync [OPTION]... SRC [SRC]... DEST",
    "rsync [OPTION]... SRC [SRC]... [USER@]HOST:DEST",
    "rsync [OPTION]... SRC [SRC]... [USER@]HOST::DEST"
};

int main(int argc, const char** argv) {
    
    termError("demo", 0, "an error occured");
    termWarn("demo", "this is a warning");
    termInfo("demo", "this info won't show because of the diag level");
    termFilter(kTermInfo);
    termInfo("demo", "this is some information");
    
    puts("\n\n");
    
    TermArgParser parser;
    termArgParserInit(&parser, argc, argv);
    
    for(;;) {
        TermArgResult r = termArgParse(&parser, options, 3);
        switch(r.name) {
            case kTermArgDone: return 0;
            
            case kTermArgHelp:
            termPrintUsage(stdout, "demo", uses, 3);
            puts("");
            termPrintHelp(stdout, options, 3);
            puts("");
            termPrintBugreports(stdout, "demo",
                                "janedoe@email.com",
                                "https://web.site/project");
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