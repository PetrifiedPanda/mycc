#include "util/IndexedStringSet.h"

#include <string.h>
#include <assert.h>

#include "util/macro_util.h"
#include "util/mem.h"

IndexedStringSet IndexedStringSet_create(uint32_t init_cap) {
    IndexedStringSet res = {
        ._len = 0,
        ._cap = init_cap,
        ._indices = mycc_alloc(sizeof *res._indices * init_cap),
        ._data = mycc_alloc(sizeof *res._data * init_cap),
    };
    memset(res._indices, 0xff, sizeof *res._indices * init_cap);
    return res;
}

void IndexedStringSet_free(const IndexedStringSet* s) {
    mycc_free(s->_indices);
    for (uint32_t i = 0; i < s->_len; ++i) {
        StrBuf_free(&s->_data[i]);
    }
    mycc_free(s->_data);
}

static uint32_t insert(IndexedStringSet* s, const StrBuf* buf, uint32_t idx_idx) {
    assert(s->_len < s->_cap);

    const uint32_t res_idx = s->_len;
    s->_indices[idx_idx] = res_idx;
    s->_data[res_idx] = *buf;
    ++s->_len;
    return res_idx;
}

static uint32_t hash_string(Str str);

// Assumes key is not already in set
static uint32_t insert_str_buf(IndexedStringSet* s, const StrBuf* buf) {
    const uint32_t hash = hash_string(StrBuf_as_str(buf));
    uint32_t i = hash % s->_cap;
    while (true) {
        const uint32_t str_idx = s->_indices[i];
        if (str_idx == UINT32_MAX) {
            return insert(s, buf, i);
        } else {
            i = (i + 1) % s->_cap;
        }
    }
    UNREACHABLE();
}

static void rehash(IndexedStringSet* s) {
    const uint32_t prev_len = s->_len;
    StrBuf* old_data = s->_data;
    s->_len = 0;
    s->_cap += s->_cap / 2 + 1;
    s->_indices = mycc_realloc(s->_indices, sizeof *s->_indices * s->_cap);
    memset(s->_indices, 0xff, sizeof *s->_indices * s->_cap);
    s->_data = mycc_alloc(sizeof *s->_data * s->_cap);
    for (uint32_t i = 0; i < prev_len; ++i) {
        insert_str_buf(s, &old_data[i]);
    }
    assert(s->_len == prev_len);
    mycc_free(old_data);
}

static void rehash_if_necessary(IndexedStringSet* s) {
    if (s->_len == s->_cap - 1) {
        rehash(s);
    }
}

uint32_t IndexedStringSet_find_or_insert(IndexedStringSet* s, Str str) {
    rehash_if_necessary(s);
    const uint32_t hash = hash_string(str);
    uint32_t i = hash % s->_cap;
    while (true) {
        const uint32_t str_idx = s->_indices[i];
        if (str_idx == UINT32_MAX) {
            StrBuf buf = StrBuf_create(str);
            return insert(s, &buf, i);
        } else if (Str_eq(str, StrBuf_as_str(&s->_data[str_idx]))) {
            return str_idx;
        } else {
            i = (i + 1) % s->_cap;
        }
    }
    UNREACHABLE(); 
}

Str IndexedStringSet_get(const IndexedStringSet* s, uint32_t idx) {
    assert(idx < s->_len);
    return StrBuf_as_str(&s->_data[idx]);
}

// Hash function taken from K&R version 2 (page 144)
static uint32_t hash_string(Str str) {
    uint32_t hash = 0;

    uint32_t i = 0;
    while (i != str.len) {
        hash = Str_at(str, i) + 32 * hash;
        ++i;
    }
    return hash;
}
