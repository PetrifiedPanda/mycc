#ifndef UTIL_STR_H
#define UTIL_STR_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t len;
    const char* data;
} Str;

typedef struct {
    size_t len;
    const char* data;
} CStr;

#define STR_LIT(lit) (Str){.len = (sizeof lit) - 1, .data = (lit)}
#define CSTR_LIT(lit) (CStr){.len = (sizeof lit) - 1, .data = (lit)}

Str Str_null(void);
CStr Str_c_str(Str s);

Str CStr_as_str(CStr);

bool Str_valid(Str);

Str Str_advance(Str s, size_t offset);
Str Str_incr(Str s);

Str Str_substr(Str s, size_t begin, size_t end);

char Str_at(Str s, size_t i);

bool Str_starts_with(Str s1, Str s2);

bool Str_eq(Str s1, Str s2);

#endif

