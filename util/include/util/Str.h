#ifndef MYCC_UTIL_STR_H
#define MYCC_UTIL_STR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Str {
    uint32_t len;
    const char* data;
} Str;

typedef struct CStr {
    uint32_t len;
    const char* data;
} CStr;

#define STR_LIT(lit) (const Str){.len = (sizeof lit) - 1, .data = (lit)}
#define CSTR_LIT(lit) (const CStr){.len = (sizeof lit) - 1, .data = (lit)}

inline Str Str_null(void);
inline CStr Str_c_str(Str s);

inline Str CStr_as_str(CStr);

inline bool Str_valid(Str);

inline Str Str_advance(Str s, uint32_t offset);
inline Str Str_incr(Str s);

inline Str Str_substr(Str s, uint32_t begin, uint32_t end);

inline char Str_at(Str s, uint32_t i);

inline bool Str_starts_with(Str s1, Str s2);

inline bool Str_eq(Str s1, Str s2);

#ifndef MYCC_STR_INLINE
#define MYCC_STR_INLINE inline
#endif

#include "Str.inc"

#endif

