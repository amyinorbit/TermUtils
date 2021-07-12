//===--------------------------------------------------------------------------------------------===
// line.c - Line editing library
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <term/line.h>
#include <term/hexes.h> // Could be moved back to private headers
#include "string_buf.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct hist_entry_s {
    struct hist_entry_s* prev;
    struct hist_entry_s* next;
    char line[];
} hist_entry_t;

typedef struct {
    int key;
    line_binding_t function;
    line_cmd_t default_cmd; // if [function] is null, we return this
} binding_data_t;

struct line_t {
    int cursor;
    
    char prompt[32];
    
    line_functions_t functions;
    string_buf_t buffer;
    
    hist_entry_t* tail;
    hist_entry_t* head;
    hist_entry_t* current;
};

#define CMD_NOTHING (line_cmd_t){LINE_STAY, 0}
#define CMD(action, param) (line_cmd_t){(action), (param)}

// MARK: - Terminal manipulation.

static void put_char(char c) {
    putchar(c);
}

static void put_string(const char* str) {
    while(*str)
        put_char(*str++);
}

static int show_char(line_t* line, char c) {
    if(IS_CTL(c)) {
        term_set_fg(stdout, TERM_BLACK);
        put_char('^');
        put_char(DE_CTL(c));
        term_set_fg(stdout, TERM_DEFAULT);
        return 2;
    }
    put_char(c & 0x7f);
    return 1;
}

static int show_string(line_t* line, const char* str) {
    int total = 0;
    while(*str)
        total += show_char(line, *str++);
    return total;
}

static void back(line_t* line, line_action_t mode) {
    if(mode == LINE_MOVE && !line->cursor) return;
    if(IS_CTL(line->buffer.data[line->cursor-1])) {
        put_char('\b');
    }
    if(mode == LINE_MOVE) line->cursor -= 1;
    put_char('\b');
}

static void back_n(line_t* line, line_action_t mode, int n) {
    while(n--)
        back(line, mode);
}

static void forward(line_t* line) {
    if(line->cursor >= line->buffer.count) return;
    show_char(line, line->buffer.data[line->cursor]);
    line->cursor += 1;
}

static void forward_n(line_t* line, int n) {
    while(n--)
        forward(line);
}

// MARK: - typing handling

// re-print the line after the cursor, to make sure we don't have any remaining stray characters.
static line_cmd_t finish_line(line_t* line) {
    int move = show_string(line, &line->buffer.data[line->cursor]);
    put_string("\e[0K");
    return CMD(LINE_STAY, -move);
}

static line_cmd_t insert(line_t* line, int key) {
    string_buf_insert(&line->buffer, line->cursor, key & 0x00ff);
    show_char(line, key);
    line->cursor += 1;
    if(line->cursor == line->buffer.count) return CMD_NOTHING;
    return finish_line(line);
}

static line_cmd_t backspace(line_t* line, int key) {
    if(!line->buffer.count || !line->cursor) return CMD_NOTHING;
    back(line, LINE_MOVE);
    string_buf_erase(&line->buffer, line->cursor, 1);
    return finish_line(line);
}

static line_cmd_t delete(line_t* line, int key) {
    if(!line->buffer.count || line->cursor >= line->buffer.count) return CMD_NOTHING;
    string_buf_erase(&line->buffer, line->cursor, 1);
    return finish_line(line);
}

static line_cmd_t ctrl_d(line_t *line, int key) {
    if(line->buffer.count) {
        return delete(line, key);
    } else {
        return CMD(LINE_DONE, 0);
    }
}

static line_cmd_t clear(line_t* line, int key) {
    line->cursor = 0;
    line->buffer.count = 0;
    if(line->buffer.capacity) line->buffer.data[0] = '\0';
    return CMD(LINE_CANCEL, 0);
}

// MARK: - Built-in utils

static void show_prompt(const line_t* line) {
    if(line->functions.print_prompt) {
        line->functions.print_prompt(line->prompt);
    } else {
        put_string(line->prompt);
        put_string("> ");
    }
}

static void reset(line_t* line) {
    line->buffer.count = 0;
    if(line->buffer.capacity) line->buffer.data[0] = '\0';
    line->cursor = 0;
}

// MARK: - History Management
// TODO: we should save the current buffer when scrolling through the history

static void free_history(line_t* line) {
    hist_entry_t* entry = line->head;
    while(entry) {
        hist_entry_t* prev = entry->prev;
        free(entry);
        entry = prev;
    }
}

static line_cmd_t history_prev(line_t* line, int key) {
    if(!line->current && !line->head) return CMD_NOTHING;
    
    hist_entry_t* saved = line->current;
    line->current = line->current ? line->current->prev : line->head;
    
    if(!line->current) {
        line->current = saved;
        return CMD_NOTHING;
    }
    
    string_buf_set(&line->buffer, line->current->line);
    line->cursor = line->buffer.count;
    return CMD(LINE_REFRESH, 0);
}

static line_cmd_t history_next(line_t* line, int key) {
    if(!line->current) return CMD_NOTHING;
    line->current = line->current->next;
    
    if(line->current) {
        string_buf_set(&line->buffer, line->current->line);
    } else {
        reset(line);
    }
    line->cursor = line->buffer.count;
    return CMD(LINE_REFRESH, 0);
}

// MARK: - Default bindings & binding dispatch
// TODO: this should probably get moved to the line_t* object itself, once we add custom bindings

static const binding_data_t bindings[] = {
    {CTL('d'),          &ctrl_d,        CMD_NOTHING},
    {CTL('c'),          &clear,         CMD_NOTHING},
    {CTL('m'),          NULL,           CMD(LINE_RETURN, 0)},
    {CTL('J'),          NULL,           CMD(LINE_RETURN, 0)},
    // {CTL('l'),          NULL,           CMD(LINE_REFRESH, 0)},
    
    {KEY_BACKSPACE,     &backspace,     CMD_NOTHING},
    {KEY_DELETE,        &delete,        CMD_NOTHING},
    
    {CTL('b'),          NULL,           CMD(LINE_MOVE, -1)},
    {KEY_ARROW_LEFT,    NULL,           CMD(LINE_MOVE, -1)},
    {CTL('f'),          NULL,           CMD(LINE_MOVE, 1)},
    {KEY_ARROW_RIGHT,   NULL,           CMD(LINE_MOVE, 1)},
    
    {CTL('p'),          &history_prev,  CMD_NOTHING},
    {KEY_ARROW_UP,      &history_prev,  CMD_NOTHING},
    {CTL('n'),          &history_next,  CMD_NOTHING},
    {KEY_ARROW_DOWN,    &history_next,  CMD_NOTHING},
    
    {0,                 NULL,           CMD_NOTHING},
};

static line_cmd_t dispatch(line_t* line, int key) {
    
    for(int i = 0; bindings[i].key != 0; ++i) {
        if(bindings[i].key != key) continue;
        return bindings[i].function ?
            bindings[i].function(line, key) :
            bindings[i].default_cmd;
    }
    return insert(line, key);
}

// MARK: - Public line_t API

line_t* line_new(const line_functions_t* functions) {
    line_t* line = malloc(sizeof(line_t));
    assert(line && "line editor allocation failed");
    strcpy(line->prompt, "");
    line->cursor = 0;
    
    line->functions = *functions;
    string_buf_init(&line->buffer);
    
    line->head = NULL;
    line->current = NULL;
    
    return line;
}

void line_destroy(line_t* line) {
    assert(line && "cannot deallocate a null line");
    string_buf_fini(&line->buffer);
    free_history(line);
    free(line);
}

void line_run_cmd(line_action_t action, void* param) {
    
}

void line_set_prompt(line_t* line, const char* prompt) {
    assert(line && "cannot set prompt on null line editor");
    assert(prompt && "prompt cannot be null");
    strncpy(line->prompt, prompt, sizeof(line->prompt)-1);
}

char* line_get(line_t* line) {
    hexes_raw_start();
    reset(line);
    
    char* result = NULL;
    put_string("\r\e[2K");
    show_prompt(line);
    for(;;) {
        int key = hexes_get_key_raw();
        line_cmd_t cmd = dispatch(line, key);
        
        switch(cmd.action) {
        case LINE_STAY:
            if(cmd.param >= 0) break;
            if(cmd.param < 0) back_n(line, LINE_STAY, -cmd.param);
            break;
            
        case LINE_DONE:
            show_char(line, key);
            put_string("\r\n");
            result = NULL;
            goto done;
            
        case LINE_RETURN:
            if(!line->buffer.count) {
                put_string("\r\n");
                show_prompt(line);
            } else {
                put_string("\n\r");
                string_buf_append(&line->buffer, '\n');
                result = string_buf_take(&line->buffer);
                goto done;
            }
            break;
            
        case LINE_MOVE:
            if(cmd.param < 0) back_n(line, LINE_MOVE, -cmd.param);
            if(cmd.param > 0) forward_n(line, cmd.param);
            break;
            
        case LINE_REFRESH:
            put_string("\r\e[2K");
            show_prompt(line);
            show_string(line, line->buffer.data);
            break;
            
        case LINE_CANCEL:
            show_char(line, key);
            put_string("\r\n");
            show_prompt(line);
            break;
            
        }
    }
    
done:
    hexes_raw_stop();
    fflush(stdout);
    line->current = NULL;
    if(result) line_history_add(line, result);
    return result;
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

void line_history_load(line_t* line, const char* path) {
    
    FILE* history = fopen(path, "rb");
    if(!history) return;
    
    char* buffer = NULL;
    size_t size = 0;
    
    while(getline(&buffer, &size, history) > 0) {
        line_history_add(line, buffer);
    }
    
    free(buffer);
    fclose(history);
}

void line_history_write(line_t* line, const char* path) {
    FILE* history = fopen(path, "wb");
    if(!history) return;
    
    hist_entry_t* entry = line->tail;
    while(entry) {
        fprintf(history, "%s\n", entry->line);
        entry = entry->next;
    }
    fclose(history);
}

void line_history_add(line_t* line, const char* data) {
    int length = strlen(data);
    hist_entry_t* entry = malloc(sizeof(hist_entry_t) + (length + 1) * sizeof(char));
    memcpy(entry->line, data, length);
    entry->line[length] = '\0';
    strip(entry->line);
    
    entry->next = NULL;
    if(line->head) {
        entry->prev = line->head;
        line->head->next = entry;
    } else {
        entry->prev = NULL;
        line->tail = entry;
    }
    line->head = entry;
}
