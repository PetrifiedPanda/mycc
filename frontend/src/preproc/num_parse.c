#include "frontend/preproc/num_parse.h"

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include "util/annotations.h"

static bool must_be_float_const(const char* spell, size_t len);

struct int_type_attrs {
    unsigned short num_long;
    bool is_unsigned;
};

static struct int_type_attrs get_int_attrs(const char* suffix,
                                           size_t suffix_len,
                                           struct preproc_err* err);

static enum value_type get_value_type(struct int_type_attrs attrs);

struct value parse_num_constant(const char* spell,
                                size_t len,
                                struct preproc_err* err) {
    assert(err);
    assert(spell);
    assert(len > 0);

    if (must_be_float_const(spell, len)) {
        const char* end = spell; // so end is set
        assert(errno == 0);
        long double val = strtold(spell, (char**)&end);
        if (errno != 0) {
            // TODO: error
            errno = 0;
            return (struct value){0};
        }
        enum value_type t = VALUE_DOUBLE;
        assert(spell <= end);
        if ((size_t)(end - spell) < len) {
            if (*end == 'f' || *end == 'F') {
                t = VALUE_FLOAT;
            } else if (*end == 'l' || *end == 'L') {
                t = VALUE_LDOUBLE;
            }
            ++end;
            if ((size_t)(end - spell) < len) {
                // TODO: error (suffix too long)
                return (struct value){0};
            }
        }

        return create_float_value(t, val);
    } else {
        const char* suffix = spell;
        assert(errno == 0);
        uintmax_t val = strtoull(spell, (char**)&suffix, 0);
        if (errno != 0) {
            // TODO: error
            errno = 0;
            return (struct value){0};
        }

        assert(spell <= suffix);

        const size_t suffix_len = len - (suffix - spell);
        if (suffix_len > 3) {
            // TODO: error suffix too long
            return (struct value){0};
        }
        struct int_type_attrs attrs = get_int_attrs(suffix, suffix_len, err);
        if (err->type != PREPROC_ERR_NONE) {
            return (struct value){0};
        }
        enum value_type type = get_value_type(attrs);
        // TODO: first integer type in which value can fit!!!
        return create_int_value(type, val);
    }
}

static bool must_be_float_const(const char* spell, size_t len) {
    const bool is_hex = len > 2 && spell[0] == '0' && tolower(spell[1]) == 'x';
    for (size_t i = 0; i < len; ++i) {
        if (is_hex) {
            switch (spell[i]) {
                case '.':
                case 'p':
                case 'P':
                    return true;
            }
        } else {
            switch (spell[i]) {
                case '.':
                case 'e':
                case 'E':
                case 'f':
                case 'F':
                    return true;
            }
        }
    }
    return false;
}

static struct int_type_attrs get_int_attrs(const char* suffix,
                                           size_t suffix_len,
                                           struct preproc_err* err) {
    assert(suffix_len <= 3);
    (void)err; // TODO: remove
    struct int_type_attrs res = {
        .num_long = 0,
        .is_unsigned = false,
    };
    bool l_is_upper = false;
    bool last_was_u = false;
    for (size_t i = 0; i < suffix_len; ++i) {
        switch (suffix[i]) {
            case 'l':
                if (res.num_long > 0 && l_is_upper) {
                    // TODO: err case mixing
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    // TODO: err
                    return (struct int_type_attrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                }
                break;
            case 'L':
                if (res.num_long > 0 && !l_is_upper) {
                    // TODO: err case mixing
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    // TODO: err
                    return (struct int_type_attrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                    l_is_upper = true;
                }
                break;
            case 'u':
            case 'U':
                if (res.is_unsigned) {
                    // TODO: err double u
                    return (struct int_type_attrs){0};
                } else {
                    res.is_unsigned = true;
                    last_was_u = true;
                }
                break;
            default:
                // TODO: err
                return (struct int_type_attrs){0};
        }
    }
    return res;
}

static enum value_type get_value_type(struct int_type_attrs attrs) {
    assert(attrs.num_long <= 2);
    if (attrs.is_unsigned) {
        switch (attrs.num_long) {
            case 0:
                return VALUE_UINT;
            case 1:
                return VALUE_ULINT;
            case 2:
                return VALUE_ULLINT;
            default:
                UNREACHABLE();
        }
    } else {
        switch (attrs.num_long) {
            case 0:
                return VALUE_INT;
            case 1:
                return VALUE_LINT;
            case 2:
                return VALUE_LLINT;
            default:
                UNREACHABLE();
        }
    }
}

