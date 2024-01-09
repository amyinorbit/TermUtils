//===--------------------------------------------------------------------------------------------===
// hexes.h - lightweight API for manipulating standard input and output
// This source is part of TermUtils
//
// Created on 2019-07-24 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef term_hexes_h
#define term_hexes_h
#include <stdbool.h>


typedef enum {
    KEY_CTRL_C      = 0x03,
    KEY_CTRL_D      = 0x04,
    // KEY_CTRL_F      = 0x06,
    // KEY_CTRL_H      = 0x08,
    KEY_TAB         = 0x09,
    // KEY_CTRL_L      = 0x0c,
    KEY_RETURN      = 0x0d,
    KEY_CTRL_Q      = 0x11,
    KEY_CTRL_S      = 0x13,
    KEY_ESC         = 0x1b,
    KEY_BACKSPACE   = 0x7f,

    KEY_ARROW_LEFT  = 500,
    KEY_ARROW_RIGHT,
    KEY_ARROW_UP,
    KEY_ARROW_DOWN,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
} HexesKey;

int hexes_get_char();
int hexes_get_size(int* x, int* y);

HexesKey hexes_get_key();
HexesKey hexes_get_key_raw();

void hexes_set_alternate(bool alt);

void hexes_cursor_up(int n);
void hexes_cursor_down(int n);
void hexes_cursor_left(int n);
void hexes_cursor_right(int n);
void hexes_cursor_go(int x, int y);

void hexes_clear_line();
void hexes_clear_screen();

void hexes_raw_start();
void hexes_raw_stop();



#endif
