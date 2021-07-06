//===--------------------------------------------------------------------------------------------===
// string.c - string implementation
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "string_buf.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void string_buf_ensure(string_buf_t* str, int count) {
    if(count < str->capacity) return;
    while(count >= str->capacity)
        str->capacity = str->capacity ? str->capacity * 2 : 32;
    str->data = realloc(str->data, sizeof(char) * str->capacity);
}

void string_buf_init(string_buf_t* str) {
    assert(str && "cannot initialise a null string");
    str->count = 0;
    str->capacity = 0;
    str->data = NULL;
    string_buf_ensure(str, 16);
}

void string_buf_fini(string_buf_t* str) {
    assert(str && "cannot deinitialise a null string");
    if(str->data) free(str->data);
    str->count = 0;
    str->capacity = 0;
    str->data = NULL;
}

void string_buf_set(string_buf_t* str, const char* other) {
    assert(str && "cannot set a null string");
    assert(other && "cannot copy a null char buffer");
    int count = strlen(other);
    string_buf_ensure(str, count + 1);
    memcpy(str->data, other, count);
    str->data[count] = '\0';
    str->count = count;
}

void string_buf_append(string_buf_t* str, char c) {
    assert(str && "cannot append to a null string");
    string_buf_insert(str, str->count, c);
}

void string_buf_insert(string_buf_t* str, int pos, char c) {
    assert(str && "cannot insert a null string");
    string_buf_ensure(str, str->count + 1);
    memmove(str->data + pos + 1, str->data + pos, str->count - pos);
    str->data[pos] = c;
    str->count += 1;
    str->data[str->count] = '\0';
    // str->count += 1;
}

void string_buf_erase(string_buf_t* str, int pos, int count) {
    assert(str && "cannot erase from a null string");
    memmove(str->data + pos, str->data + pos + count, str->count - pos);
    str->count -= count;
    str->data[str->count] = '\0';
}

char* string_buf_take(string_buf_t* str) {
    assert(str && "cannot take a null string");
    char* data = str->data;
    str->data = NULL;
    string_buf_fini(str);
    return data;
}
