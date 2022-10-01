#include "util/str.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"

struct str create_null_str(void) {
    return (struct str){
        ._is_dyn = true,
        ._len = 0,
        ._cap = 0,
        ._data = NULL,
    };
}

struct str create_empty_str(void) {
    return (struct str){
        ._is_dyn = false,
        ._small_len = 0,
        ._static_buf = {0},
    };
}

enum {
    STATIC_BUF_LEN = sizeof(struct str){0}._static_buf
                     / sizeof *(struct str){0}._static_buf
};

static struct str create_str_with_cap(size_t len, const char* str, size_t cap) {
    assert(cap >= len);
    assert(len == 0 || str);
    struct str res;
    if (cap < STATIC_BUF_LEN) {
        res._is_dyn = false;
        res._small_len = len;
        memcpy(res._static_buf, str, sizeof *res._static_buf * len);
        res._static_buf[len] = '\0';
    } else {
        res._is_dyn = true;
        res._len = len;
        res._cap = cap + 1;
        res._data = xmalloc(sizeof *res._data * res._cap);
        memcpy(res._data, str, sizeof *res._static_buf * len);
        res._data[len] = '\0';
    }
    return res;
}

struct str create_str(size_t len, const char* str) {
    assert(len == 0 || str);
    return create_str_with_cap(len, str, len);
}

bool str_is_null(const struct str* str) {
    assert(str);
    return str->_is_dyn && str->_data == NULL;
}
size_t str_len(const struct str* str) {
    assert(!str_is_null(str));
    if (str->_is_dyn) {
        return str->_len;
    } else {
        return str->_small_len;
    }
}

static char* str_get_mut_data(struct str* str) {
    assert(str);
    assert(!str_is_null(str));
    if (str->_is_dyn) {
        return str->_data;
    } else {
        return str->_static_buf;
    }
}

const char* str_get_data(const struct str* str) {
    assert(str);
    assert(!str_is_null(str));
    if (str->_is_dyn) {
        return str->_data;
    } else {
        return str->_static_buf;
    }
}

void str_push_back(struct str* str, char c) {
    if (str->_is_dyn) {
        if (str->_len == str->_cap - 1) {
            grow_alloc((void**)&str->_data, &str->_cap, sizeof *str->_data);
        }
        str->_data[str->_len] = c;
        ++str->_len;
        str->_data[str->_len] = '\0';
    } else {
        if (str->_small_len == STATIC_BUF_LEN - 1) {
            size_t len = str->_small_len;
            char* data = xmalloc(sizeof *data * (STATIC_BUF_LEN + 1));
            memcpy(data, str->_static_buf, sizeof *data * (STATIC_BUF_LEN - 1));
            data[len] = c;
            ++len;
            data[len] = '\0';
            str->_cap = len + 1;
            str->_len = len;
            str->_data = data;
            str->_is_dyn = true;
        } else {
            str->_static_buf[str->_small_len] = c;
            ++str->_small_len;
            str->_static_buf[str->_small_len] = '\0';
        }

    }
}

struct str str_concat(size_t len1,
                      const char* s1,
                      size_t len2,
                      const char* s2) {
    const size_t len = len1 + len2;
    struct str res = create_str_with_cap(len1, s1, len);
    char* res_data = str_get_mut_data(&res);
    memcpy(res_data + len1, s2, len2 * sizeof *res_data);
    res_data[len] = '\0';
    if (res._is_dyn) {
        res._len = len;
    } else {
        res._small_len = len;
    }
    return res;
}

struct str str_move(struct str* str) {
    assert(str);
    assert(!str_is_null(str));
    struct str res = *str;
    str->_is_dyn = true;
    str->_len = 0;
    str->_cap = 0;
    str->_data = NULL;
    return res;
}

struct str str_copy(const struct str* str) {
    assert(str);
    assert(!str_is_null(str));
    if (str->_is_dyn) {
        return create_str_with_cap(str->_len, str->_data, str->_cap - 1);
    } else {
        return *str;
    }
}

void free_str(const struct str* str) {
    assert(str);
    if (str->_is_dyn) {
        free(str->_data);
    }
}

