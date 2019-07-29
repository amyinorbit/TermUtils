//===--------------------------------------------------------------------------------------------===
// shims.h - Thin wrappers around Win32/UNIX specific terminal functions
// This source is part of TermUtils
//
// Created on 2019-07-25 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef term_shims_h
#define term_shims_h

#ifdef _WIN32
#include <conio.h>

#define getch _getch

#else
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BUFFER_DEFAULT_CAPACITY 512

// static inline int getch(void) {
//     struct termios oldt, newt;
//     int ch;
//     tcgetattr(STDIN_FILENO, &oldt);
//     newt = oldt;
//     newt.c_lflag &= ~(ICANON | ECHO);
//     tcsetattr(STDIN_FILENO, TCSANOW, &newt);
//     ch = getchar();
//     tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
//     return ch;
// }

static inline int getch(void) {
    int ch;
    int result = read(STDIN_FILENO, &ch, 1);
    // fprintf(stderr, "0x%02x\n", ch);
    return result == 1 ? ch : -1;
}

// #define getch getchar
#endif

static inline int tcols(void) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return -1;
	else
		return csbi.srWindow.Right - csbi.srWindow.Left + 1; // Window width
		// return csbi.dwSize.X; // Buffer width
#else
#ifdef TIOCGSIZE
	struct ttysize ts;
	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	return ts.ts_cols;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	return ts.ws_col;
#else // TIOCGSIZE
	return -1;
#endif // TIOCGSIZE
#endif // _WIN32
}

static inline int trows(void) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return -1;
	else
		return csbi.srWindow.Right - csbi.srWindow.Left + 1; // Window width
		// return csbi.dwSize.X; // Buffer width
#else
#ifdef TIOCGSIZE
	struct ttysize ts;
	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	return ts.ts_lines;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	return ts.ws_row;
#else // TIOCGSIZE
	return -1;
#endif // TIOCGSIZE
#endif // _WIN32
}


// TODO: these should have win32 equivalents
static inline void termUp(int n) { if(n) printf("\033[%dA", n); }
static inline void termDown(int n) { if(n) printf("\033[%dB", n); }
static inline void termRight(int n) { if(n) printf("\033[%dC", n); }
static inline void termLeft(int n) { if(n) printf("\033[%dD", n); }
static inline void termClear() { printf("\033[2J");}

static inline void termClearLine() { printf("\033[2K"); }

static inline void termAltStart() { printf("\033[?1049h"); fflush(stdout); }
static inline void termAltStop() { printf("\033[?1049l"); fflush(stdout); }

#endif