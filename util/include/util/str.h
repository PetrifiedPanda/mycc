#ifndef STRING_H
#define STRING_H

#include <stddef.h>

struct str {
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

struct str create_empty_str(void);

struct str create_str(size_t len, const char* str);

char* str_get_data(struct str* str);

void str_push_back(struct str* str, char c);

void free_str(struct str* str);

#endif

