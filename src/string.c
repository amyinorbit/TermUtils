//===--------------------------------------------------------------------------------------------===
// string.c - string implementation
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "string.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void stringEnsure(String* str, int count) {
    if(count < str->capacity) return;
    while(count >= str->capacity)
        str->capacity = str->capacity ? str->capacity * 2 : 32;
    str->data = realloc(str->data, sizeof(char) * str->capacity);
}

void stringInit(String* str) {
    assert(str && "cannot initialise a null string");
    str->count = 0;
    str->capacity = 0;
    str->data = NULL;
    stringEnsure(str, 16);
}

void stringDeinit(String* str) {
    assert(str && "cannot deinitialise a null string");
    if(str->data) free(str->data);
    str->count = 0;
    str->capacity = 0;
    str->data = NULL;
}

void stringSet(String* str, const char* other) {
    assert(str && "cannot set a null string");
    assert(other && "cannot copy a null char buffer");
    int count = strlen(other);
    stringEnsure(str, count + 1);
    memcpy(str->data, other, count);
    str->data[count] = '\0';
    str->count = count;
}

void stringAppend(String* str, char c) {
    assert(str && "cannot append to a null string");
    stringInsert(str, str->count, c);
}

void stringInsert(String* str, int pos, char c) {
    assert(str && "cannot insert a null string");
    stringEnsure(str, str->count + 1);
    memmove(str->data + pos + 1, str->data + pos, str->count - pos);
    str->data[pos] = c;
    str->count += 1;
    str->data[str->count] = '\0';
    // str->count += 1;
}

void stringErase(String* str, int pos, int count) {
    assert(str && "cannot erase from a null string");
    memmove(str->data + pos, str->data + pos + count, str->count - pos);
    str->count -= count;
    str->data[str->count] = '\0';
}

char* stringTake(String* str) {
    assert(str && "cannot take a null string");
    char* data = str->data;
    str->data = NULL;
    stringDeinit(str);
    return data;
}
