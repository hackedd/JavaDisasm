#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

void print_string(FILE* fp, const char* string);
void print_string_w(FILE* fp, const uint32_t* string);

char* escape_string(char* dest, const char* string, size_t length);
char* unescape_string(char* dest, const char* string, size_t length);

#endif
