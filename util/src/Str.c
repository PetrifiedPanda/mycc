#include "util/Str.h"

#include <string.h>
#include <assert.h>

Str Str_null(void) {
    return (Str){
        .len = 0,
        .data = NULL,
    };
}

CStr Str_c_str(Str s) {
    assert(s.data[s.len] == '\0');
    return (CStr){
        .len = s.len,
        .data = s.data,
    };
}

Str CStr_as_str(CStr s) {
    return (Str){
        .len = s.len,
        .data = s.data,
    };
}

bool Str_valid(Str s) {
    if (s.data == NULL) {
        assert(s.len == 0);
        return false;
    }
    return true;
}

Str Str_advance(Str s, uint32_t offset) {
    assert(s.len >= offset);
    return (Str){s.len - offset, s.data + offset};
}

Str Str_incr(Str s) {
    assert(s.len >= 1);
    return (Str){s.len - 1, s.data + 1};
}

Str Str_substr(Str s, uint32_t begin, uint32_t end) {
    assert(end <= s.len);
    assert(begin <= s.len);
    assert(begin <= end);
    return (Str){
        .len = end - begin,
        .data = s.data + begin,
    };
}

char Str_at(Str s, uint32_t i) {
    assert(i < s.len);
    return s.data[i];
}

bool Str_starts_with(Str s1, Str s2) {
    if (s1.len < s2.len) {
        return false;
    }
    return Str_eq(Str_substr(s1, 0, s2.len), s2);
}

bool Str_eq(Str s1, Str s2) {
    if (s1.len != s2.len) {
        return false;
    }
    return memcmp(s1.data, s2.data, sizeof *s1.data * s1.len) == 0;
}

