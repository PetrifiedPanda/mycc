#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

typedef struct {
    union {
        struct {
            size_t _is_static_buf : 1;
            size_t _len : sizeof(size_t) * CHAR_BIT - 1;
            size_t _cap;
            char* _data;
        };
        struct {
            bool _is_static_buf_dup : 1;
            uint8_t _small_len : sizeof(uint8_t) * CHAR_BIT - 1;
            char _static_buf[sizeof(size_t) * 2 - sizeof(uint8_t)
                             + sizeof(char*)];
        };
    };
} Str;

static_assert(sizeof(Str) == sizeof(size_t) * 2 + sizeof(char*),
              "Size of string does not match contents");

Str create_null_str(void);
Str create_empty_str(void);
Str create_str(size_t len, const char* str);
Str create_empty_str_with_cap(size_t cap);

bool str_is_valid(const Str* str);
size_t str_len(const Str* str);
size_t str_cap(const Str* str);

const char* str_get_data(const Str* str);
char str_char_at(const Str* str, size_t i);

void str_push_back(Str* str, char c);
void str_pop_back(Str* str);
void str_shrink_to_fit(Str* str);
void str_reserve(Str* str, size_t new_cap);

void str_remove_front(Str* str, size_t num_chars);

void str_append_c_str(Str* str, size_t len, const char* c_str);

Str str_concat(size_t len1, const char* s1, size_t len2, const char* s2);

Str str_take(Str* str);
Str str_copy(const Str* str);

void str_clear(Str* str);

bool str_eq(const Str* s1, const Str* s2);

void free_str(const Str* str);

#endif

