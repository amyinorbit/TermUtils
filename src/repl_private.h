//===--------------------------------------------------------------------------------------------===
// repl_private.h - Private functions used by REPL functions
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef repl_private_h
#define repl_private_h
#include "string.h"
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#include <conio.h>

#define getch _getch

#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

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

typedef struct {
    int length;
    int offset;
} LineInfo;

typedef struct {
    int cx, cy;
    int linesCount;
    LineInfo lines[32];
    String buffer;
} REPL;

void replStart(REPL* repl);

bool moveUp(REPL* repl, int count);
bool moveDown(REPL* repl, int count);
void moveLeft(REPL* repl, int count);
void moveRight(REPL* repl, int count);

bool insertNewLine(REPL* repl);
void print(REPL* repl, char c);

void homeLine(REPL* repl);
void clearLine(REPL* repl);
void clearAll(REPL* repl);


#endif