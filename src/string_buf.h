//===--------------------------------------------------------------------------------------------===
// string.h - Simple dynamic string
// This source is part of TermUtils
//
// Created on 2019-07-22 by Amy Parent <amy@amyparent.com>
// Copyright (c) 2019 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef termutils_string_h
#define termutils_string_h

typedef struct {
    int count;
    int capacity;
    char* data;
} string_buf_t;

void string_buf_init(string_buf_t* str);
void string_buf_fini(string_buf_t* str);

void string_buf_set(string_buf_t* str, const char* other);

// int stringCountLines(const char* str, int length);
// int stringCountColumns(const char* str, int length);

void string_buf_append(string_buf_t* str, char c);
void string_buf_insert(string_buf_t* str, int pos, char c);
void string_buf_erase(string_buf_t* str, int pos, int count);

char* string_buf_take(string_buf_t* str);

#endif