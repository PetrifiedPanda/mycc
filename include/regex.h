#ifndef REGEX_H
#define REGEX_H

#include <stdbool.h>
#include <stdlib.h>

bool is_hex_const(const char* str, size_t num);
bool is_oct_const(const char* str, size_t num);
bool is_dec_const(const char* str, size_t num);
bool is_char_const(const char* str, size_t num);
bool is_float_const(const char* str, size_t num);
bool is_string_literal(const char* str, size_t num);
bool is_valid_identifier(const char* str, size_t num);

#endif