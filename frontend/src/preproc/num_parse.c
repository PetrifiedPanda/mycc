#include "frontend/preproc/num_parse.h"

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include "util/annotations.h"

struct parse_float_const_res parse_float_const(const char* spell) {
    const char* end = spell; // so end is set
    assert(errno == 0);
    long double val = strtold(spell, (char**)&end);
    if (errno != 0) {
        assert(errno == ERANGE);
        errno = 0;
        return (struct parse_float_const_res){
            .err.type = FLOAT_CONST_ERR_TOO_LARGE,
        };
    }
    enum value_type t = VALUE_DOUBLE;
    assert(spell <= end);
    if (*end != '\0') {
        if (*end == 'f' || *end == 'F') {
            t = VALUE_FLOAT;
        } else if (*end == 'l' || *end == 'L') {
            t = VALUE_LDOUBLE;
        } else {
            return (struct parse_float_const_res){
                .err =
                    {
                        .type = FLOAT_CONST_ERR_INVALID_CHAR,
                        .invalid_char = *end,
                    },
            };
        }
        ++end;
        if (*end != '\0') {
            return (struct parse_float_const_res){
                .err.type = FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
            };
        }
    }

    return (struct parse_float_const_res){
        .err.type = FLOAT_CONST_ERR_NONE,
        .res = create_float_value(t, val),
    };
}

void print_float_const_err(FILE* out, const struct float_const_err* err) {
    assert(err != FLOAT_CONST_ERR_NONE);

    switch (err->type) {
        case FLOAT_CONST_ERR_NONE:
            UNREACHABLE();
        case FLOAT_CONST_ERR_TOO_LARGE:
            fprintf(out, "floating constant too large to be represented");
            break;
        case FLOAT_CONST_ERR_SUFFIX_TOO_LONG:
            fprintf(out,
                    "floating constant suffix too long. Only one character is "
                    "allowed in the suffix");
            break;
        case FLOAT_CONST_ERR_INVALID_CHAR:
            fprintf(out, "invalid character %c in suffix", err->invalid_char);
            break;
    }
    fprintf(out, "\n");
}

struct int_type_attrs {
    unsigned short num_long;
    bool is_unsigned;
};

static struct int_type_attrs get_int_attrs(const char* suffix,
                                           struct int_const_err* err);

static enum value_type get_value_type_dec(struct int_type_attrs attrs,
                                          uintmax_t val,
                                          const struct arch_int_info* info,
                                          struct int_const_err* err);

static enum value_type get_value_type_other(struct int_type_attrs attrs,
                                            uintmax_t val,
                                            const struct arch_int_info* info,
                                            struct int_const_err* err);

struct parse_int_const_res parse_int_const(const char* spell,
                                           const struct arch_int_info* info) {
    const enum {
        DEC = 10,
        HEX = 16,
        OCT = 8
    } base = spell[0] == '0' ? ((tolower(spell[1]) == 'x') ? HEX : OCT) : DEC;
    const char* suffix = spell;
    assert(errno == 0);
    uintmax_t val = strtoull(spell, (char**)&suffix, base);
    if (errno != 0) {
        assert(errno == ERANGE);
        errno = 0;
        return (struct parse_int_const_res){
            .err.type = INT_CONST_ERR_TOO_LARGE,
        };
    }

    assert(spell <= suffix);

    struct int_const_err err = {
        .type = INT_CONST_ERR_NONE,
    };
    struct int_type_attrs attrs = get_int_attrs(suffix, &err);
    if (err.type != INT_CONST_ERR_NONE) {
        return (struct parse_int_const_res){
            .err = err,
        };
    }
    const enum value_type
        type = base == DEC ? get_value_type_dec(attrs, val, info, &err)
                           : get_value_type_other(attrs, val, info, &err);
    if (err.type != INT_CONST_ERR_NONE) {
        return (struct parse_int_const_res){
            .err = err,
        };
    }
    assert(value_is_int(type) || value_is_uint(type));
    return (struct parse_int_const_res){
        .err.type = INT_CONST_ERR_NONE,
        .res = value_is_int(type) ? create_int_value(type, (intmax_t)val)
                                  : create_uint_value(type, val),
    };
}

void print_int_const_err(FILE* out, const struct int_const_err* err) {
    assert(out);
    assert(err);
    assert(err->type != INT_CONST_ERR_NONE);

    switch (err->type) {
        case INT_CONST_ERR_NONE:
            UNREACHABLE();
        case INT_CONST_ERR_TOO_LARGE:
            fprintf(out, "integer literal too large to be represented");
            break;
        case INT_CONST_ERR_SUFFIX_TOO_LONG:
            fprintf(out,
                    "integer literal suffix too long. The suffix may be a "
                    "maximum of 3 characters");
            break;
        case INT_CONST_ERR_CASE_MIXING:
            fprintf(out, "ls in suffix must be the same case");
            break;
        case INT_CONST_ERR_U_BETWEEN_LS:
            fprintf(out, "u must not be between two ls in suffix");
            break;
        case INT_CONST_ERR_TRIPLE_LONG:
            fprintf(out, "l may only appear twice in suffix");
            break;
        case INT_CONST_ERR_DOUBLE_U:
            fprintf(out, "u may only appear once in suffix");
            break;
        case INT_CONST_ERR_INVALID_CHAR:
            fprintf(out,
                    "invalid character %c in integer literal",
                    err->invalid_char);
            break;
    }
    fprintf(out, "\n");
}

static struct int_type_attrs get_int_attrs(const char* suffix,
                                           struct int_const_err* err) {
    struct int_type_attrs res = {
        .num_long = 0,
        .is_unsigned = false,
    };
    bool l_is_upper = false;
    bool last_was_u = false;
    for (size_t i = 0; suffix[i] != '\0'; ++i) {
        if (i == 3) {
            err->type = INT_CONST_ERR_SUFFIX_TOO_LONG;
            return (struct int_type_attrs){0};
        }
        switch (suffix[i]) {
            case 'l':
                if (res.num_long > 0 && l_is_upper) {
                    err->type = INT_CONST_ERR_CASE_MIXING;
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->type = INT_CONST_ERR_U_BETWEEN_LS;
                    return (struct int_type_attrs){0};
                } else if (res.num_long == 2) {
                    err->type = INT_CONST_ERR_TRIPLE_LONG;
                    return (struct int_type_attrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                }
                break;
            case 'L':
                if (res.num_long > 0 && !l_is_upper) {
                    err->type = INT_CONST_ERR_CASE_MIXING;
                    return (struct int_type_attrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->type = INT_CONST_ERR_U_BETWEEN_LS;
                    return (struct int_type_attrs){0};
                } else if (res.num_long == 2) {
                    err->type = INT_CONST_ERR_TRIPLE_LONG;
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
                    err->type = INT_CONST_ERR_DOUBLE_U;
                    return (struct int_type_attrs){0};
                } else {
                    res.is_unsigned = true;
                    last_was_u = true;
                }
                break;
            default:
                err->type = INT_CONST_ERR_INVALID_CHAR;
                err->invalid_char = suffix[i];
                return (struct int_type_attrs){0};
        }
    }
    return res;
}

enum {
    TARGET_CHAR_SIZE = CHAR_BIT,
};

static uintmax_t int_pow2(uintmax_t exp) {
    if (exp < sizeof(uintmax_t) * CHAR_BIT) {
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
                                          struct int_const_err* err) {
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
                    err->type = INT_CONST_ERR_TOO_LARGE;
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
                                            struct int_const_err* err) {
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
                    err->type = INT_CONST_ERR_TOO_LARGE;
                    return VALUE_INT;
                }
            default:
                UNREACHABLE();
        }
    }
}

static enum value_type get_uint_leastn_t_type(
    size_t n,
    const struct arch_int_info* info); 

struct parse_char_const_res parse_char_const(const char* spell,
                                             const struct arch_int_info* info) {
    assert(spell);
    assert(info);

    enum value_type type;
    switch (*spell) {
        case '\'':
            type = VALUE_INT;
            ++spell;
            break;
        case 'u':
            if (spell[1] == '8') {
                type = VALUE_UCHAR;
            } else if (spell[1] == '\'') {
                type = get_uint_leastn_t_type(16, info);
            } else {
                return (struct parse_char_const_res){
                    .err =
                        {
                            .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 2,
                            .expected_chars = {'8', '\''},
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        case 'U':
            type = get_uint_leastn_t_type(32, info);
            if (spell[1] != '\'') {
                return (struct parse_char_const_res){
                    .err =
                        {
                            .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        case 'L':
            // TODO: handle wchar_t stuff
            type = get_uint_leastn_t_type(32, info);
            if (spell[1] != '\'') {
                return (struct parse_char_const_res){
                    .err =
                        {
                            .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        default:
            return (struct parse_char_const_res){
                .err =
                    {
                        .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 4,
                        .expected_chars = {'\'', 'u', 'U', 'L'},
                        .got_char = *spell,
                    },
            };
    }

    if (value_is_uint(type)) {
        uintmax_t val;
        switch (*spell) {
            case '\\':
                ++spell;
                switch (*spell) {
                    case 'a':
                        val = '\a';
                        break;
                    case 'b':
                        val = '\b';
                        break;
                    case 'f':
                        val = '\f';
                        break;
                    case 'n':
                        val = '\n';
                        break;
                    case 'r':
                        val = '\r';
                        break;
                    case 't':
                        val = '\t';
                        break;
                    case 'v':
                        val = '\v';
                        break;
                    case '\\':
                    case '\'':
                    case '\"':
                    case '\?':
                        val = *spell;
                        break;
                    case '0': // TODO: remove hardcoded
                        val = '\0';
                        break;
                    default:
                        // TODO: other escape stuff
                        return (struct parse_char_const_res) {
                            .err = {
                                .type = CHAR_CONST_ERR_INVALID_ESCAPE,
                                .invalid_escape = *spell,
                            },
                        };
                }
                break;
            default:
                val = *spell;
                break;
        }

        ++spell;
        if (*spell != '\'') {
            return (struct parse_char_const_res) {
                .err = {
                    .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                    .num_expected = 1,
                    .expected_chars[0] = '\'',
                    .got_char = *spell,
                },
            };
        }
        assert(spell[1] == '\0');

        return (struct parse_char_const_res){
            .err.type = CHAR_CONST_ERR_NONE,
            .res = create_uint_value(type, val),
        };
    } else {
        assert(value_is_int(type));
        intmax_t val;
        switch (*spell) {
            case '\\':
                ++spell;
                switch (*spell) {
                    case 'a':
                        val = '\a';
                        break;
                    case 'b':
                        val = '\b';
                        break;
                    case 'f':
                        val = '\f';
                        break;
                    case 'n':
                        val = '\n';
                        break;
                    case 'r':
                        val = '\r';
                        break;
                       case 't':
                        val = '\t';
                        break;
                    case 'v':
                        val = '\v';
                        break;
                    case '\\':
                    case '\'':
                    case '\"':
                    case '\?':
                        val = *spell;
                        break;
                    case '0': // TODO: remove hardcoded
                        val = '\0';
                        break;
                    default:
                        // TODO: other escape stuff
                        return (struct parse_char_const_res) {
                            .err = {
                                .type = CHAR_CONST_ERR_INVALID_ESCAPE,
                                .invalid_escape = *spell,
                            },
                        };
                }
                break;
            default:
                val = *spell;
                break;
        }

        ++spell;
        if (*spell != '\'') {
            return (struct parse_char_const_res) {
                .err = {
                    .type = CHAR_CONST_ERR_EXPECTED_CHAR,
                    .num_expected = 1,
                    .expected_chars[0] = '\'',
                    .got_char = *spell,
                },
            };
        }
        assert(spell[1] == '\0');

        return (struct parse_char_const_res){
            .err.type = CHAR_CONST_ERR_NONE,
            .res = create_int_value(type, val),
        };
    }
}

void print_char_const_err(FILE* out, const struct char_const_err* err) {
    assert(err->type != CHAR_CONST_ERR_NONE);
    switch (err->type) {
        case CHAR_CONST_ERR_NONE:
            UNREACHABLE();
        case CHAR_CONST_ERR_EXPECTED_CHAR:
            fprintf(out, "Expected ");
            const uint8_t limit = err->num_expected - 1;
            for (uint8_t i = 0; i < limit; ++i) {
                fprintf(out, "%c, ", err->expected_chars[i]);
            }
            fprintf(out, " or %c but got %c", err->expected_chars[limit], err->got_char);
            break;
        case CHAR_CONST_ERR_INVALID_ESCAPE:
            fprintf(out, "Invalid escape character %c", err->invalid_escape);
            break;
    }
    fprintf(out, "\n");
}

static enum value_type get_uint_leastn_t_type(
    size_t n,
    const struct arch_int_info* info) {
    assert(n == 8 || n == 16 || n == 32 || n == 64);

    if (TARGET_CHAR_SIZE >= n) {
        return VALUE_UCHAR;
    } else if (TARGET_CHAR_SIZE * info->sint_size >= n) {
        return VALUE_USINT;
    } else if (TARGET_CHAR_SIZE * info->int_size >= n) {
        return VALUE_UINT;
    } else if (TARGET_CHAR_SIZE * info->lint_size >= n) {
        return VALUE_ULINT;
    } else if (TARGET_CHAR_SIZE * info->llint_size >= n) {
        return VALUE_ULLINT;
    }
    UNREACHABLE();
}

