#include <string.h>

#include "testing/asserts.h"

#include "frontend/preproc/preproc.h"

#include "../test_helpers.h"

static void check_token_arr_file(CStr filename, const TokenArr* expected);
static void check_token_arr_str(CStr code, const TokenArr* expected);

TEST(simple) {
    CStr code = CSTR_LIT(
        "typedef struct typedeftest /* This is a comment \n"
        "that goes over\n"
        "multiple lines\n"
        "*/\n"
        "{\n"
        "\tlong int* n;\n"
        "const long double *m;\n"
        "} Typedeftest; // Line comment\n"
        "const char* lstr = \n"
        "L\"Long string literal to check if long strings work\";\n"
        "int n = 0x123213 + 132 << 32 >> 0x123 - 0123 / 12;\n"
        "const char* str = \"Normal string literal\";\n"
        "int arr[1 ? 100 : 1000];\n");

    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) kind
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TOKEN_STRING_LITERAL
#define TOKEN_MACRO_VAL(val, line, idx) ValueKind_is_float(val.kind) ? TOKEN_F_CONSTANT : TOKEN_I_CONSTANT
#include "simple_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {.spelling = str_buf}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {.str_lit = {kind, str_buf}}
#define TOKEN_MACRO_VAL(val_arg, line, idx) {.val = val_arg}
#include "simple_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };

    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_VAL(val, line, idx) {0, {line, idx}}
#include "simple_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(vals), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    check_token_arr_str(code, &expected);
}

TEST(file) {
    CStr filename = CSTR_LIT("../frontend/test/files/no_preproc.c");

    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) kind
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TOKEN_STRING_LITERAL
#define TOKEN_MACRO_VAL(val, line, idx) ValueKind_is_float(val.kind) ? TOKEN_F_CONSTANT : TOKEN_I_CONSTANT
#include "file_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {.spelling = str_buf}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {.str_lit = {kind, str_buf}}
#define TOKEN_MACRO_VAL(val_arg, line, idx) {.val = val_arg}
#include "file_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };

    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_VAL(val, line, idx) {0, {line, idx}}
#include "file_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(vals), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    check_token_arr_file(filename, &expected);
}

TEST(include) {
    CStr filename = CSTR_LIT("../frontend/test/files/include_test/start.c");

    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx, file) kind
#include "include_expected.inc"
#undef TOKEN_MACRO
    };
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx, file) {.spelling = str_buf}
#include "include_expected.inc"
#undef TOKEN_MACRO
    };

    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx, file) {file, {line, idx}}
#include "include_expected.inc"
#undef TOKEN_MACRO
    };

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(vals), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };


    check_token_arr_file(filename, &expected);
}

TEST(preproc_if) {
    CStr filename = CSTR_LIT("../frontend/test/files/preproc_if.c");

    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) kind
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TOKEN_STRING_LITERAL
#define TOKEN_MACRO_VAL(val, line, idx) ValueKind_is_float(val.kind) ? TOKEN_F_CONSTANT : TOKEN_I_CONSTANT
#include "preproc_if_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {.spelling = str_buf}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {.str_lit = {kind, str_buf}}
#define TOKEN_MACRO_VAL(val_arg, line, idx) {.val = val_arg}
#include "preproc_if_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };

    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_VAL(val, line, idx) {0, {line, idx}}
#include "preproc_if_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(vals), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    check_token_arr_file(filename, &expected);
}

TEST(hex_literal_or_var) {
    {
        CStr code = CSTR_LIT("vare-10");
        uint8_t kinds[] = {
            TOKEN_IDENTIFIER,
            TOKEN_SUB,
            TOKEN_I_CONSTANT,
        };
        TokenVal vals[] = {
            {.spelling = STR_BUF_NON_HEAP("vare")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_sint(VALUE_INT, 10)},
        };
        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 5}},
            {0, {1, 6}},
        };

        enum {
            EX_LEN = ARR_LEN(kinds)
        };
        static_assert(EX_LEN == ARR_LEN(vals), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        check_token_arr_str(code, &expected);
    }
    {
        CStr code = CSTR_LIT("var2e-10");
        
        uint8_t kinds[] = {
            TOKEN_IDENTIFIER,
            TOKEN_SUB,
            TOKEN_I_CONSTANT,
        };

        TokenVal vals[] = {
            {.spelling = STR_BUF_NON_HEAP("var2e")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_sint(VALUE_INT, 10)},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 6}},
            {0, {1, 7}},
        };
        enum {
            EX_LEN = ARR_LEN(kinds)
        };
        static_assert(EX_LEN == ARR_LEN(vals), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        check_token_arr_str(code, &expected);
    }
    {
        CStr code = CSTR_LIT("var2p-10");

        uint8_t kinds[] = {
            TOKEN_IDENTIFIER,
            TOKEN_SUB,
            TOKEN_I_CONSTANT,
        };

        TokenVal vals[] = {
            {.spelling = STR_BUF_NON_HEAP("var2p")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_sint(VALUE_INT, 10)},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 6}},
            {0, {1, 7}},
        };
        enum {
            EX_LEN = ARR_LEN(kinds)
        };
        static_assert(EX_LEN == ARR_LEN(vals), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        check_token_arr_str(code, &expected);
    }
}

TEST(dot_float_literal_or_op) {
    {
        CStr code = CSTR_LIT("int n = .001");
        uint8_t kinds[] = {
            TOKEN_INT,
            TOKEN_IDENTIFIER,
            TOKEN_ASSIGN,
            TOKEN_F_CONSTANT,
        };

        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("n")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_float(VALUE_DOUBLE, .001)},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 5}},
            {0, {1, 7}},
            {0, {1, 9}},
        };

        enum {
            EX_LEN = ARR_LEN(kinds),
        };
        static_assert(EX_LEN == ARR_LEN(vals), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        check_token_arr_str(code, &expected);
    }
}

TEST_SUITE_BEGIN(tokenizer){
    REGISTER_TEST(simple),
    REGISTER_TEST(file),
    REGISTER_TEST(include),
    REGISTER_TEST(preproc_if),
    REGISTER_TEST(hex_literal_or_var),
    REGISTER_TEST(dot_float_literal_or_op),
} TEST_SUITE_END()

static void check_token(const TokenArr* got, const TokenArr* ex, uint32_t i) {
    assert(i < got->len);
    ASSERT_TOKEN_KIND(got->kinds[i], ex->kinds[i]);

    if (got->kinds[i] == TOKEN_I_CONSTANT) {
        ASSERT_VALUE_KIND(got->vals[i].val.kind, ex->vals[i].val.kind);
        if (ValueKind_is_sint(got->vals[i].val.kind)) {
            ASSERT_INT(got->vals[i].val.sint_val, ex->vals[i].val.sint_val);
        } else {
            ASSERT_UINT(got->vals[i].val.uint_val, ex->vals[i].val.uint_val);
        }
    } else if (got->kinds[i] == TOKEN_F_CONSTANT) {
        ASSERT_VALUE_KIND(got->vals[i].val.kind, ex->vals[i].val.kind);
        ASSERT_DOUBLE(got->vals[i].val.float_val,
                      ex->vals[i].val.float_val,
                      0.0001);
    } else if (got->kinds[i] == TOKEN_STRING_LITERAL) {
        ASSERT_STR_LIT_KIND(got->vals[i].str_lit.kind, ex->vals[i].str_lit.kind);
        ASSERT_STR(StrBuf_as_str(&got->vals[i].str_lit.contents),
                   StrBuf_as_str(&ex->vals[i].str_lit.contents));
    } else {
        ASSERT_STR(StrBuf_as_str(&got->vals[i].spelling),
                   StrBuf_as_str(&ex->vals[i].spelling));
    }
}

static void compare_tokens(const TokenArr* got, const TokenArr* expected) {
    ASSERT_UINT(got->len, expected->len);
    for (uint32_t i = 0; i < got->len; ++i) {
        check_token(got, expected, i);
    }
}

static void check_token_arr_helper(CStr file_or_code,
                                   const TokenArr* expected,
                                   PreprocRes (*func)(CStr)) {
    PreprocRes preproc_res = func(file_or_code);
    ASSERT(preproc_res.toks.len != 0);
    ASSERT_UINT(preproc_res.toks.len, expected->len);

    compare_tokens(&preproc_res.toks, expected);

    PreprocRes_free(&preproc_res);
}

static void check_token_arr_file(CStr filename, const TokenArr* expected) {
    check_token_arr_helper(filename, expected, tokenize);
}

static PreprocRes tokenize_string_wrapper(CStr code) {
    return tokenize_string(CStr_as_str(code), STR_LIT("code.c"));
}

static void check_token_arr_str(CStr code, const TokenArr* expected) {
    check_token_arr_helper(code, expected, tokenize_string_wrapper);
}
