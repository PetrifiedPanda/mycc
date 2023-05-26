#include "frontend/preproc/num_parse.h"

#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>

#include "util/macro_util.h"

ParseFloatConstRes parse_float_const(const char* spell) {
    const char* end = spell; // so end is set
    assert(errno == 0);
    double val = strtod(spell, (char**)&end);
    if (errno != 0) {
        assert(errno == ERANGE);
        errno = 0;
        return (ParseFloatConstRes){
            .err.kind = FLOAT_CONST_ERR_TOO_LARGE,
        };
    }
    ValueKind t = VALUE_D;
    assert(spell <= end);
    if (*end != '\0') {
        if (*end == 'f' || *end == 'F') {
            t = VALUE_F;
        } else if (*end == 'l' || *end == 'L') {
            t = VALUE_LD;
        } else {
            return (ParseFloatConstRes){
                .err =
                    {
                        .kind = FLOAT_CONST_ERR_INVALID_CHAR,
                        .invalid_char = *end,
                    },
            };
        }
        ++end;
        if (*end != '\0') {
            return (ParseFloatConstRes){
                .err.kind = FLOAT_CONST_ERR_SUFFIX_TOO_LONG,
            };
        }
    }

    return (ParseFloatConstRes){
        .err.kind = FLOAT_CONST_ERR_NONE,
        .res = Value_create_float(t, val),
    };
}

void FloatConstErr_print(FILE* out, const FloatConstErr* err) {
    assert(err != FLOAT_CONST_ERR_NONE);

    switch (err->kind) {
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

typedef struct {
    unsigned short num_long;
    bool is_unsigned;
} IntTypeAttrs;

static IntTypeAttrs get_int_attrs(const char* suffix, IntConstErr* err);

static ValueKind get_value_type_dec(IntTypeAttrs attrs,
                                    uintmax_t val,
                                    const ArchTypeInfo* type_info,
                                    IntConstErr* err);

static ValueKind get_value_type_other(IntTypeAttrs attrs,
                                      uintmax_t val,
                                      const ArchTypeInfo* type_info,
                                      IntConstErr* err);

ParseIntConstRes parse_int_const(const char* spell,
                                 const ArchTypeInfo* type_info) {
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
        return (ParseIntConstRes){
            .err.kind = INT_CONST_ERR_TOO_LARGE,
        };
    }

    assert(spell <= suffix);

    IntConstErr err = {
        .kind = INT_CONST_ERR_NONE,
    };
    IntTypeAttrs attrs = get_int_attrs(suffix, &err);
    if (err.kind != INT_CONST_ERR_NONE) {
        return (ParseIntConstRes){
            .err = err,
        };
    }
    const ValueKind kind = base == DEC
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
        .res = ValueKind_is_sint(kind) ? Value_create_sint(kind, (int64_t)val)
                                       : Value_create_uint(kind, val),
    };
}

void IntConstErr_print(FILE* out, const IntConstErr* err) {
    assert(out);
    assert(err);
    assert(err->kind != INT_CONST_ERR_NONE);

    switch (err->kind) {
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

static IntTypeAttrs get_int_attrs(const char* suffix, IntConstErr* err) {
    IntTypeAttrs res = {
        .num_long = 0,
        .is_unsigned = false,
    };
    bool l_is_upper = false;
    bool last_was_u = false;
    for (size_t i = 0; suffix[i] != '\0'; ++i) {
        if (i == 3) {
            err->kind = INT_CONST_ERR_SUFFIX_TOO_LONG;
            return (IntTypeAttrs){0};
        }
        switch (suffix[i]) {
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
                err->invalid_char = suffix[i];
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

static uintmax_t get_max_int(const ArchTypeInfo* type_info, ValueKind kind) {
    assert(kind == VALUE_UI || kind == VALUE_UL || kind == VALUE_ULL
           || kind == VALUE_I || kind == VALUE_L || kind == VALUE_LL);

    const ArchIntInfo* info = &type_info->int_info;
    const uint8_t bits_in_char = type_info->bits_in_char;
    switch (kind) {
        case VALUE_I:
            return max_int(bits_in_char * info->int_size);
        case VALUE_UI:
            return max_uint(bits_in_char * info->int_size);
        case VALUE_L:
            return max_int(bits_in_char * info->lint_size);
        case VALUE_UL:
            return max_uint(bits_in_char * info->lint_size);
        case VALUE_LL:
            return max_int(bits_in_char * info->llint_size);
        case VALUE_ULL:
            return max_uint(bits_in_char * info->llint_size);

        default:
            UNREACHABLE();
    }
}

static ValueKind get_value_type_unsigned(IntTypeAttrs attrs,
                                         uintmax_t val,
                                         const ArchTypeInfo* type_info) {
    assert(attrs.is_unsigned);
    assert(attrs.num_long <= 2);

    switch (attrs.num_long) {
        case 0:
            if (val <= get_max_int(type_info, VALUE_UI)) {
                return VALUE_UI;
            }
            FALLTHROUGH();
        case 1:
            if (val <= get_max_int(type_info, VALUE_UL)) {
                return VALUE_UL;
            }
            FALLTHROUGH();
        case 2:
            if (val <= get_max_int(type_info, VALUE_ULL)) {
                return VALUE_ULL;
            } else {
                // unsigned will throw error in strotull
                UNREACHABLE();
            }
        default:
            UNREACHABLE();
    }
}

static ValueKind get_value_type_dec(IntTypeAttrs attrs,
                                    uintmax_t val,
                                    const ArchTypeInfo* type_info,
                                    IntConstErr* err) {
    assert(attrs.num_long <= 2);
    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, type_info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(type_info, VALUE_I)) {
                    return VALUE_I;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(type_info, VALUE_L)) {
                    return VALUE_L;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(type_info, VALUE_LL)) {
                    return VALUE_LL;
                } else {
                    err->kind = INT_CONST_ERR_TOO_LARGE;
                    return VALUE_UI;
                }
            default:
                UNREACHABLE();
        }
    }
}

static ValueKind get_value_type_other(IntTypeAttrs attrs,
                                      uintmax_t val,
                                      const ArchTypeInfo* type_info,
                                      IntConstErr* err) {
    assert(attrs.num_long <= 2);

    if (attrs.is_unsigned) {
        return get_value_type_unsigned(attrs, val, type_info);
    } else {
        switch (attrs.num_long) {
            case 0:
                if (val <= get_max_int(type_info, VALUE_I)) {
                    return VALUE_I;
                } else if (val <= get_max_int(type_info, VALUE_UI)) {
                    return VALUE_UI;
                }
                FALLTHROUGH();
            case 1:
                if (val <= get_max_int(type_info, VALUE_L)) {
                    return VALUE_L;
                } else if (val <= get_max_int(type_info, VALUE_UL)) {
                    return VALUE_UL;
                }
                FALLTHROUGH();
            case 2:
                if (val <= get_max_int(type_info, VALUE_LL)) {
                    return VALUE_LL;
                } else if (val <= get_max_int(type_info, VALUE_ULL)) {
                    return VALUE_ULL;
                } else {
                    err->kind = INT_CONST_ERR_TOO_LARGE;
                    return VALUE_I;
                }
            default:
                UNREACHABLE();
        }
    }
}

static ValueKind get_uint_leastn_t_type(size_t n,
                                        const ArchTypeInfo* type_info);

ParseCharConstRes parse_char_const(const char* spell,
                                   const ArchTypeInfo* type_info) {
    assert(spell);
    assert(type_info);

    ValueKind kind;
    switch (*spell) {
        case '\'':
            kind = VALUE_I;
            ++spell;
            break;
        case 'u':
            if (spell[1] == '8') {
                kind = VALUE_UC;
            } else if (spell[1] == '\'') {
                kind = get_uint_leastn_t_type(16, type_info);
            } else {
                return (ParseCharConstRes){
                    .err =
                        {
                            .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 2,
                            .expected_chars = {'8', '\''},
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        case 'U':
            kind = get_uint_leastn_t_type(32, type_info);
            if (spell[1] != '\'') {
                return (ParseCharConstRes){
                    .err =
                        {
                            .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        case 'L':
            kind = get_uint_leastn_t_type(type_info->int_info.wchar_t_size
                                              * type_info->bits_in_char,
                                          type_info);
            if (spell[1] != '\'') {
                return (ParseCharConstRes){
                    .err =
                        {
                            .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                            .num_expected = 1,
                            .expected_chars[0] = '\'',
                            .got_char = spell[1],
                        },
                };
            }
            spell += 2;
            break;
        default:
            return (ParseCharConstRes){
                .err =
                    {
                        .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 4,
                        .expected_chars = {'\'', 'u', 'U', 'L'},
                        .got_char = *spell,
                    },
            };
    }

    if (ValueKind_is_uint(kind)) {
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
                        return (ParseCharConstRes){
                            .err =
                                {
                                    .kind = CHAR_CONST_ERR_INVALID_ESCAPE,
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
            return (ParseCharConstRes){
                .err =
                    {
                        .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 1,
                        .expected_chars[0] = '\'',
                        .got_char = *spell,
                    },
            };
        }
        assert(spell[1] == '\0');

        return (ParseCharConstRes){
            .err.kind = CHAR_CONST_ERR_NONE,
            .res = Value_create_uint(kind, val),
        };
    } else {
        assert(ValueKind_is_sint(kind));
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
                        return (ParseCharConstRes){
                            .err =
                                {
                                    .kind = CHAR_CONST_ERR_INVALID_ESCAPE,
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
            return (ParseCharConstRes){
                .err =
                    {
                        .kind = CHAR_CONST_ERR_EXPECTED_CHAR,
                        .num_expected = 1,
                        .expected_chars[0] = '\'',
                        .got_char = *spell,
                    },
            };
        }
        assert(spell[1] == '\0');

        return (ParseCharConstRes){
            .err.kind = CHAR_CONST_ERR_NONE,
            .res = Value_create_sint(kind, val),
        };
    }
}

void CharConstErr_print(FILE* out, const CharConstErr* err) {
    assert(err->kind != CHAR_CONST_ERR_NONE);
    switch (err->kind) {
        case CHAR_CONST_ERR_NONE:
            UNREACHABLE();
        case CHAR_CONST_ERR_EXPECTED_CHAR:
            fputs("Expected ", out);
            const uint8_t limit = err->num_expected - 1;
            for (uint8_t i = 0; i < limit; ++i) {
                fprintf(out, "%c, ", err->expected_chars[i]);
            }
            fprintf(out,
                    " or %c but got %c",
                    err->expected_chars[limit],
                    err->got_char);
            break;
        case CHAR_CONST_ERR_INVALID_ESCAPE:
            fprintf(out, "Invalid escape character %c", err->invalid_escape);
            break;
    }
    fputc('\n', out);
}

static ValueKind get_uint_leastn_t_type(size_t n,
                                        const ArchTypeInfo* type_info) {
    assert(n == 8 || n == 16 || n == 32 || n == 64);

    const ArchIntInfo* info = &type_info->int_info;
    const uint8_t bits_in_char = type_info->bits_in_char;
    if (bits_in_char >= n) {
        return VALUE_UC;
    } else if (bits_in_char * info->sint_size >= n) {
        return VALUE_US;
    } else if (bits_in_char * info->int_size >= n) {
        return VALUE_UI;
    } else if (bits_in_char * info->lint_size >= n) {
        return VALUE_UL;
    } else if (bits_in_char * info->llint_size >= n) {
        return VALUE_ULL;
    }
    UNREACHABLE();
}

