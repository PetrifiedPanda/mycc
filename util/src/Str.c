#include "util/Str.h"

#include <string.h>
#include <assert.h>

Str Str_null(void) {
    return (Str){
        .len = 0,
        .data = NULL,
    };
}

bool Str_valid(Str s) {
    if (s.data == NULL) {
        assert(s.len == 0);
        return false;
    }
    return true;
}

Str Str_advance(Str s, size_t offset) {
    assert(s.len >= offset);
    return (Str){s.len - offset, s.data + offset};
}

Str Str_incr(Str s) {
    assert(s.len >= 1);
    return (Str){s.len - 1, s.data + 1};
}

Str Str_substr(Str s, size_t begin, size_t end) {
    assert(end <= s.len);
    assert(begin <= s.len);
    assert(begin <= end);
    return (Str){
        .len = end - begin,
        .data = s.data + begin,
    };
}

char Str_at(Str s, size_t i) {
    assert(i < s.len);
    return s.data[i];
}

bool Str_eq(Str s1, Str s2) {
    if (s1.len != s2.len) {
        return false;
    }
    return memcmp(s1.data, s2.data, sizeof *s1.data * s1.len) == 0;
}

