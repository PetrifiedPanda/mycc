#include "frontend/Token.h"
#include "frontend/preproc/PreprocMacro.h"

#include "util/mem.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

static void compare_macro_item(const PreprocMacro* got, const PreprocMacro* ex,
                               const PreprocTokenValList* vals, uint32_t i) {
    const TokenValOrArg* got_item = &got->vals[i];
    const TokenValOrArg* ex_item = &ex->vals[i];
    ASSERT_TOKEN_KIND(got->kinds[i], ex->kinds[i]);
    if (got->kinds[i] == TOKEN_INVALID) {
        ASSERT_UINT(got_item->arg_num, ex_item->arg_num);
    } else {
        const uint32_t got_val_idx = got_item->val_idx;
        const uint32_t ex_val_idx = ex_item->val_idx;
        switch (got->kinds[i]) {
            case TOKEN_IDENTIFIER: {
                const Str got_str = IndexedStringSet_get(&vals->identifiers, got_val_idx);
                const Str ex_str = IndexedStringSet_get(&vals->identifiers, ex_val_idx);
                ASSERT_STR(got_str, ex_str);
                break;
            }
            case TOKEN_I_CONSTANT: {
                const Str got_str = IndexedStringSet_get(&vals->int_consts, got_val_idx);
                const Str ex_str = IndexedStringSet_get(&vals->int_consts, ex_val_idx);
                ASSERT_STR(got_str, ex_str);
                break;
            }
            case TOKEN_F_CONSTANT: {
                const Str got_str = IndexedStringSet_get(&vals->float_consts, got_val_idx);
                const Str ex_str = IndexedStringSet_get(&vals->float_consts, ex_val_idx);
                ASSERT_STR(got_str, ex_str);
                break;
            }
            case TOKEN_STRING_LITERAL: {
                const Str got_str = IndexedStringSet_get(&vals->str_lits, got_val_idx);
                const Str ex_str = IndexedStringSet_get(&vals->str_lits, ex_val_idx);
                ASSERT_STR(got_str, ex_str);
                break;
            }
            default:
                break;
        }
    }
}

static void compare_preproc_macros(const PreprocMacro* got,
                                   const PreprocMacro* ex,
                                   const PreprocTokenValList* vals) {
    ASSERT_BOOL(got->is_func_macro, ex->is_func_macro);
    ASSERT_UINT(got->num_args, ex->num_args);
    ASSERT_BOOL(got->is_variadic, ex->is_variadic);

    ASSERT_UINT(got->expansion_len, ex->expansion_len);

    for (uint32_t i = 0; i < got->expansion_len; ++i) {
        compare_macro_item(got, ex, vals, i);
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

        const Str identifiers[] = {
            STR_LIT("define"),
            STR_LIT("TEST_MACRO"),
        };

        const PreprocInitialStrings initial_strs = {
            .identifiers = identifiers,
            .identifiers_len = ARR_LEN(identifiers),
        };

        uint32_t val_indices[] = {
            UINT32_MAX,
            0,
            1,
        };

        SourceLoc locs[] = {
            {0, {1, 1}},
            {0, {1, 2}},
            {0, {1, 9}},
        };

        static_assert(ARR_LEN(kinds) == ARR_LEN(val_indices), "array lengths don't match");
        static_assert(ARR_LEN(kinds) == ARR_LEN(locs), "array lengths don't match");

        PreprocTokenArr arr = {
            .len = ARR_LEN(kinds),
            .cap = arr.len,
            .kinds = kinds,
            .val_indices = val_indices,
            .locs = locs,
        };
        PreprocTokenValList vals = PreprocTokenValList_create_empty();
        PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 10, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        PreprocMacro ex = {
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .kinds = NULL,
            .vals = NULL,
        };

        compare_preproc_macros(&got, &ex, &vals);
        mycc_free(got.kinds);
        mycc_free(got.vals);

        PreprocTokenValList_free(&vals);
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

        const Str identifiers[] = {
            STR_LIT("define"),
            STR_LIT("ANOTHER_MACRO"),
            STR_LIT("func"),
            STR_LIT("a"),
            STR_LIT("b"),
        };

        const Str int_consts[] = {
            STR_LIT("1"),
            STR_LIT("2"),
            STR_LIT("3"),
        };

        const PreprocInitialStrings initial_strs = {
            .identifiers = identifiers,
            .identifiers_len = ARR_LEN(identifiers),
            .int_consts = int_consts,
            .int_consts_len = ARR_LEN(int_consts),
        };

        uint32_t val_indices[] = {
            UINT32_MAX, // #
            0, // define
            1, // ANOTHER_MACRO
            0, // 1
            UINT32_MAX,
            1, // 2
            UINT32_MAX,
            2, // 3
            UINT32_MAX,
            2, // func
            UINT32_MAX,
            3, // a
            UINT32_MAX,
            4, // b
            UINT32_MAX,
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
        static_assert(TOKENS_LEN == ARR_LEN(val_indices), "array lengths don't match");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "array lengths don't match");
        TokenValOrArg ex_vals[EXPANSION_LEN];
        uint8_t ex_kinds[EXPANSION_LEN];
        for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
            ex_kinds[i] = kinds[i + 3];
            ex_vals[i] = (TokenValOrArg){.val_idx = val_indices[i + 3]};
        }

        PreprocTokenArr arr = {
            .len = TOKENS_LEN,
            .cap = arr.len,
            .kinds = kinds,
            .val_indices = val_indices,
            .locs = locs,
        };
        PreprocTokenValList vals = PreprocTokenValList_create_empty();
        PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 13, &err);

        PreprocMacro ex = {
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .kinds = ex_kinds,
            .vals = ex_vals,
        };

        compare_preproc_macros(&got, &ex, &vals);
        mycc_free(got.kinds);
        mycc_free(got.vals);

        PreprocTokenValList_free(&vals);
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

        const Str identifiers[] = {
            STR_LIT("define"),
            STR_LIT("FUNC_LIKE"),
            STR_LIT("a"),
            STR_LIT("b"),
            STR_LIT("c"),
            STR_LIT("other_name"),
        };

        const Str int_consts[] = {
            STR_LIT("38"),
        };

        const PreprocInitialStrings initial_strs = {
            .identifiers = identifiers,
            .identifiers_len = ARR_LEN(identifiers),
            .int_consts = int_consts,
            .int_consts_len = ARR_LEN(int_consts),
        };

        uint32_t val_indices[] = {
            UINT32_MAX,
            0, // define
            1, // FUNC_LIKE
            UINT32_MAX,
            2, // a
            UINT32_MAX,
            3, // b
            UINT32_MAX,
            4, // c
            UINT32_MAX,
            2, // a
            UINT32_MAX,
            0, // 38
            UINT32_MAX,
            3, // b
            UINT32_MAX,
            5, // other_name
            UINT32_MAX,
            4, // c
            UINT32_MAX,
            2,
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

        static_assert(TOKENS_LEN == ARR_LEN(val_indices), "");
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
            {.val_idx = val_indices[11]},
            {.val_idx = val_indices[12]},
            {.val_idx = val_indices[13]},
            {.arg_num = 1},
            {.val_idx = val_indices[15]},
            {.val_idx = val_indices[16]},
            {.val_idx = val_indices[17]},
            {.arg_num = 2},
            {.val_idx = val_indices[19]},
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

        PreprocTokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .val_indices = val_indices,
            .locs = locs,
        };
        PreprocTokenValList vals = PreprocTokenValList_create_empty();
        PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex, &vals);
        mycc_free(got.kinds);
        mycc_free(got.vals);

        PreprocTokenValList_free(&vals);
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

        const Str identifiers[] = {
            STR_LIT("define"),
            STR_LIT("NO_PARAMS"),
        };

        const Str int_consts[] = {
            STR_LIT("1"),
            STR_LIT("2"),
            STR_LIT("3"),
        };

        const PreprocInitialStrings initial_strs = {
            .identifiers = identifiers,
            .identifiers_len = ARR_LEN(identifiers),
            .int_consts = int_consts,
            .int_consts_len = ARR_LEN(int_consts),
        };

        uint32_t val_indices[] = {
            UINT32_MAX,
            0, // define
            1, // NO_PARAMS
            UINT32_MAX,
            UINT32_MAX,
            0, // 1
            UINT32_MAX,
            1, // 2
            UINT32_MAX,
            2, // 3
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

        static_assert(TOKENS_LEN == ARR_LEN(val_indices), "");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "");
        
        uint8_t ex_kinds[EXPANSION_LEN];
        TokenValOrArg ex_vals[EXPANSION_LEN];
        for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
            ex_kinds[i] = kinds[i + 5];
            ex_vals[i].val_idx = val_indices[i + 5];
        }

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .kinds = ex_kinds,
            .vals = ex_vals,
        };

        PreprocTokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .val_indices = val_indices,
            .locs = locs,
        };
        PreprocTokenValList vals = PreprocTokenValList_create_empty();
        PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex, &vals);
        mycc_free(got.kinds);
        mycc_free(got.vals);

        PreprocTokenValList_free(&vals);
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

        const Str identifiers[] = {
            STR_LIT("define"),
            STR_LIT("NO_PARAMS_EMPTY"),
        };

        const PreprocInitialStrings initial_strs = {
            .identifiers = identifiers,
            .identifiers_len = ARR_LEN(identifiers),
        };

        uint32_t val_indices[] = {
            UINT32_MAX,
            0, // define
            1, // NO_PARAMS_EMPTY
            UINT32_MAX,
            UINT32_MAX,
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
        static_assert(TOKENS_LEN == ARR_LEN(val_indices), "");
        static_assert(TOKENS_LEN == ARR_LEN(locs), "");

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .kinds = NULL,
            .vals = NULL,
        };

        PreprocTokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .val_indices = val_indices,
            .locs = locs,
        };
        PreprocTokenValList vals = PreprocTokenValList_create_empty();
        PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 15, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex, &vals);
        mycc_free(got.kinds);
        mycc_free(got.vals);

        PreprocTokenValList_free(&vals);
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

        const Str identifiers[] = {
            STR_LIT("define"),
            STR_LIT("FUNC_LIKE"),
            STR_LIT("a"),
            STR_LIT("b"),
            STR_LIT("c"),
            STR_LIT("other_name"),
        };

        const Str int_consts[] = {
            STR_LIT("38"),
        };

        const PreprocInitialStrings initial_strs = {
            .identifiers = identifiers,
            .identifiers_len = ARR_LEN(identifiers),
            .int_consts = int_consts,
            .int_consts_len = ARR_LEN(int_consts),
        };

        uint32_t val_indices[] = {
            UINT32_MAX,
            0, // define
            1, // FUNC_LIKE
            UINT32_MAX,
            2, // a,
            UINT32_MAX,
            3, // b
            UINT32_MAX,
            4, // c
            UINT32_MAX,
            UINT32_MAX,
            UINT32_MAX,
            2, // a
            UINT32_MAX,
            0, // 38
            UINT32_MAX,
            3, // b
            UINT32_MAX,
            5, // other_name
            UINT32_MAX,
            4, // c
            UINT32_MAX,
            2, // a
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

        static_assert(TOKENS_LEN == ARR_LEN(val_indices), "");
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
            {.val_idx = val_indices[13]},
            {.val_idx = val_indices[14]},
            {.val_idx = val_indices[15]},
            {.arg_num = 1},
            {.val_idx = val_indices[17]},
            {.val_idx = val_indices[18]},
            {.val_idx = val_indices[19]},
            {.arg_num = 2},
            {.val_idx = val_indices[21]},
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

        PreprocTokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .val_indices = val_indices,
            .locs = locs,
        };
        PreprocTokenValList vals = PreprocTokenValList_create_empty();
        PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex, &vals);
        mycc_free(got.kinds);
        mycc_free(got.vals);

        PreprocTokenValList_free(&vals);
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

        const Str identifiers[] = {
            STR_LIT("define"),
            STR_LIT("FUNC_LIKE"),
            STR_LIT("a"),
            STR_LIT("b"),
            STR_LIT("c"),
            STR_LIT("other_name"),
            STR_LIT("__VA_ARGS__"),
        };

        const Str int_consts[] = {
            STR_LIT("38"),
        };

        const PreprocInitialStrings initial_strs = {
            .identifiers = identifiers,
            .identifiers_len = ARR_LEN(identifiers),
            .int_consts = int_consts,
            .int_consts_len = ARR_LEN(int_consts),
        };

        uint32_t val_indices[] = {
            UINT32_MAX,
            0, // define
            1, // FUNC_LIKE
            UINT32_MAX,
            2, // a
            UINT32_MAX,
            3, // b
            UINT32_MAX,
            4, // c
            UINT32_MAX,
            UINT32_MAX,
            UINT32_MAX,
            2, // a
            UINT32_MAX,
            0, // 38
            UINT32_MAX,
            3, // b
            UINT32_MAX,
            5, // other_name
            UINT32_MAX,
            6, // __VA_ARGS__
            UINT32_MAX,
            UINT32_MAX,
            4, // c
            UINT32_MAX,
            2, // a
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

        static_assert(TOKENS_LEN == ARR_LEN(val_indices), "");
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
            {.val_idx = val_indices[13]},
            {.val_idx = val_indices[14]},
            {.val_idx = val_indices[15]},
            {.arg_num = 1},
            {.val_idx = val_indices[17]},
            {.val_idx = val_indices[18]},
            {.val_idx = val_indices[19]},
            {.arg_num = 3},
            {.val_idx = val_indices[21]},
            {.val_idx = val_indices[22]},
            {.arg_num = 2},
            {.val_idx = val_indices[24]},
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

        PreprocTokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .kinds = kinds,
            .val_indices = val_indices,
            .locs = locs,
        };
        PreprocTokenValList vals = PreprocTokenValList_create_empty();
        PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex, &vals);
        mycc_free(got.kinds);
        mycc_free(got.vals);

        PreprocTokenValList_free(&vals);
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

    const Str identifiers[] = {
        STR_LIT("define"),
        STR_LIT("FUNC_LIKE"),
        STR_LIT("a"),
        STR_LIT("b"),
        STR_LIT("c"),
        STR_LIT("other_name"),
        STR_LIT("__VA_ARGS__"),
    };

    const Str int_consts[] = {
        STR_LIT("38"),
    };

    const PreprocInitialStrings initial_strs = {
        .identifiers = identifiers,
        .identifiers_len = ARR_LEN(identifiers),
        .int_consts = int_consts,
        .int_consts_len = ARR_LEN(int_consts),
    };

    uint32_t val_indices[] = {
        UINT32_MAX,
        0, // define
        1, // FUNC_LIKE
        UINT32_MAX,
        2, // a
        UINT32_MAX,
        3, // b
        UINT32_MAX,
        4, // c
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        2, // a
        UINT32_MAX,
        0, // 38
        UINT32_MAX,
        3, // b
        UINT32_MAX,
        5, // other_name
        UINT32_MAX,
        6, // __VA_ARGS__
        UINT32_MAX,
        UINT32_MAX,
        4, // c
        UINT32_MAX,
        2, // a
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

    static_assert(TOKENS_LEN == ARR_LEN(val_indices), "");
    static_assert(TOKENS_LEN == ARR_LEN(locs), "");
    PreprocTokenArr arr = {
        .len = TOKENS_LEN,
        .cap = TOKENS_LEN,
        .kinds = kinds,
        .val_indices = val_indices,
        .locs = locs,
    };
    PreprocTokenValList vals = PreprocTokenValList_create_empty();
    PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);

    // change c to a
    val_indices[8] = 2;
    {
        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 9, &err);
        is_zeroed_macro(&got);
        ASSERT(err.kind == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(err.duplicate_arg_name, STR_LIT("a"));
        ASSERT_UINT(err.base.loc.file_idx, 0);
        ASSERT_UINT(err.base.loc.file_loc.line, 1);
        ASSERT_UINT(err.base.loc.file_loc.index, 25);
    }

    // change a back to c
    val_indices[8] = 4;
    // change b to c
    val_indices[6] = 4;
    {
        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 9, &err);
        is_zeroed_macro(&got);
        ASSERT(err.kind == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(err.duplicate_arg_name, STR_LIT("c"));
        ASSERT_UINT(err.base.loc.file_idx, 0);
        ASSERT_UINT(err.base.loc.file_loc.line, 1);
        ASSERT_UINT(err.base.loc.file_loc.index, 25);
    }
    // change c to a
    val_indices[6] = 2;
    {
        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, &vals, 9, &err);
        is_zeroed_macro(&got);
        ASSERT(err.kind == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(err.duplicate_arg_name, STR_LIT("a"));
        ASSERT_UINT(err.base.loc.file_idx, 0);
        ASSERT_UINT(err.base.loc.file_loc.line, 1);
        ASSERT_UINT(err.base.loc.file_loc.index, 22);
    }
    PreprocTokenValList_free(&vals);
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

    const Str identifiers[] = {
        STR_LIT("define"),
    };

    const Str int_consts[] = {
        STR_LIT("0"),
    };

    const PreprocInitialStrings initial_strs = {
        .identifiers = identifiers,
        .identifiers_len = ARR_LEN(identifiers),
        .int_consts = int_consts,
        .int_consts_len = ARR_LEN(int_consts),
    };

    uint32_t val_indices[] = {
        UINT32_MAX,
        0, // define
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        0, // 0
        UINT32_MAX,
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
    static_assert(TOKENS_LEN == ARR_LEN(val_indices), "");
    static_assert(TOKENS_LEN == ARR_LEN(locs), "");

    uint8_t ex_kinds[EXPANSION_LEN];
    TokenValOrArg ex_vals[EXPANSION_LEN];
    for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
        ex_kinds[i] = kinds[i + 3];
        ex_vals[i].val_idx = val_indices[i + 3];
    }

    PreprocTokenArr arr = {
        .len = TOKENS_LEN,
        .cap = arr.len,
        .kinds = kinds,
        .val_indices = val_indices,
        .locs = locs,
    };
    PreprocTokenValList vals = PreprocTokenValList_create_empty();
    PreprocTokenValList_insert_initial_strings(&vals, &initial_strs);
    
    PreprocErr err = PreprocErr_create();
    PreprocMacro got = parse_preproc_macro(&arr, &vals, 4, &err);

    PreprocMacro ex = {
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = EXPANSION_LEN,
        .kinds = ex_kinds,
        .vals = ex_vals,
    };

    compare_preproc_macros(&got, &ex, &vals);
    mycc_free(got.kinds);
    mycc_free(got.vals);

    PreprocTokenValList_free(&vals);
}

TEST_SUITE_BEGIN(preproc_macro_parser){
    REGISTER_TEST(parse_obj_like),
    REGISTER_TEST(parse_func_like),
    REGISTER_TEST(parse_variadic),
    REGISTER_TEST(parse_duplicate_arg_name),
    REGISTER_TEST(parse_obj_like_starting_with_bracket),
} TEST_SUITE_END()
