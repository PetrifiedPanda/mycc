#include "frontend/Token.h"
#include "frontend/preproc/PreprocMacro.h"

#include "util/mem.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

static void compare_preproc_macros(const PreprocMacro* got,
                                   const PreprocMacro* ex) {
    ASSERT_BOOL(got->is_func_macro, ex->is_func_macro);
    ASSERT_UINT(got->num_args, ex->num_args);
    ASSERT_BOOL(got->is_variadic, ex->is_variadic);

    ASSERT_UINT(got->expansion_len, ex->expansion_len);

    for (uint32_t i = 0; i < got->expansion_len; ++i) {
        const TokenValOrArg* got_item = &got->vals[i];
        const TokenValOrArg* ex_item = &ex->vals[i];
        ASSERT_TOKEN_KIND(got->kinds[i], ex->kinds[i]); 
        if (got->kinds[i] == TOKEN_INVALID) {
            ASSERT_UINT(got_item->arg_num, ex_item->arg_num);
        } else {
            const StrBuf* got_spell = &got_item->val;
            const StrBuf* ex_spell = &ex_item->val;

            ASSERT_STR(StrBuf_as_str(got_spell),
                       StrBuf_as_str(ex_spell));    
        }
    }
}

TEST(parse_obj_like) {
    {
        // #define TEST_MACRO
        uint8_t kinds[] = {
            TOKEN_PP_STRINGIFY,
            TOKEN_IDENTIFIER,
            TOKEN_IDENTIFIER,
        };
        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("define")},
            {.spelling = STR_BUF_NON_HEAP("TEST_MACRO")},
        };
        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
        };

        static_assert(ARR_LEN(kinds) == ARR_LEN(vals), "array lengths don't match");
        static_assert(ARR_LEN(kinds) == ARR_LEN(locs), "array lengths don't match");

        TokenArr arr = {
            .len = ARR_LEN(kinds),
            .cap = arr.len,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 10, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        PreprocMacro ex = {
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .kinds = NULL,
            .vals = NULL,
        };

        compare_preproc_macros(&got, &ex);
        mycc_free(got.kinds);
        mycc_free(got.vals);
    }
    {
        // #define ANOTHER_MACRO 1 + 2 * 3 - func(a, b)
        uint8_t kinds[] = {
            TOKEN_PP_STRINGIFY,
            TOKEN_IDENTIFIER,
            TOKEN_IDENTIFIER,
            TOKEN_I_CONSTANT,
            TOKEN_ADD,
            TOKEN_I_CONSTANT,
            TOKEN_ASTERISK,
            TOKEN_I_CONSTANT,
            TOKEN_SUB,
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_IDENTIFIER,
            TOKEN_RBRACKET,
        };

        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("define")},
            {.spelling = STR_BUF_NON_HEAP("ANOTHER_MACRO")},
            {.spelling = STR_BUF_NON_HEAP("1")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("2")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("3")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("func")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("b")},
            {.spelling = StrBuf_null()},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
            {0, {1, 23}},
            {0, {1, 25}},
            {0, {1, 27}},
            {0, {1, 29}},
            {0, {1, 31}},
            {0, {1, 33}},
            {0, {1, 35}},
            {0, {1, 39}},
            {0, {1, 40}},
            {0, {1, 41}},
            {0, {1, 43}},
            {0, {1, 44}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(kinds),
            EXPANSION_LEN = TOKENS_LEN - 3
        };
        static_assert(TOKENS_LEN == ARR_LEN(vals), "array lengths don't match");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "array lengths don't match");
        TokenValOrArg ex_vals[EXPANSION_LEN];
        uint8_t ex_kinds[EXPANSION_LEN];
        for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
            ex_kinds[i] = kinds[i + 3];
            ex_vals[i] = (TokenValOrArg){.val = vals[i + 3].spelling};
        }

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = arr.len,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 13, &err);

        PreprocMacro ex = {
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .kinds = ex_kinds,
            .vals = ex_vals,
        };

        compare_preproc_macros(&got, &ex);
        mycc_free(got.kinds);
        mycc_free(got.vals);
    }
}

TEST(parse_func_like) {
    {
        // #define FUNC_LIKE(a, b, c) a != 38 ? b * other_name : c + a
        uint8_t kinds[] = {
            TOKEN_PP_STRINGIFY,
            TOKEN_IDENTIFIER,
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_IDENTIFIER,
            TOKEN_RBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_NE,
            TOKEN_I_CONSTANT,
            TOKEN_QMARK,
            TOKEN_IDENTIFIER,
            TOKEN_ASTERISK,
            TOKEN_IDENTIFIER,
            TOKEN_COLON,
            TOKEN_IDENTIFIER,
            TOKEN_ADD,
            TOKEN_IDENTIFIER,
        };

        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("define")},
            {.spelling = STR_BUF_NON_HEAP("FUNC_LIKE")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("b")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("c")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("38")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("b")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("other_name")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("c")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
            {0, {1, 18}},
            {0, {1, 19}},
            {0, {1, 20}},
            {0, {1, 22}},
            {0, {1, 23}},
            {0, {1, 25}},
            {0, {1, 26}},
            {0, {1, 28}},
            {0, {1, 30}},
            {0, {1, 33}},
            {0, {1, 36}},
            {0, {1, 38}},
            {0, {1, 40}},
            {0, {1, 42}},
            {0, {1, 53}},
            {0, {1, 55}},
            {0, {1, 57}},
            {0, {1, 59}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(kinds),
            EXPANSION_LEN = TOKENS_LEN - 10
        };

        static_assert(TOKENS_LEN == ARR_LEN(vals), "");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "");
        
        uint8_t ex_kinds[EXPANSION_LEN] = {
            TOKEN_INVALID,
            kinds[11],
            kinds[12],
            kinds[13],
            TOKEN_INVALID,
            kinds[15],
            kinds[16],
            kinds[17],
            TOKEN_INVALID,
            kinds[19],
            TOKEN_INVALID,
        };
        TokenValOrArg ex_vals[EXPANSION_LEN] = {
            {.arg_num = 0},
            {.val = vals[11].spelling},
            {.val = vals[12].spelling},
            {.val = vals[13].spelling},
            {.arg_num = 1},
            {.val = vals[15].spelling},
            {.val = vals[16].spelling},
            {.val = vals[17].spelling},
            {.arg_num = 2},
            {.val = vals[19].spelling},
            {.arg_num = 0},
        };

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .kinds = ex_kinds,
            .vals = ex_vals,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.kinds);
        mycc_free(got.vals);
    }
    {
        // #define NO_PARAMS() 1 + 2 + 3
        uint8_t kinds[] = {
            TOKEN_PP_STRINGIFY,
            TOKEN_IDENTIFIER,
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
            TOKEN_RBRACKET,
            TOKEN_I_CONSTANT,
            TOKEN_ADD,
            TOKEN_I_CONSTANT,
            TOKEN_ADD,
            TOKEN_I_CONSTANT,
        };
        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("define")},
            {.spelling = STR_BUF_NON_HEAP("NO_PARAMS")},
            {.spelling = StrBuf_null()},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("1")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("2")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("3")},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
            {0, {1, 18}},
            {0, {1, 19}},
            {0, {1, 21}},
            {0, {1, 23}},
            {0, {1, 25}},
            {0, {1, 27}},
            {0, {1, 29}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(kinds),
            EXPANSION_LEN = TOKENS_LEN - 5
        };

        static_assert(TOKENS_LEN == ARR_LEN(vals), "");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "");
        
        uint8_t ex_kinds[EXPANSION_LEN];
        TokenValOrArg ex_vals[EXPANSION_LEN];
        for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
            ex_kinds[i] = kinds[i + 5];
            ex_vals[i].val = vals[i + 5].spelling;
        }

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .kinds = ex_kinds,
            .vals = ex_vals,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.kinds);
        mycc_free(got.vals);
    }
    {
        // #define NO_PARAMS_EMPTY()
        uint8_t kinds[] = {
            TOKEN_PP_STRINGIFY,
            TOKEN_IDENTIFIER,
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
            TOKEN_RBRACKET,
        };

        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("define")},
            {.spelling = STR_BUF_NON_HEAP("NO_PARAMS_EMPTY")},
            {.spelling = StrBuf_null()},
            {.spelling = StrBuf_null()},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
            {0, {1, 24}},
            {0, {1, 25}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(kinds),
        };
        static_assert(TOKENS_LEN == ARR_LEN(vals), "");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "");

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .kinds = NULL,
            .vals = NULL,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 15, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.kinds);
        mycc_free(got.vals);
    }
}

TEST(parse_variadic) {
    {
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name : c + a
        uint8_t kinds[] = {
            TOKEN_PP_STRINGIFY,
            TOKEN_IDENTIFIER,
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_ELLIPSIS,
            TOKEN_RBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_NE,
            TOKEN_I_CONSTANT,
            TOKEN_QMARK,
            TOKEN_IDENTIFIER,
            TOKEN_ASTERISK,
            TOKEN_IDENTIFIER,
            TOKEN_COLON,
            TOKEN_IDENTIFIER,
            TOKEN_ADD,
            TOKEN_IDENTIFIER,
        };

        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("define")},
            {.spelling = STR_BUF_NON_HEAP("FUNC_LIKE")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("b")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("c")},
            {.spelling = StrBuf_null()},
            {.spelling = StrBuf_null()},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("38")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("b")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("other_name")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("c")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
            {0, {1, 18}},
            {0, {1, 19}},
            {0, {1, 20}},
            {0, {1, 22}},
            {0, {1, 23}},
            {0, {1, 25}},
            {0, {1, 26}},
            {0, {1, 28}},
            {0, {1, 31}},
            {0, {1, 33}},
            {0, {1, 35}},
            {0, {1, 38}},
            {0, {1, 41}},
            {0, {1, 43}},
            {0, {1, 45}},
            {0, {1, 47}},
            {0, {1, 58}},
            {0, {1, 60}},
            {0, {1, 62}},
            {0, {1, 64}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(kinds),
            EXPANSION_LEN = TOKENS_LEN - 12,
        };

        static_assert(TOKENS_LEN == ARR_LEN(vals), "");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "");
        
        uint8_t ex_kinds[EXPANSION_LEN] = {
            TOKEN_INVALID,
            kinds[13],
            kinds[14],
            kinds[15],
            TOKEN_INVALID,
            kinds[17],
            kinds[18],
            kinds[19],
            TOKEN_INVALID,
            kinds[21],
            TOKEN_INVALID,
        };
        TokenValOrArg ex_vals[EXPANSION_LEN] = {
            {.arg_num = 0},
            {.val = vals[13].spelling},
            {.val = vals[14].spelling},
            {.val = vals[15].spelling},
            {.arg_num = 1},
            {.val = vals[17].spelling},
            {.val = vals[18].spelling},
            {.val = vals[19].spelling},
            {.arg_num = 2},
            {.val = vals[21].spelling},
            {.arg_num = 0},
        };

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = true,

            .expansion_len = EXPANSION_LEN,
            .kinds = ex_kinds,
            .vals = ex_vals,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.kinds);
        mycc_free(got.vals);
    }
    {
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name(__VA_ARGS__)
        // : c + a
        uint8_t kinds[] = {
            TOKEN_PP_STRINGIFY,
            TOKEN_IDENTIFIER,
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_IDENTIFIER,
            TOKEN_COMMA,
            TOKEN_ELLIPSIS,
            TOKEN_RBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_NE,
            TOKEN_I_CONSTANT,
            TOKEN_QMARK,
            TOKEN_IDENTIFIER,
            TOKEN_ASTERISK,
            TOKEN_IDENTIFIER,
            TOKEN_LBRACKET,
            TOKEN_IDENTIFIER,
            TOKEN_RBRACKET,
            TOKEN_COLON,
            TOKEN_IDENTIFIER,
            TOKEN_ADD,
            TOKEN_IDENTIFIER,

        };

        TokenVal vals[] = {
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("define")},
            {.spelling = STR_BUF_NON_HEAP("FUNC_LIKE")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("b")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("c")},
            {.spelling = StrBuf_null()},
            {.spelling = StrBuf_null()},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("38")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("b")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("other_name")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("__VA_ARGS__")},
            {.spelling = StrBuf_null()},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("c")},
            {.spelling = StrBuf_null()},
            {.spelling = STR_BUF_NON_HEAP("a")},
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
            {0, {1, 18}},
            {0, {1, 19}},
            {0, {1, 20}},
            {0, {1, 22}},
            {0, {1, 23}},
            {0, {1, 25}},
            {0, {1, 26}},
            {0, {1, 28}},
            {0, {1, 31}},
            {0, {1, 33}},
            {0, {1, 35}},
            {0, {1, 38}},
            {0, {1, 41}},
            {0, {1, 43}},
            {0, {1, 45}},
            {0, {1, 47}},
            {0, {1, 48}},
            {0, {1, 49}},
            {0, {1, 60}},
            {0, {1, 62}},
            {0, {1, 64}},
            {0, {1, 66}},
            {0, {1, 68}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(kinds),
            EXPANSION_LEN = TOKENS_LEN - 12,
        };

        static_assert(TOKENS_LEN == ARR_LEN(vals), "");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "");
        
        uint8_t ex_kinds[EXPANSION_LEN] = {
            TOKEN_INVALID,
            kinds[13],
            kinds[14],
            kinds[15],
            TOKEN_INVALID,
            kinds[17],
            kinds[18],
            kinds[19],
            TOKEN_INVALID,
            kinds[21],
            kinds[22],
            TOKEN_INVALID,
            kinds[24],
            TOKEN_INVALID,
        };
        TokenValOrArg ex_vals[EXPANSION_LEN] = {
            {.arg_num = 0},
            {.val = vals[13].spelling},
            {.val = vals[14].spelling},
            {.val = vals[15].spelling},
            {.arg_num = 1},
            {.val = vals[17].spelling},
            {.val = vals[18].spelling},
            {.val = vals[19].spelling},
            {.arg_num = 3},
            {.val = vals[21].spelling},
            {.val = vals[22].spelling},
            {.arg_num = 2},
            {.val = vals[24].spelling},
            {.arg_num = 0},
        };

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = true,

            .expansion_len = EXPANSION_LEN,
            .kinds = ex_kinds,
            .vals = ex_vals,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .vals = vals,
            .locs = locs,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.kinds);
        mycc_free(got.vals);
    }
}

static void is_zeroed_macro(const PreprocMacro* got) {
    ASSERT_BOOL(got->is_func_macro, false);
    ASSERT_BOOL(got->is_variadic, false);
    ASSERT_UINT(got->num_args, 0);
    ASSERT_UINT(got->expansion_len, 0);
    ASSERT_NULL(got->kinds);
    ASSERT_NULL(got->vals);
}

TEST(parse_duplicate_arg_name) {
    // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name(__VA_ARGS__)
    // : c + a
    uint8_t kinds[] = {
        TOKEN_PP_STRINGIFY,
        TOKEN_IDENTIFIER,
        TOKEN_IDENTIFIER,
        TOKEN_LBRACKET,
        TOKEN_IDENTIFIER,
        TOKEN_COMMA,        
        TOKEN_IDENTIFIER,
        TOKEN_COMMA,        
        TOKEN_IDENTIFIER,
        TOKEN_COMMA,
        TOKEN_ELLIPSIS,
        TOKEN_RBRACKET,
        TOKEN_IDENTIFIER,
        TOKEN_NE,
        TOKEN_I_CONSTANT,
        TOKEN_QMARK,
        TOKEN_IDENTIFIER,
        TOKEN_ASTERISK,
        TOKEN_IDENTIFIER,
        TOKEN_LBRACKET,
        TOKEN_IDENTIFIER,
        TOKEN_RBRACKET,
        TOKEN_COLON,
        TOKEN_IDENTIFIER,
        TOKEN_ADD,
        TOKEN_IDENTIFIER,
    };

    TokenVal vals[] = {
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("define")},
        {.spelling = STR_BUF_NON_HEAP("FUNC_LIKE")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("a")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("b")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("c")},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("a")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("38")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("b")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("other_name")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("__VA_ARGS__")},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("c")},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("a")},
    };

    SourceLoc locs[] = {
        {0, {1, 1}},
        {0, {1, 2}},
        {0, {1, 9}},
        {0, {1, 18}},
        {0, {1, 19}},
        {0, {1, 20}},
        {0, {1, 22}},
        {0, {1, 23}},
        {0, {1, 25}},
        {0, {1, 26}},
        {0, {1, 28}},
        {0, {1, 31}},
        {0, {1, 33}},
        {0, {1, 35}},
        {0, {1, 38}},
        {0, {1, 41}},
        {0, {1, 43}},
        {0, {1, 45}},
        {0, {1, 47}},
        {0, {1, 48}},
        {0, {1, 49}},
        {0, {1, 60}},
        {0, {1, 62}},
        {0, {1, 64}},
        {0, {1, 66}},
        {0, {1, 68}},
    };
    enum {
        TOKENS_LEN = ARR_LEN(kinds),
    };

    static_assert(TOKENS_LEN == ARR_LEN(vals), "");
    static_assert(TOKENS_LEN == ARR_LEN(locs), "");
    TokenArr arr = {
        .len = TOKENS_LEN,
        .cap = TOKENS_LEN,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    // change c to a
    vals[8].spelling = STR_BUF_NON_HEAP("a");
    {
        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        is_zeroed_macro(&got);
        ASSERT(err.kind == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(StrBuf_as_str(&err.duplicate_arg_name), STR_LIT("a"));
        ASSERT_UINT(err.base.loc.file_idx, 0);
        ASSERT_UINT(err.base.loc.file_loc.line, 1);
        ASSERT_UINT(err.base.loc.file_loc.index, 25);
    }

    vals[8].spelling = STR_BUF_NON_HEAP("c");
    vals[6].spelling = STR_BUF_NON_HEAP("c");
    {
        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        is_zeroed_macro(&got);
        ASSERT(err.kind == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(StrBuf_as_str(&err.duplicate_arg_name), STR_LIT("c"));
        ASSERT_UINT(err.base.loc.file_idx, 0);
        ASSERT_UINT(err.base.loc.file_loc.line, 1);
        ASSERT_UINT(err.base.loc.file_loc.index, 25);
    }
    vals[6].spelling = STR_BUF_NON_HEAP("a");
    {
        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        is_zeroed_macro(&got);
        ASSERT(err.kind == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(StrBuf_as_str(&err.duplicate_arg_name), STR_LIT("a"));
        ASSERT_UINT(err.base.loc.file_idx, 0);
        ASSERT_UINT(err.base.loc.file_loc.line, 1);
        ASSERT_UINT(err.base.loc.file_loc.index, 22);
    }
}

TEST(parse_obj_like_starting_with_bracket) {
    // The token "NULL" will already have been taken to register it later
    // #define NULL ((void*)0)
    uint8_t kinds[] = {
        TOKEN_PP_STRINGIFY,
        TOKEN_IDENTIFIER,
        TOKEN_IDENTIFIER,
        TOKEN_LBRACKET,
        TOKEN_LBRACKET,
        TOKEN_VOID, // TODO: ?
        TOKEN_VOID,
        TOKEN_ASTERISK,
        TOKEN_RBRACKET,
        TOKEN_I_CONSTANT,
        TOKEN_RBRACKET,
    };

    TokenVal vals[] = {
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("define")},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = StrBuf_null()},
        {.spelling = STR_BUF_NON_HEAP("0")},
        {.spelling = StrBuf_null()},
    };

    SourceLoc locs[] = {
        {0, {1, 1}},
        {0, {1, 2}},
        {0, {1, 9}},
        {0, {1, 15}},
        {0, {1, 16}},
        {0, {1, 17}},
        {0, {1, 17}},
        {0, {1, 21}},
        {0, {1, 22}},
        {0, {1, 23}},
        {0, {1, 24}},
    };

    enum {
        TOKENS_LEN = ARR_LEN(kinds),
        EXPANSION_LEN = TOKENS_LEN - 3,
    };
    static_assert(TOKENS_LEN == ARR_LEN(vals), "");
    static_assert(TOKENS_LEN == ARR_LEN(locs), "");

    uint8_t ex_kinds[EXPANSION_LEN];
    TokenValOrArg ex_vals[EXPANSION_LEN];
    for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
        ex_kinds[i] = kinds[i + 3];
        ex_vals[i].val = vals[i + 3].spelling;
    }

    TokenArr arr = {
        .len = TOKENS_LEN,
        .cap = arr.len,
        .kinds = kinds,
        .vals = vals,
        .locs = locs,
    };
    
    PreprocErr err = PreprocErr_create();
    PreprocMacro got = parse_preproc_macro(&arr, 4, &err);

    PreprocMacro ex = {
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = EXPANSION_LEN,
        .kinds = ex_kinds,
        .vals = ex_vals,
    };

    compare_preproc_macros(&got, &ex);
    mycc_free(got.kinds);
    mycc_free(got.vals);
}

TEST_SUITE_BEGIN(preproc_macro_parser){
    REGISTER_TEST(parse_obj_like),
    REGISTER_TEST(parse_func_like),
    REGISTER_TEST(parse_variadic),
    REGISTER_TEST(parse_duplicate_arg_name),
    REGISTER_TEST(parse_obj_like_starting_with_bracket),
} TEST_SUITE_END()
