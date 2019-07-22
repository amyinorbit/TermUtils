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
#include <assert.h>
#include <stdlib.h>
#include <string.h>


#ifdef _WIN32
#include <conio.h>

#define getch _getch

#else
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#define BUFFER_DEFAULT_CAPACITY 512

static inline int getch(void) {
    struct termios oldt, newt;
	int ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}
#endif

typedef struct Buffer {
    int count;
    int capacity;
    char* data;
} Buffer;

static void bufferInit(Buffer* buf) {
    buf->data = malloc(sizeof(char) * BUFFER_DEFAULT_CAPACITY);
    buf->capacity = BUFFER_DEFAULT_CAPACITY;
    buf->count = 0;
}

static void bufferInsert(Buffer* buf, int pos, char c) {
    if(buf->count + 2 >= buf->capacity) {
        buf->capacity *= 2;
        buf->data = realloc(buf->data, sizeof(char) * buf->capacity);
    }
    memmove(buf->data + pos + 1, buf->data + pos, buf->count - pos);
    buf->data[pos] = c;
    buf->data[++buf->count] = '\0';
}

static void bufferErase(Buffer* buf, int pos, int count) {
    memmove(buf->data + pos, buf->data + pos + count, buf->count - pos);
    buf->count -= count;
    buf->data[buf->count] = '\0';
}

static void bufferReplace(Buffer* buf, const char* data) {
    int length = strlen(data);
    if(length + 1 >= buf->capacity) {
        while(length + 1 >= buf->capacity)
            buf->capacity *= 2;
        buf->data = realloc(buf->data, sizeof(char) * buf->capacity);
    }
    memcpy(buf->data, data, length);
    buf->data[length] = '\0';
    buf->count = length;
}

static void goBack(int count) {
    for(int i = 0; i < count; ++i) putchar('\b');
}

static void pad(int count) {
    for(int i = 0; i < count; ++i) putchar(' ');
}

static void erase(int count, int pos) {
    goBack(pos);
    pad(count);
    goBack(count);
}


static inline int historyLength(const TermREPL* repl) {
    int size = 0;
    for(int i = 0; i < TERM_MAX_HISTORY; ++i) {
        if(!repl->history[i]) break;
        size += 1;
    }
    return size;
}

void termREPLInit(TermREPL* repl) {
    assert(repl && "cannot initialise a null REPL");
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

const char* termREPL(const char* prompt, TermREPL* repl) {
    
    Buffer buffer;
    bufferInit(&buffer);
    
    int historyLen = historyLength(repl);
    int historyIndex = historyLen;
    
    int start = 0;
    int cursor = 0;
    int braceCount = 0;
    int parenCount = 0;
    int promptLength = strlen(prompt);
    printf("%s", prompt);
    
    for(;;) {
        int c = getch();
        
        if(c == 127) {
            if(cursor <= 0) continue;
            erase(buffer.count - start, cursor);
            bufferErase(&buffer, start + (--cursor), 1);
            printf("%s", buffer.data + start);
            goBack(buffer.count - (cursor + start));
            continue;
        }
        
        if(c == 27 && getch() == 91) {
            switch(getch()) {
                
            case 65:
                if(historyIndex > 0) {
                    erase(buffer.count - start, cursor);
                    bufferReplace(&buffer, repl->history[--historyIndex]);
                    printf("%s", buffer.data + start);
                    cursor = buffer.count;
                }
                break;
                
            case 66:
                if(historyIndex+1 < historyLen) {
                    erase(buffer.count - start, cursor);
                    bufferReplace(&buffer, repl->history[++historyIndex]);
                    printf("%s", buffer.data + start);
                    cursor = buffer.count;
                }
                break;
                
            // right arrow
            case 67:
                if(cursor < (buffer.count - start)) putchar(buffer.data[start + cursor++]);
                break;
                
            // left arrow
            case 68:
                if(cursor > 0) {
                    putchar('\b');
                    cursor -= 1;
                }
                break;
            }
            continue;
        }
        bufferInsert(&buffer, start + cursor++, c);
        
        if(c == '\n' && braceCount == 0 && parenCount == 0) {
            putchar('\n');
            
            
            buffer.data = realloc(buffer.data, sizeof(char) * (buffer.count + 1));
            repl->history[historyIndex] = buffer.data;
            if(historyIndex+1 >= TERM_MAX_HISTORY) {
                memmove(repl->history, repl->history+1, sizeof(char*));
                repl->history[historyIndex] = NULL;
            }
            return buffer.data;
        }
        
        if(c == '(') parenCount += 1;
        if(c == ')') parenCount -= 1;
        if(c == '{') braceCount += 1;
        if(c == '}') braceCount -= 1;
        if(braceCount < 0) braceCount = 0;
        if(parenCount < 0) parenCount = 0;
        
        historyIndex = historyLength(repl);
        
        if(c == '\n') {
            start += cursor;
            cursor = 0;
            printf("\n%-*s", promptLength, "...");
        } else {
            erase(buffer.count, cursor-1);
        }
        
        printf("%s", buffer.data + start);
        goBack(buffer.count - (cursor + start));
    }
    return NULL;
}
