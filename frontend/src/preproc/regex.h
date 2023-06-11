#ifndef REGEX_H
#define REGEX_H

#include <stdbool.h>
#include <stddef.h>

#include "util/Str.h"

bool is_int_const(Str str);
bool is_char_const(Str str);
bool is_float_const(Str str);
bool is_string_literal(Str str);
bool is_valid_identifier(Str str);

#endif
