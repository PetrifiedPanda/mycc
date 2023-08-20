#ifndef MYCC_UTIL_STR_H
#define MYCC_UTIL_STR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t len;
    const char* data;
} Str;

typedef struct {
    uint32_t len;
    const char* data;
} CStr;

#define STR_LIT(lit) (const Str){.len = (sizeof lit) - 1, .data = (lit)}
#define CSTR_LIT(lit) (const CStr){.len = (sizeof lit) - 1, .data = (lit)}

Str Str_null(void);
CStr Str_c_str(Str s);

Str CStr_as_str(CStr);

bool Str_valid(Str);

Str Str_advance(Str s, uint32_t offset);
Str Str_incr(Str s);

Str Str_substr(Str s, uint32_t begin, uint32_t end);

char Str_at(Str s, uint32_t i);

bool Str_starts_with(Str s1, Str s2);

bool Str_eq(Str s1, Str s2);

#endif

