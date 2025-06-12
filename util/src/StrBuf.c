#include "util/StrBuf.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"

StrBuf StrBuf_null(void) {
    return (StrBuf){
        ._is_static_buf = false,
        ._len = 0,
        ._cap = 0,
        ._data = NULL,
    };
}

StrBuf StrBuf_create_empty(void) {
    StrBuf res = {
        ._is_static_buf_dup = true,
        ._small_len = 0,
        ._static_buf = {0},
    };
    return res;
}

enum {
    STR_BUF_STATIC_LEN = sizeof(StrBuf){{{0}}}._static_buf
                         / sizeof *(StrBuf){{{0}}}._static_buf
};

static StrBuf StrBuf_create_with_cap(Str str, uint32_t cap) {
    assert(cap >= str.len);
    assert(str.len == 0 || str.data);
    StrBuf res;
    if (cap < STR_BUF_STATIC_LEN) {
        res._is_static_buf = true;
        res._small_len = (uint8_t)str.len;
        memcpy(res._static_buf, str.data, sizeof *res._static_buf * str.len);
        res._static_buf[str.len] = '\0';
    } else {
        res._is_static_buf = false;
        res._len = str.len;
        res._cap = cap + 1;
        res._data = mycc_alloc(sizeof *res._data * res._cap);
        memcpy(res._data, str.data, sizeof *res._static_buf * str.len);
        res._data[str.len] = '\0';
    }
    return res;
}

StrBuf StrBuf_create(Str str) {
    assert(str.len == 0 || str.data);
    return StrBuf_create_with_cap(str, str.len);
}

StrBuf StrBuf_create_empty_with_cap(uint32_t cap) {
    StrBuf res;
    if (cap < STR_BUF_STATIC_LEN) {
        res._is_static_buf = true;
        res._small_len = 0;
        res._static_buf[0] = '\0';
    } else {
        res._is_static_buf = false;
        res._len = 0;
        res._cap = cap + 1;
        res._data = mycc_alloc(sizeof *res._data * res._cap);
        res._data[0] = '\0';
    }
    return res;
}

bool StrBuf_valid(const StrBuf* str) {
    assert(str);
    return str->_is_static_buf || str->_data != NULL;
}

uint32_t StrBuf_len(const StrBuf* str) {
    assert(StrBuf_valid(str));
    if (str->_is_static_buf) {
        return str->_small_len;
    } else {
        return str->_len;
    }
}

uint32_t StrBuf_cap(const StrBuf* str) {
    assert(StrBuf_valid(str));
    if (str->_is_static_buf) {
        return STR_BUF_STATIC_LEN - 1;
    } else {
        return str->_cap - 1;
    }
}

static char* StrBuf_get_mut_data(StrBuf* str) {
    assert(str);
    assert(StrBuf_valid(str));
    if (str->_is_static_buf) {
        return str->_static_buf;
    } else {
        return str->_data;
    }
}

Str StrBuf_as_str(const StrBuf* str) {
    assert(str);
    if (str->_is_static_buf) {
        return (Str){
            .len = str->_small_len,
            .data = str->_static_buf,
        };
    } else {
        return (Str){
            .len = str->_len,
            .data = str->_data,
        };
    }
}

CStr StrBuf_c_str(StrBuf* str) {
    assert(str);
    return Str_c_str(StrBuf_as_str(str));
}

char StrBuf_at(const StrBuf* str, uint32_t i) {
    assert(str);
    assert(StrBuf_valid(str));
    if (str->_is_static_buf) {
        assert(i < str->_small_len);
        return str->_static_buf[i];
    } else {
        assert(i < str->_len);
        return str->_data[i];
    }
}

static void StrBuf_move_to_dyn_buf(StrBuf* str, uint32_t dyn_buf_cap) {
    assert(str->_is_static_buf);

    const uint32_t len = str->_small_len;
    const uint32_t real_cap = dyn_buf_cap + 1;
    char* data = mycc_alloc(sizeof *data * real_cap);
    memcpy(data, str->_static_buf, sizeof *data * (len + 1));
    str->_cap = real_cap;
    str->_len = len;
    str->_data = data;
    str->_is_static_buf = false;
}

void StrBuf_push_back(StrBuf* str, char c) {
    assert(str);
    assert(StrBuf_valid(str));
    if (str->_is_static_buf) {
        if (str->_small_len == STR_BUF_STATIC_LEN - 1) {
            StrBuf_move_to_dyn_buf(str, STR_BUF_STATIC_LEN);
            str->_data[str->_len] = c;
            ++str->_len;
            str->_data[str->_len] = '\0';
        } else {
            str->_static_buf[str->_small_len] = c;
            ++str->_small_len;
            str->_static_buf[str->_small_len] = '\0';
        }
    } else {
        if (str->_len == str->_cap - 1) {
            mycc_grow_alloc((void**)&str->_data,
                            &str->_cap,
                            sizeof *str->_data);
        }
        str->_data[str->_len] = c;
        ++str->_len;
        str->_data[str->_len] = '\0';
    }
}

static void StrBuf_set_len(StrBuf* str, uint32_t len) {
    if (str->_is_static_buf) {
        assert(len < STR_BUF_STATIC_LEN);
        str->_small_len = (uint8_t)len;
    } else {
        str->_len = len;
    }
}

void StrBuf_pop_back(StrBuf* str) {
    assert(str);
    assert(StrBuf_valid(str));
    const uint32_t len = StrBuf_len(str);
    assert(len >= 1);
    char* data = StrBuf_get_mut_data(str);
    data[len - 1] = '\0';
    StrBuf_set_len(str, len - 1);
}

void StrBuf_shrink_to_fit(StrBuf* str) {
    assert(str);
    assert(StrBuf_valid(str));
    // The cast in this if is so gcc does not give a warning about signedness
    // (even though both _len and _cap should be unsigned)
    if (!str->_is_static_buf && (uint32_t)str->_len + 1 != str->_cap) {
        if (str->_len < STR_BUF_STATIC_LEN) {
            char* data = str->_data;
            const uint32_t len = str->_len;
            str->_is_static_buf = true;
            str->_small_len = (uint8_t)len;
            memcpy(str->_static_buf, data, sizeof *data * (len + 1));
            mycc_free(data);
        } else {
            str->_cap = str->_len + 1;
            str->_data = mycc_realloc(str->_data,
                                      sizeof *str->_data * str->_cap);
        }
    }
}

void StrBuf_reserve(StrBuf* str, uint32_t new_cap) {
    if (str->_is_static_buf) {
        if (new_cap >= STR_BUF_STATIC_LEN) {
            StrBuf_move_to_dyn_buf(str, new_cap);
        }
    } else if (str->_cap < new_cap) {
        str->_cap = new_cap + 1;
        str->_data = mycc_realloc(str->_data, sizeof *str->_data * str->_cap);
    }
}

void StrBuf_remove_front(StrBuf* str, uint32_t num_chars) {
    char* data = StrBuf_get_mut_data(str);
    const uint32_t new_len = StrBuf_len(str) - num_chars;
    memmove(data, data + num_chars, new_len);
    data[new_len] = '\0';
    StrBuf_set_len(str, new_len);
}

void StrBuf_remove_back(StrBuf* str, uint32_t num_chars) {
    assert(StrBuf_len(str) >= num_chars);
    const uint32_t new_len = StrBuf_len(str) - num_chars;
    StrBuf_set_len(str, new_len);
    StrBuf_get_mut_data(str)[new_len] = '\0';
}

void StrBuf_append(StrBuf* str, Str app) {
    const uint32_t curr_len = StrBuf_len(str);
    const uint32_t new_len = curr_len + app.len;
    if (StrBuf_cap(str) < new_len) {
        StrBuf_reserve(str, new_len);
    }

    char* data = StrBuf_get_mut_data(str);
    memcpy(data + curr_len, app.data, sizeof *app.data * app.len);
    data[new_len] = '\0';
    StrBuf_set_len(str, new_len);
}

StrBuf StrBuf_concat(Str s1, Str s2) {
    const uint32_t len = s1.len + s2.len;
    StrBuf res = StrBuf_create_with_cap(s1, len);
    char* res_data = StrBuf_get_mut_data(&res);
    memcpy(res_data + s1.len, s2.data, s2.len * sizeof *res_data);
    res_data[len] = '\0';
    StrBuf_set_len(&res, len);
    return res;
}

void StrBuf_clear(StrBuf* str) {
    if (str->_is_static_buf) {
        str->_small_len = 0;
        str->_static_buf[0] = '\0';
    } else {
        str->_len = 0;
        str->_data[0] = '\0';
    }
}

void StrBuf_free(const StrBuf* str) {
    assert(str);
    if (!str->_is_static_buf) {
        mycc_free(str->_data);
    }
}

