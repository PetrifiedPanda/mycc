#ifndef REGEX_H
#define REGEX_H

#include <stdbool.h>
#include <stddef.h>

bool is_int_const(const char* str, size_t len);
bool is_char_const(const char* str, size_t len);
bool is_float_const(const char* str, size_t len);
bool is_string_literal(const char* str, size_t len);
bool is_valid_identifier(const char* str, size_t len);

#endif
