//===--------------------------------------------------------------------------------------------===
// editor_keys.h - Editor key codes
// This source is part of TermUtils
//
// Created on 2019-07-24 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef term_editor_keys_h
#define term_editor_keys_h


typedef enum {
    KEY_CTRL_C      = 0x03,
    KEY_CTRL_D      = 0x04,
    KEY_CTRL_F      = 0x06,
    KEY_CTRL_H      = 0x08,
    KEY_TAB         = 0x09,
    KEY_RETURN      = 0x0a,
    KEY_CTRL_L      = 0x0c,
    KEY_ENTER       = 0x0d,
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
    KEY_PAGE_DOWN
} EditorKey;

#endif