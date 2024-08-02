#include "frontend/preproc/num_parse.h"

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include "util/macro_util.h"

ParseFloatConstRes parse_float_const(Str spell) {
    const char* suffix = spell.data; // so end is set
    const char* end = spell.data + spell.len;
    assert(errno == 0);
    double val = strtod(spell.data, (char**)&suffix);
    if (errno != 0) {
        assert(errno == ERANGE);
        errno = 0;
        return (ParseFloatConstRes){
            .err.kind = FLOAT_CONST_ERR_TOO_LARGE,
        };
    }
    FloatValKind t = FLOAT_VAL_DOUBLE;
    assert(spell.data <= suffix);
    if (suffix != end) {
        if (*suffix == 'f' || *suffix == 'F') {
            t = FLOAT_VAL_FLOAT;
        } else if (*suffix == 'l' || *suffix == 'L') {
            t = FLOAT_VAL_LDOUBLE;
        } else {
            return (ParseFloatConstRes){
                .err =
                    {
                        .kind = FLOAT_CONST_ERR_INVALID_CHAR,
                        .invalid_char = *suffix,
                    },
            };
        }
        ++suffix;
        if (suffix != end) {
            return (ParseFloatConstRes){
                .err.kind = FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
            };
        }
    }

    return (ParseFloatConstRes){
        .err.kind = FLOAT_CONST_ERR_NONE,
        .res = FloatVal_create(t, val),
    };
}

void FloatConstErr_print(File out, const FloatConstErr* err) {
    assert(err != FLOAT_CONST_ERR_NONE);

    switch (err->kind) {
        case FLOAT_CONST_ERR_NONE:
            UNREACHABLE();
        case FLOAT_CONST_ERR_TOO_LARGE:
            File_put_str("floating constant too large to be represented", out);
            break;
        case FLOAT_CONST_ERR_SUFFIX_TOO_LONG:
            File_put_str(
                "floating constant suffix too long. Only one character is "
                "allowed in the suffix",
                out);
            break;
        case FLOAT_CONST_ERR_INVALID_CHAR:
            File_printf(out,
                        "invalid character {char} in suffix",
                        err->invalid_char);
            break;
    }
    File_putc('\n', out);
}

typedef struct {
    unsigned short num_long;
    bool is_unsigned;
} IntTypeAttrs;

static IntTypeAttrs get_int_attrs(Str suffix, IntConstErr* err);

static IntValKind get_value_type_dec(IntTypeAttrs attrs,
                                     uintmax_t val,
                                     const ArchTypeInfo* type_info,
                                     IntConstErr* err);

static IntValKind get_value_type_other(IntTypeAttrs attrs,
                                       uintmax_t val,
                                       const ArchTypeInfo* type_info,
                                       IntConstErr* err);

ParseIntConstRes parse_int_const(Str spell, const ArchTypeInfo* type_info) {
    const enum {
        DEC = 10,
        HEX = 16,
        OCT = 8
    } base = Str_at(spell, 0) == '0'
                 ? ((spell.len > 1 && tolower(Str_at(spell, 1)) == 'x') ? HEX
                                                                        : OCT)
                 : DEC;
    const char* suffix = spell.data;
    assert(errno == 0);
    uintmax_t val = strtoull(spell.data, (char**)&suffix, base);
    if (errno != 0) {
        assert(errno == ERANGE);
        errno = 0;
        return (ParseIntConstRes){
            .err.kind = INT_CONST_ERR_TOO_LARGE,
        };
    }

    assert(spell.data <= suffix);

    IntConstErr err = {
        .kind = INT_CONST_ERR_NONE,
    };
    
    const size_t sz_len = suffix - spell.data;
    const uint32_t len = (uint32_t)sz_len;
    assert((size_t)len == sz_len);
    IntTypeAttrs attrs = get_int_attrs(Str_advance(spell, len),
                                       &err);
    if (err.kind != INT_CONST_ERR_NONE) {
        return (ParseIntConstRes){
            .err = err,
        };
    }
    const IntValKind kind = base == DEC
                               ? get_value_type_dec(attrs, val, type_info, &err)
                               : get_value_type_other(attrs,
                                                      val,
                                                      type_info,
                                                      &err);
    if (err.kind != INT_CONST_ERR_NONE) {
        return (ParseIntConstRes){
            .err = err,
        };
    }
    return (ParseIntConstRes){
        .err.kind = INT_CONST_ERR_NONE,
        .res = IntValKind_is_sint(kind) ? IntVal_create_sint(kind, (int64_t)val)
                                       : IntVal_create_uint(kind, val),
    };
}

void IntConstErr_print(File out, const IntConstErr* err) {
    assert(err);
    assert(err->kind != INT_CONST_ERR_NONE);

    switch (err->kind) {
        case INT_CONST_ERR_NONE:
            UNREACHABLE();
        case INT_CONST_ERR_TOO_LARGE:
            File_put_str("integer literal too large to be represented", out);
            break;
        case INT_CONST_ERR_SUFFIX_TOO_LONG:
            File_put_str("integer literal suffix too long. The suffix may be a "
                         "maximum of 3 characters",
                         out);
            break;
        case INT_CONST_ERR_CASE_MIXING:
            File_put_str("ls in suffix must be the same case", out);
            break;
        case INT_CONST_ERR_U_BETWEEN_LS:
            File_put_str("u must not be between two ls in suffix", out);
            break;
        case INT_CONST_ERR_TRIPLE_LONG:
            File_put_str("l may only appear twice in suffix", out);
            break;
        case INT_CONST_ERR_DOUBLE_U:
            File_put_str("u may only appear once in suffix", out);
            break;
        case INT_CONST_ERR_INVALID_CHAR:
            File_printf(out,
                        "invalid character {char} in integer literal",
                        err->invalid_char);
            break;
    }
    File_putc('\n', out);
}

static IntTypeAttrs get_int_attrs(Str suffix, IntConstErr* err) {
    IntTypeAttrs res = {
        .num_long = 0,
        .is_unsigned = false,
    };
    bool l_is_upper = false;
    bool last_was_u = false;
    for (uint32_t i = 0; i < suffix.len; ++i) {
        if (i == 3) {
            err->kind = INT_CONST_ERR_SUFFIX_TOO_LONG;
            return (IntTypeAttrs){0};
        }
        switch (Str_at(suffix, i)) {
            case 'l':
                if (res.num_long > 0 && l_is_upper) {
                    err->kind = INT_CONST_ERR_CASE_MIXING;
                    return (IntTypeAttrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->kind = INT_CONST_ERR_U_BETWEEN_LS;
                    return (IntTypeAttrs){0};
                } else if (res.num_long == 2) {
                    err->kind = INT_CONST_ERR_TRIPLE_LONG;
                    return (IntTypeAttrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                }
                break;
            case 'L':
                if (res.num_long > 0 && !l_is_upper) {
                    err->kind = INT_CONST_ERR_CASE_MIXING;
                    return (IntTypeAttrs){0};
                } else if (res.num_long > 0 && last_was_u) {
                    err->kind = INT_CONST_ERR_U_BETWEEN_LS;
                    return (IntTypeAttrs){0};
                } else if (res.num_long == 2) {
                    err->kind = INT_CONST_ERR_TRIPLE_LONG;
                    return (IntTypeAttrs){0};
                } else {
                    ++res.num_long;
                    last_was_u = false;
                    l_is_upper = true;
                }
                break;
            case 'u':
            case 'U':
                if (res.is_unsigned) {
                    err->kind = INT_CONST_ERR_DOUBLE_U;
                    return (IntTypeAttrs){0};
                } else {
                    res.is_unsigned = true;
                    last_was_u = true;
                }
                break;
            default:
                err->kind = INT_CONST_ERR_INVALID_CHAR;
                err->invalid_char = Str_at(suffix, i);
                return (IntTypeAttrs){0};
        }
    }
    return res;
}

static uintmax_t int_pow2(uintmax_t exp) {
    if (exp < sizeof exp * CHAR_BIT) {
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

static uintmax_t get_max_int(const ArchTypeInfo* type_info, IntValKind kind) {
    assert(kind == INT_VAL_UINT || kind == INT_VAL_ULINT || kind == INT_VAL_ULLINT
           || kind == INT_VAL_INT || kind == INT_VAL_LINT || kind == INT_VAL_LLINT);

    const ArchIntInfo* info = &type_info->int_info;
    const uint8_t bits_in_char = type_info->bits_in_char;
    switch (kind) {
        case INT_VAL_INT:
            return max_int(bits_in_char * info->int_size);
        case INT_VAL_UINT:
            return max_uint(bits_in_char * info->int_size);
        case INT_VAL_LINT:
            return max_int(bits_in_char * info->lint_size);
        case INT_VAL_ULINT:
            return max_uint(bits_in_char * info->lint_size);
        case INT_VAL_LLINT:
            return max_int(bits_in_char * info->llint_size);
        case INT_VAL_ULLINT:
            return max_uint(bits_in_char * info->llint_size);

        default:
            UNREACHABLE();
    }
}

static IntValKind get_value_type_unsigned(IntTypeAttrs attrs,
                                          uintmax_t val,
                                          const ArchTypeInfo* type_info) {
    assert(attrs.is_unsigned);
    assert(attrs.num_long <= 2);

    switch (attrs.num_long) {
        case 0:
            if (val <= get_max_int(type_info, INT_VAL_UINT)) {
                return INT_VAL_UINT;
            }
            FALLTHROUGH();
        case 1:
            if (val <= get_max_int(type_info, INT_VAL_ULINT)) {
                return INT_VAL_ULINT;
            }
            FALLTHROUGH();
        case 2:
            if (val <= get_max_int(type_info, INT_VAL_ULLINT)) {
                return INT_VAL_ULLINT;
            } else {
                // unsigned will throw error in strotull
                UNREACHABLE();
            }
        default:
            UNREACHABLE();
    }
}

static IntValKind get_value_type_dec(IntTypeAttrs attrs,
                                     uintmax_t val,
                                     const ArchTypeInfo* type_info,
                                     IntConstErr* err) {
    assert(attrs.num_long <= 2);
    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, type_info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(type_info, INT_VAL_INT)) {
                    return INT_VAL_INT;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(type_info, INT_VAL_LINT)) {
                    return INT_VAL_LINT;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(type_info, INT_VAL_LLINT)) {
                    return INT_VAL_LLINT;
                } else {
                    err->kind = INT_CONST_ERR_TOO_LARGE;
                    return INT_VAL_UINT;
                }
            default:
                UNREACHABLE();
        }
    }
}

static IntValKind get_value_type_other(IntTypeAttrs attrs,
                                       uintmax_t val,
                                       const ArchTypeInfo* type_info,
                                       IntConstErr* err) {
    assert(attrs.num_long <= 2);

    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, type_info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(type_info, INT_VAL_INT)) {
                    return INT_VAL_INT;
                } else if (val <= get_max_int(type_info, INT_VAL_UINT)) {
                    return INT_VAL_UINT;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(type_info, INT_VAL_LINT)) {
                    return INT_VAL_LINT;
                } else if (val <= get_max_int(type_info, INT_VAL_ULINT)) {
                    return INT_VAL_ULINT;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(type_info, INT_VAL_LLINT)) {
                    return INT_VAL_LLINT;
                } else if (val <= get_max_int(type_info, INT_VAL_ULLINT)) {
                    return INT_VAL_ULLINT;
                } else {
                    err->kind = INT_CONST_ERR_TOO_LARGE;
                    return INT_VAL_INT;
                }
            default:
                UNREACHABLE();
        }
    }
}

static IntValKind get_uint_leastn_t_type(uint32_t n,
                                         const ArchTypeInfo* type_info);

ParseCharConstRes parse_char_const(Str spell, const ArchTypeInfo* type_info) {
    assert(spell.data);
    assert(type_info);

    Str spell_it = spell;

    IntValKind kind;
    switch (Str_at(spell_it, 0)) {
        case '\'':
            kind = INT_VAL_INT;
            spell_it = Str_incr(spell_it);
            break;
        case 'u':
            if (Str_at(spell_it, 1) == '8') {
                kind = INT_VAL_UCHAR;
            } else if (Str_at(spell_it, 1) == '\'') {
                kind = get_uint_leastn_t_type(16, type_info);
            } else {
                return (ParseCharConstRes){
                    .err =
                        {
                            .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 2,
                            .expected_chars = {'8', '\''},
                            .got_char = Str_at(spell_it, 1),
                        },
                };
            }
            spell_it = Str_advance(spell_it, 2);
            break;
        case 'U':
            kind = get_uint_leastn_t_type(32, type_info);
            if (Str_at(spell_it, 1) != '\'') {
                return (ParseCharConstRes){
                    .err =
                        {
                            .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = Str_at(spell_it, 1),
                        },
                };
            }
            spell_it = Str_advance(spell_it, 2);
            break;
        case 'L':
            kind = get_uint_leastn_t_type(type_info->int_info.wchar_t_size
                                              * type_info->bits_in_char,
                                          type_info);
            if (Str_at(spell_it, 1) != '\'') {
                return (ParseCharConstRes){
                    .err =
                        {
                            .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = Str_at(spell_it, 1),
                        },
                };
            }
            spell_it = Str_advance(spell_it, 2);
            break;
        default:
            return (ParseCharConstRes){
                .err =
                    {
                        .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 4,
                        .expected_chars = {'\'', 'u', 'U', 'L'},
                        .got_char = Str_at(spell_it, 0),
                    },
            };
    }

    if (IntValKind_is_uint(kind)) {
        uintmax_t val;
        switch (Str_at(spell_it, 0)) {
            case '\\':
                spell_it = Str_incr(spell_it);
                switch (Str_at(spell_it, 0)) {
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
                        val = Str_at(spell_it, 0);
                        break;
                    case '0': // TODO: remove hardcoded
                        val = '\0';
                        break;
                    default:
                        // TODO: other escape stuff
                        return (ParseCharConstRes){
                            .err =
                                {
                                    .kind = CHAR_CONST_ERR_INVALID_ESCAPE,
                                    .invalid_escape = Str_at(spell_it, 0),
                                },
                        };
                }
                break;
            default:
                val = Str_at(spell_it, 0);
                break;
        }

        spell_it = Str_incr(spell_it);
        if (Str_at(spell_it, 0) != '\'') {
            return (ParseCharConstRes){
                .err =
                    {
                        .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 1,
                        .expected_chars[0] = '\'',
                        .got_char = Str_at(spell_it, 0),
                    },
            };
        }
        assert(spell_it.len == 1);

        return (ParseCharConstRes){
            .err.kind = CHAR_CONST_ERR_NONE,
            .res = IntVal_create_uint(kind, val),
        };
    } else {
        assert(IntValKind_is_sint(kind));
        intmax_t val;
        switch (Str_at(spell_it, 0)) {
            case '\\':
                spell_it = Str_incr(spell_it);
                switch (Str_at(spell_it, 0)) {
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
                        val = Str_at(spell_it, 0);
                        break;
                    case '0': // TODO: remove hardcoded
                        val = '\0';
                        break;
                    default:
                        // TODO: other escape stuff
                        return (ParseCharConstRes){
                            .err =
                                {
                                    .kind = CHAR_CONST_ERR_INVALID_ESCAPE,
                                    .invalid_escape = Str_at(spell_it, 0),
                                },
                        };
                }
                break;
            default:
                val = Str_at(spell_it, 0);
                break;
        }

        spell_it = Str_incr(spell_it);
        if (Str_at(spell_it, 0) != '\'') {
            return (ParseCharConstRes){
                .err =
                    {
                        .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 1,
                        .expected_chars[0] = '\'',
                        .got_char = Str_at(spell_it, 0),
                    },
            };
        }
        assert(spell_it.len == 1);

        return (ParseCharConstRes){
            .err.kind = CHAR_CONST_ERR_NONE,
            .res = IntVal_create_sint(kind, val),
        };
    }
}

void CharConstErr_print(File out, const CharConstErr* err) {
    assert(err->kind != CHAR_CONST_ERR_NONE);
    switch (err->kind) {
        case CHAR_CONST_ERR_NONE:
            UNREACHABLE();
        case CHAR_CONST_ERR_EXPECTED_CHAR:
            File_put_str("Expected ", out);
            const uint8_t limit = err->num_expected - 1;
            for (uint8_t i = 0; i < limit; ++i) {
                File_printf(out, "{char}, ", err->expected_chars[i]);
            }
            File_printf(out,
                        " or {char} but got {char}",
                        err->expected_chars[limit],
                        err->got_char);
            break;
        case CHAR_CONST_ERR_INVALID_ESCAPE:
            File_printf(out,
                        "Invalid escape character {char}",
                        err->invalid_escape);
            break;
    }
    File_putc('\n', out);
}

static IntValKind get_uint_leastn_t_type(uint32_t n,
                                         const ArchTypeInfo* type_info) {
    assert(n == 8 || n == 16 || n == 32 || n == 64);

    const ArchIntInfo* info = &type_info->int_info;
    const uint8_t bits_in_char = type_info->bits_in_char;
    if (bits_in_char >= n) {
        return INT_VAL_UCHAR;
    } else if ((uint32_t)bits_in_char * info->sint_size >= n) {
        return INT_VAL_USHORT;
    } else if ((uint32_t)bits_in_char * info->int_size >= n) {
        return INT_VAL_UINT;
    } else if ((uint32_t)bits_in_char * info->lint_size >= n) {
        return INT_VAL_ULINT;
    } else if ((uint32_t)bits_in_char * info->llint_size >= n) {
        return INT_VAL_ULLINT;
    }
    UNREACHABLE();
}

