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
} String;

void stringInit(String* str);
void stringDeinit(String* str);

void stringSet(String* str, const char* other);

// int stringCountLines(const char* str, int length);
// int stringCountColumns(const char* str, int length);

void stringAppend(String* str, char c);
void stringInsert(String* str, int pos, char c);
void stringErase(String* str, int pos, int count);

char* stringTake(String* str);

#endif