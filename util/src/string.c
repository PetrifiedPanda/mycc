#include "util/string.h"

#include <stdlib.h>
#include <string.h>

#include "util/mem.h"

struct string create_empty_string(void) {
    return (struct string){
        ._len = 0,
        ._static_buf = {0},
    };
}

enum {
    STATIC_BUF_LEN = sizeof(struct string){0}._static_buf
                     / sizeof *(struct string){0}._static_buf
};

struct string create_string(size_t len, const char* str) {
    struct string res;
    res._len = len;
    if (len < STATIC_BUF_LEN) {
        memcpy(res._static_buf, str, len * sizeof *res._static_buf);
        res._static_buf[len] = '\0';
    } else {
        res._cap = len + 1;
        res._data = xmalloc(sizeof *res._data * res._cap);
        memcpy(res._data, str, len * sizeof *res._data);
        res._data[len] = '\0';
    }
    return res;
}

char* string_get_data(struct string* str) {
    if (str->_len < STATIC_BUF_LEN) {
        return str->_static_buf;
    } else {
        return str->_data;
    }
}

void string_push_back(struct string* str, char c) {
    if (str->_len < STATIC_BUF_LEN - 1) {
        str->_static_buf[str->_len] = c;
        ++str->_len;
        str->_static_buf[str->_len] = '\0';
    } else if (str->_len == STATIC_BUF_LEN - 1) {
        char* data = xmalloc(sizeof *data * (STATIC_BUF_LEN + 1));
        memcpy(data, str->_static_buf, sizeof *data * (STATIC_BUF_LEN - 1));
        data[str->_len] = c;
        ++str->_len;
        data[str->_len] = '\0';
        str->_cap = str->_len + 1;
        str->_data = data;
    } else {
        if (str->_len == str->_cap - 1) {
            grow_alloc((void**)&str->_data, &str->_cap, sizeof *str->_data);
        }
        str->_data[str->_len] = c;
        ++str->_len;
        str->_data[str->_len] = '\0';
    }
}

void free_string(struct string* str) {
    if (str->_len >= STATIC_BUF_LEN) {
        free(str->_data);
    }
}

