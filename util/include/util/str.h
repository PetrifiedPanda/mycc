#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

struct str {
    union {
        struct {
            bool _is_dyn : 1;
            size_t _len : sizeof(size_t) * CHAR_BIT - 1;
            size_t _cap;
            char* _data;
        };
        struct {
            bool _is_dyn_dup : 1;
            uint8_t _small_len : sizeof(uint8_t) * CHAR_BIT - 1;
            char _static_buf[sizeof(size_t) * 2 - sizeof(uint8_t) + sizeof(char*)];
        };
    };
};

struct str create_null_str(void);
struct str create_empty_str(void);
struct str create_str(size_t len, const char* str);

bool str_is_null(const struct str* str);
size_t str_len(const struct str* str);

const char* str_get_data(const struct str* str);

void str_push_back(struct str* str, char c);

struct str str_concat(size_t len1, const char* s1, size_t len2, const char* s2);

struct str str_take(struct str* str);
struct str str_copy(const struct str* str);

void free_str(const struct str* str);

#endif

