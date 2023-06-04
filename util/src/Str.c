#include "util/Str.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"

Str Str_create_null(void) {
    return (Str){
        ._is_static_buf = false,
        ._len = 0,
        ._cap = 0,
        ._data = NULL,
    };
}

Str Str_create_empty(void) {
    Str res = {
        ._is_static_buf_dup = true,
        ._small_len = 0,
        ._static_buf = {0},
    };
    return res;
}

enum {
    STATIC_BUF_LEN = sizeof(Str){{{0}}}._static_buf
                     / sizeof *(Str){{{0}}}._static_buf
};

static Str Str_create_with_cap(size_t len, const char* str, size_t cap) {
    assert(cap >= len);
    assert(len == 0 || str);
    Str res;
    if (cap < STATIC_BUF_LEN) {
        res._is_static_buf = true;
        res._small_len = (uint8_t)len;
        memcpy(res._static_buf, str, sizeof *res._static_buf * len);
        res._static_buf[len] = '\0';
    } else {
        res._is_static_buf = false;
        res._len = len;
        res._cap = cap + 1;
        res._data = mycc_alloc(sizeof *res._data * res._cap);
        memcpy(res._data, str, sizeof *res._static_buf * len);
        res._data[len] = '\0';
    }
    return res;
}

Str Str_create(size_t len, const char* str) {
    assert(len == 0 || str);
    return Str_create_with_cap(len, str, len);
}

Str Str_create_empty_with_cap(size_t cap) {
    Str res;
    if (cap < STATIC_BUF_LEN) {
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

bool Str_is_valid(const Str* str) {
    assert(str);
    return str->_is_static_buf || str->_data != NULL;
}

size_t Str_len(const Str* str) {
    assert(Str_is_valid(str));
    if (str->_is_static_buf) {
        return str->_small_len;
    } else {
        return str->_len;
    }
}

size_t Str_cap(const Str* str) {
    assert(Str_is_valid(str));
    if (str->_is_static_buf) {
        return STATIC_BUF_LEN - 1;
    } else {
        return str->_cap - 1;
    }
}

static char* Str_get_mut_data(Str* str) {
    assert(str);
    assert(Str_is_valid(str));
    if (str->_is_static_buf) {
        return str->_static_buf;
    } else {
        return str->_data;
    }
}

const char* Str_get_data(const Str* str) {
    assert(str);
    if (str->_is_static_buf) {
        return str->_static_buf;
    } else {
        return str->_data;
    }
}

char Str_char_at(const Str* str, size_t i) {
    assert(str);
    assert(Str_is_valid(str));
    if (str->_is_static_buf) {
        assert(i < str->_small_len);
        return str->_static_buf[i];
    } else {
        assert(i < str->_len);
        return str->_data[i];
    }
}

static void Str_move_to_dyn_buf(Str* str, size_t dyn_buf_cap) {
    assert(str->_is_static_buf);

    const size_t len = str->_small_len;
    const size_t real_cap = dyn_buf_cap + 1;
    char* data = mycc_alloc(sizeof *data * real_cap);
    memcpy(data, str->_static_buf, sizeof *data * (len + 1));
    str->_cap = real_cap;
    str->_len = len;
    str->_data = data;
    str->_is_static_buf = false;
}

void Str_push_back(Str* str, char c) {
    assert(str);
    assert(Str_is_valid(str));
    if (str->_is_static_buf) {
        if (str->_small_len == STATIC_BUF_LEN - 1) {
            Str_move_to_dyn_buf(str, STATIC_BUF_LEN);
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

static void Str_set_len(Str* str, size_t len) {
    if (str->_is_static_buf) {
        assert(len < STATIC_BUF_LEN);
        str->_small_len = (uint8_t)len;
    } else {
        str->_len = len;
    }
}

void Str_pop_back(Str* str) {
    assert(str);
    assert(Str_is_valid(str));
    const size_t len = Str_len(str);
    assert(len >= 1);
    char* data = Str_get_mut_data(str);
    data[len - 1] = '\0';
    Str_set_len(str, len - 1);
}

void Str_shrink_to_fit(Str* str) {
    assert(str);
    assert(Str_is_valid(str));
    if (!str->_is_static_buf && str->_len + 1 != str->_cap) {
        if (str->_len < STATIC_BUF_LEN) {
            char* data = str->_data;
            const size_t len = str->_len;
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

void Str_reserve(Str* str, size_t new_cap) {
    if (str->_is_static_buf) {
        if (new_cap >= STATIC_BUF_LEN) {
            Str_move_to_dyn_buf(str, new_cap);
        }
    } else if (str->_cap < new_cap) {
        str->_cap = new_cap + 1;
        str->_data = mycc_realloc(str->_data, sizeof *str->_data * str->_cap);
    }
}

void Str_remove_front(Str* str, size_t num_chars) {
    char* data = Str_get_mut_data(str);
    const size_t new_len = Str_len(str) - num_chars;
    memmove(data, data + num_chars, new_len);
    data[new_len] = '\0';
    Str_set_len(str, new_len);
}

void Str_append_c_str(Str* str, size_t len, const char* c_str) {
    const size_t curr_len = Str_len(str);
    const size_t new_len = curr_len + len;
    if (Str_cap(str) < new_len) {
        Str_reserve(str, new_len);
    }

    char* data = Str_get_mut_data(str);
    memcpy(data + curr_len, c_str, sizeof *c_str * len);
    data[new_len] = '\0';
    Str_set_len(str, new_len);
}

Str Str_concat(size_t len1,
                      const char* s1,
                      size_t len2,
                      const char* s2) {
    const size_t len = len1 + len2;
    Str res = Str_create_with_cap(len1, s1, len);
    char* res_data = Str_get_mut_data(&res);
    memcpy(res_data + len1, s2, len2 * sizeof *res_data);
    res_data[len] = '\0';
    Str_set_len(&res, len);
    return res;
}

Str Str_take(Str* str) {
    assert(str);
    assert(Str_is_valid(str));
    Str res = *str;
    str->_is_static_buf = false;
    str->_len = 0;
    str->_cap = 0;
    str->_data = NULL;
    return res;
}

Str Str_copy(const Str* str) {
    assert(str);
    if (str->_is_static_buf) {
        return *str;
    } else {
        if (Str_is_valid(str)) {
            return Str_create_with_cap(str->_len, str->_data, str->_cap - 1);
        } else {
            return Str_create_null();
        }
    }
}

void Str_clear(Str* str) {
    if (str->_is_static_buf) {
        str->_small_len = 0;
        str->_static_buf[0] = '\0';
    } else {
        str->_len = 0;
        str->_data[0] = '\0';
    }
}

bool Str_eq(const Str* s1, const Str* s2) {
    assert(s1);
    assert(s2);
    assert(Str_is_valid(s1));
    assert(Str_is_valid(s2));

    const size_t l1 = Str_len(s1);
    if (l1 != Str_len(s2)) {
        return false;
    }
    const char* d1 = Str_get_data(s1);
    return memcmp(d1, Str_get_data(s2), sizeof *d1 * l1) == 0;
}

void Str_free(const Str* str) {
    assert(str);
    if (!str->_is_static_buf) {
        mycc_free(str->_data);
    }
}

