#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t len;
    const char* data;
} Str;

#define STR_LIT(lit) (Str){.len = (sizeof lit) - 1, .data = (lit)}

Str Str_null(void);

bool Str_valid(Str);

Str Str_advance(Str s, size_t offset);
Str Str_incr(Str s);

Str Str_substr(Str s, size_t begin, size_t end);

char Str_at(Str s, size_t i);

bool Str_eq(Str s1, Str s2);

#endif

