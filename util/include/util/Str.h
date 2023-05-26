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

Str Str_create_null(void);
Str Str_create_empty(void);
Str Str_create(size_t len, const char* str);
Str Str_create_empty_with_cap(size_t cap);

bool Str_is_valid(const Str* str);
size_t Str_len(const Str* str);
size_t Str_cap(const Str* str);

const char* Str_get_data(const Str* str);
char Str_char_at(const Str* str, size_t i);

void Str_push_back(Str* str, char c);
void Str_pop_back(Str* str);
void Str_shrink_to_fit(Str* str);
void Str_reserve(Str* str, size_t new_cap);

void Str_remove_front(Str* str, size_t num_chars);

void Str_append_c_str(Str* str, size_t len, const char* c_str);

Str Str_concat(size_t len1, const char* s1, size_t len2, const char* s2);

Str Str_take(Str* str);
Str Str_copy(const Str* str);

void Str_clear(Str* str);

bool Str_eq(const Str* s1, const Str* s2);

void Str_free(const Str* str);

#endif

