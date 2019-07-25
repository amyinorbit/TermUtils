//===--------------------------------------------------------------------------------------------===
// repl.c - REPL interface utilities
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/repl.h>
#include "editor.h"
#include "shims.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void termREPLInit(TermREPL* repl, const char* prompt, TermColor promptColor) {
    assert(repl && "cannot initialise a null REPL");
    repl->prompt = prompt;
    repl->promptColor = promptColor;
    repl->historyCount = 0;
    for(int i = 0; i < TERM_MAX_HISTORY; ++i) {
        repl->history[i] = NULL;
    }
    
}

void termREPLDeinit(TermREPL* repl) {
    for(char** str = repl->history; str != repl->history + TERM_MAX_HISTORY; ++str) {
        if(!*str) break;
        free(*str);
        *str = NULL;
    }
}

static char* strip(char* data) {
    char* end = data;
    while(*end) end += 1;
    if(*end != '\n' && *end != ' ' && *end != '\0') return data;
    
    for(; end > data; --end) {
        if(end[-1] != '\n' && end[-1] != ' ') break;
    }
    *end = '\0';
    return data;
}

char* termREPL(TermREPL* repl) {
    
    int historyIndex = -1;
    
    Editor editor;
    termEditorInit(&editor, repl->prompt);
    
    EditorStatus status;
    int braces = 0;
    int parens = 0;
    
    char* output = NULL;
    
    for(;;) {
        termEditorRender(&editor);
        char c = getch();
        status = termEditorUpdate(&editor, c);
        switch(c) {
            case '(': parens += 1; break;
            case ')': parens -= 1; break;
            case '{': braces += 1; break;
            case '}': braces -= 1; break;
            default: break;
        }
        
        switch(status) {
        case kTermEditorDone:
            goto cleanup;
            
        case kTermEditorTop:
            if(historyIndex < repl->historyCount - 1) {
                historyIndex += 1;
                termEditorReplace(&editor, repl->history[historyIndex]);
            }
            break;
            
        case kTermEditorBottom:
            if(historyIndex > 0) {
                historyIndex -= 1;
                termEditorReplace(&editor, repl->history[historyIndex]);
            } else if(historyIndex == 0) {
                historyIndex = -1;
                termEditorReplace(&editor, "");
            }
            break;
            
        case kTermEditorReturn:
            if(braces) {
                for(int i = 0; i < braces * 4; ++i) 
                    termEditorUpdate(&editor, ' ');
            }
            if(parens || braces) break;
            
            output = termEditorFlush(&editor);
            int length = strlen(output);
            
            memmove(repl->history+1, repl->history, (TERM_MAX_HISTORY-1) * sizeof(char*));
            repl->history[0] = malloc((1 + length) * sizeof(char));
            memcpy(repl->history[0], output, length);
            repl->history[0][length] = '\0';
            strip(repl->history[0]);
            
            repl->historyCount += 1;
            if(repl->historyCount > TERM_MAX_HISTORY) repl->historyCount = TERM_MAX_HISTORY;
            goto cleanup;
            
        case kTermEditorOK:
            break;
        }
    }
    
    cleanup:
    termEditorDeinit(&editor);
    return output;
}
