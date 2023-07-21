#ifndef MYCC_UTIL_STR_BUF_H
#define MYCC_UTIL_STR_BUF_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

#include "Str.h"

typedef struct {
    union {
        struct {
            uint32_t _is_static_buf : 1;
            uint32_t _len : sizeof(uint32_t) * CHAR_BIT - 1;
            uint32_t _cap;
            char* _data;
        };
        struct {
            bool _is_static_buf_dup : 1;
            uint8_t _small_len : sizeof(uint8_t) * CHAR_BIT - 1;
            char _static_buf[sizeof(uint32_t) * 2 - sizeof(uint8_t)
                             + sizeof(char*)];
        };
    };
} StrBuf;

static_assert(sizeof(StrBuf) == sizeof(uint32_t) * 2 + sizeof(char*),
              "Size of string does not match contents");

StrBuf StrBuf_null(void);
StrBuf StrBuf_create_empty(void);
StrBuf StrBuf_create(Str str);
StrBuf StrBuf_create_empty_with_cap(uint32_t cap);

bool StrBuf_valid(const StrBuf* str);
uint32_t StrBuf_len(const StrBuf* str);
uint32_t StrBuf_cap(const StrBuf* str);

Str StrBuf_as_str(const StrBuf* str);
CStr StrBuf_c_str(StrBuf* str);
char StrBuf_at(const StrBuf* str, uint32_t i);

void StrBuf_push_back(StrBuf* str, char c);
void StrBuf_pop_back(StrBuf* str);
void StrBuf_shrink_to_fit(StrBuf* str);
void StrBuf_reserve(StrBuf* str, uint32_t new_cap);

void StrBuf_remove_front(StrBuf* str, uint32_t num_chars);

void StrBuf_append(StrBuf* str, Str app);

StrBuf StrBuf_concat(Str s1, Str s2);

StrBuf StrBuf_take(StrBuf* str);
StrBuf StrBuf_copy(const StrBuf* str);

void StrBuf_clear(StrBuf* str);

bool StrBuf_eq(const StrBuf* s1, const StrBuf* s2);

void StrBuf_free(const StrBuf* str);

#endif

