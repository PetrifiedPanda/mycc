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
                                           struct num_constant_err* err);

static enum value_type get_value_type_dec(struct int_type_attrs attrs,
                                          uintmax_t val,
                                          const struct arch_int_info* info,
                                          struct num_constant_err* err);

static enum value_type get_value_type_other(struct int_type_attrs attrs,
                                            uintmax_t val,
                                            const struct arch_int_info* info,
                                            struct num_constant_err* err);

struct parse_num_constant_res parse_num_constant(
    const char* spell,
    size_t len,
    const struct arch_int_info* int_info) {
    assert(spell);
    assert(len > 0);
    assert(isdigit(spell[0]) || spell[0] == '.');

    if (must_be_float_const(spell, len)) {
        const char* end = spell; // so end is set
        assert(errno == 0);
        long double val = strtold(spell, (char**)&end);
        if (errno != 0) {
            assert(errno == ERANGE);
            errno = 0;
            return (struct parse_num_constant_res){
                .err =
                    {
                        .type = NUM_CONSTANT_ERR_TOO_LARGE,
                        .is_int_lit = false,
                    },
            };
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
                return (struct parse_num_constant_res){
                    .err =
                        {
                            .type = NUM_CONSTANT_ERR_SUFFIX_TOO_LONG,
                            .is_int_lit = false,
                        },
                };
            }
        }

        return (struct parse_num_constant_res){
            .err.type = NUM_CONSTANT_ERR_NONE,
            .res = create_float_value(t, val),
        };
    } else {
        const enum {
            DEC = 10,
            HEX = 16,
            OCT = 8
        } base = spell[0] == '0'
                     ? ((len > 1 && tolower(spell[1]) == 'x') ? HEX : OCT)
                     : DEC;
        const char* suffix = spell;
        assert(errno == 0);
        uintmax_t val = strtoull(spell, (char**)&suffix, base);
        if (errno != 0) {
            assert(errno == ERANGE);
            errno = 0;
            return (struct parse_num_constant_res){
                .err =
                    {
                        .type = NUM_CONSTANT_ERR_TOO_LARGE,
                        .is_int_lit = true,
                    },
            };
        }

        assert(spell <= suffix);

        const size_t suffix_len = len - (suffix - spell);
        if (suffix_len > 3) {
            return (struct parse_num_constant_res){
                .err =
                    {
                        .type = NUM_CONSTANT_ERR_SUFFIX_TOO_LONG,
                        .is_int_lit = true,
                    },
            };
        }

        struct num_constant_err err = {
            .type = NUM_CONSTANT_ERR_NONE,
        };
        struct int_type_attrs attrs = get_int_attrs(suffix, suffix_len, &err);
        if (err.type != NUM_CONSTANT_ERR_NONE) {
            return (struct parse_num_constant_res){
                .err = err,
            };
        }
        const enum value_type
            type = base == DEC
                       ? get_value_type_dec(attrs, val, int_info, &err)
                       : get_value_type_other(attrs, val, int_info, &err);
        if (err.type != NUM_CONSTANT_ERR_NONE) {
            return (struct parse_num_constant_res){
                .err = err,
            };
        }
        return (struct parse_num_constant_res){
            .err.type = NUM_CONSTANT_ERR_NONE,
            .res = create_int_value(type, val)};
    }
}

void print_num_constant_err(FILE* out, const struct num_constant_err* err) {
    assert(err->type != NUM_CONSTANT_ERR_NONE);

    switch (err->type) {
        case NUM_CONSTANT_ERR_NONE:
            UNREACHABLE();
        case NUM_CONSTANT_ERR_SUFFIX_TOO_LONG:
            fprintf(out,
                    "%s literal suffix too long",
                    err->is_int_lit ? "integer" : "floating point");
            break;
        case NUM_CONSTANT_ERR_CASE_MIXING:
            fprintf(out, "L in interger suffix must be the same case");
            break;
        case NUM_CONSTANT_ERR_DOUBLE_U:
            fprintf(out, "u specifier can only appear once in integer suffix");
            break;
        case NUM_CONSTANT_ERR_U_BETWEEN_LS:
            fprintf(out,
                    "u specifier may not be between long specifiers in integer "
                    "suffix");
            break;
        case NUM_CONSTANT_ERR_INVALID_CHAR:
            fprintf(out, "invalid character in integer suffix");
            break;
        case NUM_CONSTANT_ERR_TOO_LARGE:
            fprintf(out,
                    "%s literal too large",
                    err->is_int_lit ? "integer" : "floating point");
            break;
    }
    fprintf(out, "\n");
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
                                           struct num_constant_err* err) {
    assert(suffix_len <= 3);
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
                    err->type = NUM_CONSTANT_ERR_CASE_MIXING;
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->type = NUM_CONSTANT_ERR_U_BETWEEN_LS;
                    return (struct int_type_attrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                }
                break;
            case 'L':
                if (res.num_long > 0 && !l_is_upper) {
                    err->type = NUM_CONSTANT_ERR_CASE_MIXING;
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->type = NUM_CONSTANT_ERR_U_BETWEEN_LS;
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
                    err->type = NUM_CONSTANT_ERR_DOUBLE_U;
                    return (struct int_type_attrs){0};
                } else {
                    res.is_unsigned = true;
                    last_was_u = true;
                }
                break;
            default:
                err->type = NUM_CONSTANT_ERR_INVALID_CHAR;
                return (struct int_type_attrs){0};
        }
    }
    return res;
}

enum {
    TARGET_CHAR_SIZE = 8,
    THIS_CHAR_SIZE = TARGET_CHAR_SIZE,
};

static uintmax_t int_pow2(uintmax_t exp) {
    if (exp < sizeof(uintmax_t) * THIS_CHAR_SIZE) {
        return 1ull << exp;
    }
    uintmax_t base = 2;
    uintmax_t res = 1;
    while (true) {
        if (exp & 1) {
            res *= base;
        }
        exp >>= 1;
        if (exp == 0) {
            break;
        }
        base *= base;
    }
    return res;
}

static uintmax_t max_uint(uintmax_t num_bits) {
    return int_pow2(num_bits) - 1;
}

static uintmax_t max_int(uintmax_t num_bits) {
    return int_pow2(num_bits - 1) - 1;
}

static uintmax_t get_max_int(const struct arch_int_info* info,
                             enum value_type type) {
    assert(type == VALUE_UINT || type == VALUE_ULINT || type == VALUE_ULLINT
           || type == VALUE_INT || type == VALUE_LINT || type == VALUE_LLINT);

    switch (type) {
        case VALUE_INT:
            return max_int(TARGET_CHAR_SIZE * info->int_size);
        case VALUE_UINT:
            return max_uint(TARGET_CHAR_SIZE * info->int_size);
        case VALUE_LINT:
            return max_int(TARGET_CHAR_SIZE * info->lint_size);
        case VALUE_ULINT:
            return max_uint(TARGET_CHAR_SIZE * info->lint_size);
        case VALUE_LLINT:
            return max_int(TARGET_CHAR_SIZE * info->llint_size);
        case VALUE_ULLINT:
            return max_uint(TARGET_CHAR_SIZE * info->llint_size);

        default:
            UNREACHABLE();
    }
}

static enum value_type get_value_type_unsigned(
    struct int_type_attrs attrs,
    uintmax_t val,
    const struct arch_int_info* info) {
    assert(attrs.is_unsigned);
    assert(attrs.num_long <= 2);

    switch (attrs.num_long) {
        case 0:
            if (val <= get_max_int(info, VALUE_UINT)) {
                return VALUE_UINT;
            }
            FALLTHROUGH();
        case 1:
            if (val <= get_max_int(info, VALUE_ULINT)) {
                return VALUE_ULINT;
            }
            FALLTHROUGH();
        case 2:
            if (val <= get_max_int(info, VALUE_ULLINT)) {
                return VALUE_ULLINT;
            } else {
                // unsigned will throw error in strotull
                UNREACHABLE();
            }
        default:
            UNREACHABLE();
    }
}

static enum value_type get_value_type_dec(struct int_type_attrs attrs,
                                          uintmax_t val,
                                          const struct arch_int_info* info,
                                          struct num_constant_err* err) {
    assert(attrs.num_long <= 2);
    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(info, VALUE_INT)) {
                    return VALUE_INT;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(info, VALUE_LINT)) {
                    return VALUE_LINT;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(info, VALUE_LLINT)) {
                    return VALUE_LLINT;
                } else {
                    err->type = NUM_CONSTANT_ERR_TOO_LARGE;
                    return VALUE_UINT;
                }
            default:
                UNREACHABLE();
        }
    }
}

static enum value_type get_value_type_other(struct int_type_attrs attrs,
                                            uintmax_t val,
                                            const struct arch_int_info* info,
                                            struct num_constant_err* err) {
    assert(attrs.num_long <= 2);

    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(info, VALUE_INT)) {
                    return VALUE_INT;
                } else if (val <= get_max_int(info, VALUE_UINT)) {
                    return VALUE_UINT;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(info, VALUE_LINT)) {
                    return VALUE_LINT;
                } else if (val <= get_max_int(info, VALUE_ULINT)) {
                    return VALUE_ULINT;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(info, VALUE_LLINT)) {
                    return VALUE_LLINT;
                } else if (val <= get_max_int(info, VALUE_ULLINT)) {
                    return VALUE_ULLINT;
                } else {
                    err->type = NUM_CONSTANT_ERR_TOO_LARGE;
                    return VALUE_INT;
                }
            default:
                UNREACHABLE();
        }
    }
}

