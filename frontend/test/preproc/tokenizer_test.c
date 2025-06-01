#include "util/mem.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

static void check_token_arr_file(CStr filename, const TokenArr* expected);
static void check_token_arr_str(CStr code, const TokenArr* expected);

static uint32_t TokenArr_add_identifier(TokenArr* arr, StrBuf id) {
    const uint32_t idx = arr->identifiers_len;
    ++arr->identifiers_len;
    arr->identifiers = mycc_realloc(arr->identifiers, arr->identifiers_len * sizeof *arr->identifiers);
    arr->identifiers[idx] = id;
    return idx;
}

static uint32_t TokenArr_add_str_lit(TokenArr* arr, StrLitKind kind, StrBuf buf) {
    const uint32_t idx = arr->str_lits_len;
    ++arr->str_lits_len;
    arr->str_lits = mycc_realloc(arr->str_lits, arr->str_lits_len * sizeof *arr->str_lits);
    arr->str_lits[idx] = (StrLit){
        kind, buf,
    };
    return idx;
}

static uint32_t TokenArr_add_int_const(TokenArr* arr, IntVal val) {
    const uint32_t idx = arr->int_consts_len;
    ++arr->int_consts_len;
    arr->int_consts = mycc_realloc(arr->int_consts, arr->int_consts_len * sizeof *arr->int_consts);
    arr->int_consts[idx] = val;
    return idx;
}

static uint32_t TokenArr_add_float_const(TokenArr* arr, FloatVal val) {
    const uint32_t idx = arr->float_consts_len;
    ++arr->float_consts_len;
    arr->float_consts = mycc_realloc(arr->float_consts, arr->float_consts_len * sizeof *arr->float_consts);
    arr->float_consts[idx] = val;
    return idx;
}

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

    TokenArr expected = TokenArr_create_empty();
    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) kind
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) TOKEN_IDENTIFIER
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TOKEN_STRING_LITERAL
#define TOKEN_MACRO_INT_VAL(int_val, line, idx) TOKEN_I_CONSTANT
#include "simple_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
    };
    expected.kinds = kinds;

    uint32_t val_indices[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) UINT32_MAX
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) TokenArr_add_identifier(&expected, str_buf)
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TokenArr_add_str_lit(&expected, kind, str_buf)
#define TOKEN_MACRO_INT_VAL(int_val, line, idx) TokenArr_add_int_const(&expected, int_val)
#include "simple_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
    };
    expected.val_indices = val_indices;
    /*
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {.spelling = str_buf}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {.str_lit = {kind, str_buf}}
#define TOKEN_MACRO_VAL(val_arg, line, idx) {.val = val_arg}
#include "simple_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };
    */

    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_INT_VAL(val, line, idx) {0, {line, idx}}
#include "simple_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
    };

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(val_indices), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    expected.len = expected.cap = EX_LEN;
    /*
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    */
    check_token_arr_str(code, &expected);
}

TEST(file) {
    CStr filename = CSTR_LIT("../frontend/test/files/no_preproc.c");

    TokenArr expected = TokenArr_create_empty();
    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) kind
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) TOKEN_IDENTIFIER
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TOKEN_STRING_LITERAL
#define TOKEN_MACRO_INT_VAL(val, line, idx) TOKEN_I_CONSTANT
#define TOKEN_MACRO_FLOAT_VAL(val, line, idx) TOKEN_F_CONSTANT
#include "file_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
#undef TOKEN_MACRO_FLOAT_VAL
    };
    expected.kinds = kinds;

    uint32_t val_indices[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) UINT32_MAX
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) TokenArr_add_identifier(&expected, str_buf)
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TokenArr_add_str_lit(&expected, kind, str_buf)
#define TOKEN_MACRO_INT_VAL(val, line, idx) TokenArr_add_int_const(&expected, val)
#define TOKEN_MACRO_FLOAT_VAL(val, line, idx) TokenArr_add_float_const(&expected, val)
#include "file_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
#undef TOKEN_MACRO_FLOAT_VAL
    };
    expected.val_indices = val_indices;
/*
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {.spelling = str_buf}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {.str_lit = {kind, str_buf}}
#define TOKEN_MACRO_VAL(val_arg, line, idx) {.val = val_arg}
#include "file_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };
*/
    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_INT_VAL(val, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_FLOAT_VAL(val, line, idx) {0, {line, idx}}
#include "file_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
#undef TOKEN_MACRO_FLOAT_VAL
    };
    expected.locs = locs;

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(val_indices), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    expected.len = expected.cap = EX_LEN;
    /*
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    */
    check_token_arr_file(filename, &expected);
}

TEST(include) {
    CStr filename = CSTR_LIT("../frontend/test/files/include_test/start.c");

    TokenArr expected = TokenArr_create_empty();
    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx, file) kind
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx, file) TOKEN_IDENTIFIER
#include "include_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
    };
    expected.kinds = kinds;

    uint32_t val_indices[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx, file) UINT32_MAX
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx, file) TokenArr_add_identifier(&expected, str_buf)
#include "include_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
    };
    expected.val_indices = val_indices;
    /*
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx, file) {.spelling = str_buf}
#include "include_expected.inc"
#undef TOKEN_MACRO
    };
    */

    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx, file) {file, {line, idx}}
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx, file) {file, {line, idx}}
#include "include_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
    };
    expected.locs = locs;

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(val_indices), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    expected.len = expected.cap = EX_LEN;
    /*
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    */

    check_token_arr_file(filename, &expected);
}

TEST(preproc_if) {
    CStr filename = CSTR_LIT("../frontend/test/files/preproc_if.c");

    TokenArr expected = TokenArr_create_empty();
    uint8_t kinds[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) kind
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) TOKEN_IDENTIFIER
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TOKEN_STRING_LITERAL
#define TOKEN_MACRO_INT_VAL(val, line, idx) TOKEN_I_CONSTANT
#include "preproc_if_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
    };
    expected.kinds = kinds;

    uint32_t val_indices[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) UINT32_MAX
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) TokenArr_add_identifier(&expected, str_buf)
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) TokenArr_add_str_lit(&expected, kind, str_buf)
#define TOKEN_MACRO_INT_VAL(val, line, idx) TokenArr_add_int_const(&expected, val)
#include "preproc_if_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
    };
    expected.val_indices = val_indices;
    /*
    TokenVal vals[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {.spelling = str_buf}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {.str_lit = {kind, str_buf}}
#define TOKEN_MACRO_VAL(val_arg, line, idx) {.val = val_arg}
#include "preproc_if_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_VAL
    };
    */

    SourceLoc locs[] = {
#define TOKEN_MACRO(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_IDENTIFIER(str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_STR_LIT(kind, str_buf, line, idx) {0, {line, idx}}
#define TOKEN_MACRO_INT_VAL(val, line, idx) {0, {line, idx}}
#include "preproc_if_expected.inc"
#undef TOKEN_MACRO
#undef TOKEN_MACRO_IDENTIFIER
#undef TOKEN_MACRO_STR_LIT
#undef TOKEN_MACRO_INT_VAL
    };
    expected.locs = locs;

    enum {
        EX_LEN = ARR_LEN(kinds),
    };
    static_assert(EX_LEN == ARR_LEN(val_indices), "");
    static_assert(EX_LEN == ARR_LEN(locs), "");
    expected.len = expected.cap = EX_LEN;
    /*
    const TokenArr expected = {
        .len = EX_LEN,
        .cap = EX_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    */
    check_token_arr_file(filename, &expected);
}

TEST(hex_literal_or_var) {
    {
        CStr code = CSTR_LIT("vare-10");

        TokenArr expected = TokenArr_create_empty();
        uint8_t kinds[] = {
            TOKEN_IDENTIFIER,
            TOKEN_SUB,
            TOKEN_I_CONSTANT,
        };
        expected.kinds = kinds;

        uint32_t val_indices[] = {
            TokenArr_add_identifier(&expected, STR_BUF_NON_HEAP("vare")),
            UINT32_MAX,
            TokenArr_add_int_const(&expected, IntVal_create_sint(INT_VAL_INT, 10)),
        };
        expected.val_indices = val_indices;
        /*
        TokenVal vals[] = {
            {.spelling = STR_BUF_NON_HEAP("vare")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_sint(VALUE_INT, 10)},
        };
        */
        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 5}},
            {0, {1, 6}},
        };
        expected.locs = locs;

        enum {
            EX_LEN = ARR_LEN(kinds)
        };
        static_assert(EX_LEN == ARR_LEN(val_indices), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        expected.len = expected.cap = EX_LEN;
        /*
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        */
        check_token_arr_str(code, &expected);
    }
    {
        CStr code = CSTR_LIT("var2e-10");

        TokenArr expected = TokenArr_create_empty();
        uint8_t kinds[] = {
            TOKEN_IDENTIFIER,
            TOKEN_SUB,
            TOKEN_I_CONSTANT,
        };
        expected.kinds = kinds;

        uint32_t val_indices[] = {
            TokenArr_add_identifier(&expected, STR_BUF_NON_HEAP("var2e")),
            UINT32_MAX,
            TokenArr_add_int_const(&expected, IntVal_create_sint(INT_VAL_INT, 10)),
        };
        expected.val_indices = val_indices;
        /*
        TokenVal vals[] = {
            {.spelling = STR_BUF_NON_HEAP("var2e")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_sint(VALUE_INT, 10)},
        };
        */

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 6}},
            {0, {1, 7}},
        };
        expected.locs = locs;
        enum {
            EX_LEN = ARR_LEN(kinds)
        };
        static_assert(EX_LEN == ARR_LEN(val_indices), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        expected.len = expected.cap = EX_LEN;
        /*
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        */
        check_token_arr_str(code, &expected);
    }
    {
        CStr code = CSTR_LIT("var2p-10");

        TokenArr expected = TokenArr_create_empty();
        uint8_t kinds[] = {
            TOKEN_IDENTIFIER,
            TOKEN_SUB,
            TOKEN_I_CONSTANT,
        };
        expected.kinds = kinds;

        uint32_t val_indices[] = {
            TokenArr_add_identifier(&expected, STR_BUF_NON_HEAP("var2p")),
            UINT32_MAX,
            TokenArr_add_int_const(&expected, IntVal_create_sint(INT_VAL_INT, 10)),
        };
        expected.val_indices = val_indices;
        /*
        TokenVal vals[] = {
            {.spelling = STR_BUF_NON_HEAP("var2p")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_sint(VALUE_INT, 10)},
        };
        */

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 6}},
            {0, {1, 7}},
        };
        expected.locs = locs;
        enum {
            EX_LEN = ARR_LEN(kinds)
        };
        static_assert(EX_LEN == ARR_LEN(val_indices), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        expected.len = expected.cap = EX_LEN;
        /*
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        */
        check_token_arr_str(code, &expected);
    }
}

TEST(dot_float_literal_or_op) {
    {
        CStr code = CSTR_LIT("int n = .001");

        TokenArr expected = TokenArr_create_empty();
        uint8_t kinds[] = {
            TOKEN_INT,
            TOKEN_IDENTIFIER,
            TOKEN_ASSIGN,
            TOKEN_F_CONSTANT,
        };
        expected.kinds = kinds;

        uint32_t val_indices[] = {
            UINT32_MAX,
            TokenArr_add_identifier(&expected, STR_BUF_NON_HEAP("n")),
            UINT32_MAX,
            TokenArr_add_float_const(&expected, FloatVal_create(FLOAT_VAL_DOUBLE, .001)),
        };
        expected.val_indices = val_indices;
        /*
        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("n")},
            {.spelling = StrBuf_null()},
            {.val = Value_create_float(VALUE_DOUBLE, .001)},
        };
        */

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 5}},
            {0, {1, 7}},
            {0, {1, 9}},
        };
        expected.locs = locs;

        enum {
            EX_LEN = ARR_LEN(kinds),
        };
        static_assert(EX_LEN == ARR_LEN(val_indices), "");
        static_assert(EX_LEN == ARR_LEN(locs), "");
        expected.len = expected.cap = EX_LEN;
        /*
        const TokenArr expected = {
            .len = EX_LEN,
            .cap = EX_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };
        */
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
    const uint32_t got_val_idx = got->val_indices[i];
    const uint32_t ex_val_idx = ex->val_indices[i];
    switch (got->kinds[i]) {
        case TOKEN_I_CONSTANT: {
            const IntVal got_val = got->int_consts[got_val_idx];
            const IntVal ex_val = ex->int_consts[ex_val_idx];
            ASSERT_INT_VAL_KIND(got_val.kind, ex_val.kind);
            if (IntValKind_is_sint(got_val.kind)) {
                ASSERT_INT(got_val.sint_val, ex_val.sint_val);
            } else {
                ASSERT_UINT(got_val.uint_val, ex_val.uint_val);
            }
            break;
        }
        case TOKEN_F_CONSTANT: {
            const FloatVal got_val = got->float_consts[got_val_idx];
            const FloatVal ex_val = ex->float_consts[ex_val_idx];
            ASSERT_FLOAT_VAL_KIND(got_val.kind, ex_val.kind);
            ASSERT_DOUBLE(got_val.val, ex_val.val, 0.0001);
            break;
        }
        case TOKEN_STRING_LITERAL: {
            const StrLit got_lit = got->str_lits[got_val_idx];
            const StrLit ex_lit = got->str_lits[ex_val_idx];
            ASSERT_STR_LIT_KIND(got_lit.kind, ex_lit.kind);
            ASSERT_STR(StrBuf_as_str(&got_lit.contents),
                       StrBuf_as_str(&ex_lit.contents));
            break;
        }
        case TOKEN_IDENTIFIER: {
            const StrBuf got_spell = got->identifiers[got_val_idx];
            const StrBuf ex_spell = ex->identifiers[ex_val_idx];
            ASSERT_STR(StrBuf_as_str(&got_spell), StrBuf_as_str(&ex_spell));
            break;
        }
        default:
            // TODO: fix this
            //ASSERT_INT(got_val_idx, ex_val_idx);
            //ASSERT_INT(got_val_idx, UINT32_MAX);
            break;
    }
    /*
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
    */
}

static void compare_tokens(const TokenArr* got, const TokenArr* expected) {
    ASSERT_UINT(got->len, expected->len);
    for (uint32_t i = 0; i < got->len; ++i) {
        check_token(got, expected, i);
    }
}

static void TokenArr_free_identifiers_only(const TokenArr* arr) {
    mycc_free(arr->identifiers);
    mycc_free(arr->int_consts);
    mycc_free(arr->float_consts);
    mycc_free(arr->str_lits);
}

static void check_token_arr_helper(CStr file_or_code,
                                   const TokenArr* expected,
                                   TestPreprocRes (*func)(CStr)) {
    TestPreprocRes preproc_res = func(file_or_code);
    ASSERT(preproc_res.toks.len != 0);
    ASSERT_UINT(preproc_res.toks.len, expected->len);

    compare_tokens(&preproc_res.toks, expected);

    TestPreprocRes_free(&preproc_res);
    TokenArr_free_identifiers_only(expected);
}

static void check_token_arr_file(CStr filename, const TokenArr* expected) {
    check_token_arr_helper(filename, expected, tokenize);
}

static TestPreprocRes tokenize_string_wrapper(CStr code) {
    return tokenize_string(CStr_as_str(code), STR_LIT("code.c"),
                           &(PreprocInitialStrings){0});
}

static void check_token_arr_str(CStr code, const TokenArr* expected) {
    check_token_arr_helper(code, expected, tokenize_string_wrapper);
}
