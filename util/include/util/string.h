#ifndef STRING_H
#define STRING_H

#include <stddef.h>

struct string {
    size_t _len;
    union {
        struct {
            // capacity including '\0'
            size_t _cap; 
            char* _data;
        };
        char _static_buf[sizeof(size_t) + sizeof(char*)];
    };
};

struct string create_empty_string(void);

struct string create_string(size_t len, const char* str);

char* string_get_data(struct string* str);

void string_push_back(struct string* str, char c);

void free_string(struct string* str);

#endif

