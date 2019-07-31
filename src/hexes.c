//===--------------------------------------------------------------------------------------------===
// hexes.h - lightweight API for manipulating standard input and output
// This source is part of TermUtils
//
// Created on 2019-07-24 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/hexes.h>
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>

#define getch _getch

#else
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#endif

int hexesGetCh() {
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

HexesKey hexesGetKey() {
    int buf[3];

    char c = hexesGetCh();
    switch(c) {
    case KEY_ESC:
        if((buf[0] = hexesGetCh()) < 0) return KEY_ESC;
        if((buf[1] = hexesGetCh()) < 0) return KEY_ESC;

        if(buf[0] == '[') {
            // If we have a digit, then we have an extended escape sequence
            if(isdigit(buf[1])) {
                if((buf[2] = hexesGetCh()) < 0) return KEY_ESC;
                if(buf[2] == '~') {
                    switch(buf[1]) {
                        case '3': return KEY_DELETE;
                        case '5': return KEY_PAGE_UP;
                        case '6': return KEY_PAGE_DOWN;
                    }
                }
            } else {
                switch(buf[1]) {
                    case 'A': return KEY_ARROW_UP;
                    case 'B': return KEY_ARROW_DOWN;
                    case 'C': return KEY_ARROW_RIGHT;
                    case 'D': return KEY_ARROW_LEFT;
                }
            }
        }
        break;
    default:
        return c;
    }
    return -1;
}

HexesKey hexesGetKeyRaw() {
    int buf[3];

    char c = getchar();
    switch(c) {
    case KEY_ESC:
        if((buf[0] = getchar()) < 0) return KEY_ESC;
        if((buf[1] = getchar()) < 0) return KEY_ESC;

        if(buf[0] == '[') {
            // If we have a digit, then we have an extended escape sequence
            if(isdigit(buf[1])) {
                if((buf[2] = getchar()) < 0) return KEY_ESC;
                if(buf[2] == '~') {
                    switch(buf[1]) {
                        case '3': return KEY_DELETE;
                        case '5': return KEY_PAGE_UP;
                        case '6': return KEY_PAGE_DOWN;
                    }
                }
            } else {
                switch(buf[1]) {
                    case 'A': return KEY_ARROW_UP;
                    case 'B': return KEY_ARROW_DOWN;
                    case 'C': return KEY_ARROW_RIGHT;
                    case 'D': return KEY_ARROW_LEFT;
                }
            }
        }
        break;
    default:
        return c;
    }
    return -1;
}

int hexesGetSize(int* x, int* y) {
    #ifdef _WIN32
    	CONSOLE_SCREEN_BUFFER_INFO info;
    	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
    		return -1;
        if(x) *x = info.srWindow.Right - info.srWindow.Left + 1;
        if(y) *y = info.srWindow.Bottom - info.srWindow.Top + 1
        return 0;
    #else
    #ifdef TIOCGSIZE
    	struct ttysize ts;
    	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
        if(x) *x = ts.ts_cols;
        if(y) *y = ts.ts_lines;
        return 0;
    #elif defined(TIOCGWINSZ)
    	struct winsize ts;
    	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
        if(x) *x = ts.ws_col;
        if(y) *y = ts.ws_row;
        return 0;
    #else // TIOCGSIZE
    	return -1;
    #endif // TIOCGSIZE
    #endif // _WIN32
}

void hexesScreenAlternate(bool alt) {
    if(alt)
        printf("\033[?1049h");
    else
        printf("\033[?1049l");
}

void hexesCursorUp(int n) {
    #ifdef _WIN32
    #else
    if(n)printf("\033[%dA", n);
    #endif
}

void hexesCursorDown(int n) {
    #ifdef _WIN32
    #else
    if(n)printf("\033[%dB", n);
    #endif
}

void hexesCursorLeft(int n) {
    #ifdef _WIN32
    #else
    if(n)printf("\033[%dD", n);
    #endif
}

void hexesCursorRight(int n) {
    #ifdef _WIN32
    #else
    if(n)printf("\033[%dC", n);
    #endif
}

void hexesCursorGo(int x, int y) {
    #ifdef _WIN32
    #else
    // printf("\033[%d;1H\033[2K", 1 + i);
    printf("\033[%d;%dH", y+1, x+1);
    #endif
}

static bool inRawMode = false;

#ifndef _WIN32
static struct termios savedTerm;
#endif

void hexesStartRawMode() {
    if(inRawMode) return;
#ifdef _WIN32
#else
    struct termios raw;

    if (!isatty(STDIN_FILENO)) return;
    // atexit(editorAtExit);
    if (tcgetattr(STDIN_FILENO,&savedTerm) == -1) return;

    raw = savedTerm;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    /* control chars - set return condition: min number of bytes and timer. */
    // raw.c_cc[VMIN] = 0; /* Return each byte, or zero for timeout. */
    // raw.c_cc[VTIME] = 1; /* 100 ms timeout (unit is tens of second). */
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&raw) < 0) return;
    inRawMode = true;
#endif
}

void hexesStopRawMode() {
    if(!inRawMode) return;
#ifdef _WIN32
#else
    tcsetattr(STDIN_FILENO,TCSAFLUSH,&savedTerm);
    inRawMode = false;
#endif
}

void hexesClearLine() {
#ifdef _WIN32
#else
    printf("\033[2K");
#endif
}

void hexesClearScreen() {
#ifdef _WIN32
#else
    printf("\033[2J");
#endif
}
