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
        const TokenOrArg* got_item = &got->expansion[i];
        const TokenOrArg* ex_item = &ex->expansion[i];
        ASSERT_BOOL(got_item->is_arg, ex_item->is_arg);

        if (got_item->is_arg) {
            ASSERT_UINT(got_item->arg_num, ex_item->arg_num);
        } else {
            const Token* got_tok = &got_item->token;
            const Token* ex_tok = &ex_item->token;

            ASSERT_TOKEN_KIND(got_tok->kind, ex_tok->kind);
            ASSERT_STR(StrBuf_as_str(&got_tok->spelling),
                       StrBuf_as_str(&ex_tok->spelling));

            ASSERT_UINT(got_tok->loc.file_idx, ex_tok->loc.file_idx);
            ASSERT_UINT(got_tok->loc.file_loc.line,
                          ex_tok->loc.file_loc.line);
            ASSERT_UINT(got_tok->loc.file_loc.index,
                          ex_tok->loc.file_loc.index);
        }
    }
}

TEST(parse_obj_like) {
    {
        // #define TEST_MACRO
        Token tokens[] = {
            {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("define"),
             {0, {1, 2}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("TEST_MACRO"),
             {0, {1, 9}}},
        };

        TokenArr arr = {
            .len = ARR_LEN(tokens),
            .cap = arr.len,
            .tokens = tokens,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 10, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        PreprocMacro ex = {
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .expansion = NULL,
        };

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
    {
        // #define ANOTHER_MACRO 1 + 2 * 3 - func(a, b)
        Token tokens[] = {
            {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("define"),
             {0, {1, 2}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("ANOTHER_MACRO"),
             {0, {1, 9}}},
            {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("1"), {0, {1, 23}}},
            {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 25}}},
            {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("2"), {0, {1, 27}}},
            {TOKEN_ASTERISK, .spelling = StrBuf_null(), {0, {1, 29}}},
            {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("3"), {0, {1, 31}}},
            {TOKEN_SUB, .spelling = StrBuf_null(), {0, {1, 33}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("func"),
             {0, {1, 35}}},
            {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 39}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 40}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 41}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 43}}},
            {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 44}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 3
        };
        TokenOrArg expansion[EXPANSION_LEN];
        for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
            expansion[i] = (TokenOrArg){
                .is_arg = false,
                .token = tokens[i + 3],
            };
        }

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = arr.len,
            .tokens = tokens,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 13, &err);

        PreprocMacro ex = {
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
}

TEST(parse_func_like) {
    {
        // #define FUNC_LIKE(a, b, c) a != 38 ? b * other_name : c + a
        Token tokens[] = {
            {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("define"),
             {0, {1, 2}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("FUNC_LIKE"),
             {0, {1, 9}}},
            {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 18}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 19}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 20}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 22}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 23}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 25}}},
            {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 26}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 28}}},
            {TOKEN_NE, .spelling = StrBuf_null(), {0, {1, 30}}},
            {TOKEN_I_CONSTANT,
             .spelling = STR_BUF_NON_HEAP("38"),
             {0, {1, 33}}},
            {TOKEN_QMARK, .spelling = StrBuf_null(), {0, {1, 36}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 38}}},
            {TOKEN_ASTERISK, .spelling = StrBuf_null(), {0, {1, 40}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("other_name"),
             {0, {1, 42}}},
            {TOKEN_COLON, .spelling = StrBuf_null(), {0, {1, 53}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 55}}},
            {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 57}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 59}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 10
        };

        TokenOrArg expansion[EXPANSION_LEN] = {
            {.is_arg = true, .arg_num = 0},
            {.is_arg = false, .token = tokens[11]},
            {.is_arg = false, .token = tokens[12]},
            {.is_arg = false, .token = tokens[13]},
            {.is_arg = true, .arg_num = 1},
            {.is_arg = false, .token = tokens[15]},
            {.is_arg = false, .token = tokens[16]},
            {.is_arg = false, .token = tokens[17]},
            {.is_arg = true, .arg_num = 2},
            {.is_arg = false, .token = tokens[19]},
            {.is_arg = true, .arg_num = 0},
        };

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
    {
        // #define NO_PARAMS() 1 + 2 + 3
        Token tokens[] = {
            {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("define"),
             {0, {1, 2}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("NO_PARAMS"),
             {0, {1, 9}}},
            {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 18}}},
            {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 19}}},
            {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("1"), {0, {1, 21}}},
            {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 23}}},
            {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("2"), {0, {1, 25}}},
            {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 27}}},
            {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("3"), {0, {1, 29}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 5
        };

        TokenOrArg expansion[EXPANSION_LEN];
        for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
            expansion[i] = (TokenOrArg){
                .is_arg = false,
                .token = tokens[i + 5],
            };
        }

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
    {
        // #define NO_PARAMS_EMPTY()
        Token tokens[] = {
            {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("define"),
             {0, {1, 2}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("NO_PARAMS_EMPTY"),
             {0, {1, 9}}},
            {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 24}}},
            {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 25}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
        };

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .expansion = NULL,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 15, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
}

TEST(parse_variadic) {
    {
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name : c + a
        Token tokens[] = {
            {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("define"),
             {0, {1, 2}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("FUNC_LIKE"),
             {0, {1, 9}}},
            {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 18}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 19}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 20}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 22}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 23}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 25}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 26}}},
            {TOKEN_ELLIPSIS, .spelling = StrBuf_null(), {0, {1, 28}}},
            {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 31}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 33}}},
            {TOKEN_NE, .spelling = StrBuf_null(), {0, {1, 35}}},
            {TOKEN_I_CONSTANT,
             .spelling = STR_BUF_NON_HEAP("38"),
             {0, {1, 38}}},
            {TOKEN_QMARK, .spelling = StrBuf_null(), {0, {1, 41}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 43}}},
            {TOKEN_ASTERISK, .spelling = StrBuf_null(), {0, {1, 45}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("other_name"),
             {0, {1, 47}}},
            {TOKEN_COLON, .spelling = StrBuf_null(), {0, {1, 58}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 60}}},
            {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 62}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 64}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 12,
        };

        TokenOrArg expansion[EXPANSION_LEN] = {
            {.is_arg = true, .arg_num = 0},
            {.is_arg = false, .token = tokens[13]},
            {.is_arg = false, .token = tokens[14]},
            {.is_arg = false, .token = tokens[15]},
            {.is_arg = true, .arg_num = 1},
            {.is_arg = false, .token = tokens[17]},
            {.is_arg = false, .token = tokens[18]},
            {.is_arg = false, .token = tokens[19]},
            {.is_arg = true, .arg_num = 2},
            {.is_arg = false, .token = tokens[21]},
            {.is_arg = true, .arg_num = 0},
        };

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = true,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
    {
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name(__VA_ARGS__)
        // : c + a
        Token tokens[] = {
            {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("define"),
             {0, {1, 2}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("FUNC_LIKE"),
             {0, {1, 9}}},
            {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 18}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 19}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 20}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 22}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 23}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 25}}},
            {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 26}}},
            {TOKEN_ELLIPSIS, .spelling = StrBuf_null(), {0, {1, 28}}},
            {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 31}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 33}}},
            {TOKEN_NE, .spelling = StrBuf_null(), {0, {1, 35}}},
            {TOKEN_I_CONSTANT,
             .spelling = STR_BUF_NON_HEAP("38"),
             {0, {1, 38}}},
            {TOKEN_QMARK, .spelling = StrBuf_null(), {0, {1, 41}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 43}}},
            {TOKEN_ASTERISK, .spelling = StrBuf_null(), {0, {1, 45}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("other_name"),
             {0, {1, 47}}},
            {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 48}}},
            {TOKEN_IDENTIFIER,
             .spelling = STR_BUF_NON_HEAP("__VA_ARGS__"),
             {0, {1, 49}}},
            {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 60}}},
            {TOKEN_COLON, .spelling = StrBuf_null(), {0, {1, 62}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 64}}},
            {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 66}}},
            {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 68}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 12,
        };

        TokenOrArg expansion[EXPANSION_LEN] = {
            {.is_arg = true, .arg_num = 0},
            {.is_arg = false, .token = tokens[13]},
            {.is_arg = false, .token = tokens[14]},
            {.is_arg = false, .token = tokens[15]},
            {.is_arg = true, .arg_num = 1},
            {.is_arg = false, .token = tokens[17]},
            {.is_arg = false, .token = tokens[18]},
            {.is_arg = false, .token = tokens[19]},
            {.is_arg = true, .arg_num = 3},
            {.is_arg = false, .token = tokens[21]},
            {.is_arg = false, .token = tokens[22]},
            {.is_arg = true, .arg_num = 2},
            {.is_arg = false, .token = tokens[24]},
            {.is_arg = true, .arg_num = 0},
        };

        PreprocMacro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = true,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        TokenArr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        PreprocErr err = PreprocErr_create();
        PreprocMacro got = parse_preproc_macro(&arr, 9, &err);
        ASSERT(err.kind == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
}

static void is_zeroed_macro(const PreprocMacro* got) {
    ASSERT_BOOL(got->is_func_macro, false);
    ASSERT_BOOL(got->is_variadic, false);
    ASSERT_UINT(got->num_args, 0);
    ASSERT_UINT(got->expansion_len, 0);
    ASSERT_NULL(got->expansion);
}

TEST(parse_duplicate_arg_name) {
    // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name(__VA_ARGS__)
    // : c + a
    Token tokens[] = {
        {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("define"), {0, {1, 2}}},
        {TOKEN_IDENTIFIER,
         .spelling = STR_BUF_NON_HEAP("FUNC_LIKE"),
         {0, {1, 9}}},
        {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 18}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 19}}},
        {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 20}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 22}}},
        {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 23}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 25}}},
        {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 26}}},
        {TOKEN_ELLIPSIS, .spelling = StrBuf_null(), {0, {1, 28}}},
        {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 31}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 33}}},
        {TOKEN_NE, .spelling = StrBuf_null(), {0, {1, 35}}},
        {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("38"), {0, {1, 38}}},
        {TOKEN_QMARK, .spelling = StrBuf_null(), {0, {1, 41}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("b"), {0, {1, 43}}},
        {TOKEN_ASTERISK, .spelling = StrBuf_null(), {0, {1, 45}}},
        {TOKEN_IDENTIFIER,
         .spelling = STR_BUF_NON_HEAP("other_name"),
         {0, {1, 47}}},
        {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 48}}},
        {TOKEN_IDENTIFIER,
         .spelling = STR_BUF_NON_HEAP("__VA_ARGS__"),
         {0, {1, 49}}},
        {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 60}}},
        {TOKEN_COLON, .spelling = StrBuf_null(), {0, {1, 62}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("c"), {0, {1, 64}}},
        {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 66}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("a"), {0, {1, 68}}},
    };
    enum {
        TOKENS_LEN = ARR_LEN(tokens),
    };
    TokenArr arr = {
        .len = TOKENS_LEN,
        .cap = TOKENS_LEN,
        .tokens = tokens,
    };
    // change c to a
    tokens[8].spelling = STR_BUF_NON_HEAP("a");
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

    tokens[8].spelling = STR_BUF_NON_HEAP("c");
    tokens[6].spelling = STR_BUF_NON_HEAP("c");
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
    tokens[6].spelling = STR_BUF_NON_HEAP("a");
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
    Token tokens[] = {
        {TOKEN_PP_STRINGIFY, .spelling = StrBuf_null(), {0, {1, 1}}},
        {TOKEN_IDENTIFIER, .spelling = STR_BUF_NON_HEAP("define"), {0, {1, 2}}},
        {TOKEN_IDENTIFIER, .spelling = StrBuf_null(), {0, {1, 9}}},
        {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 15}}},
        {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 16}}},
        {TOKEN_VOID, .spelling = StrBuf_null(), {0, {1, 17}}},
        {TOKEN_VOID, .spelling = StrBuf_null(), {0, {1, 17}}},
        {TOKEN_ASTERISK, .spelling = StrBuf_null(), {0, {1, 21}}},
        {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 22}}},
        {TOKEN_I_CONSTANT, .spelling = STR_BUF_NON_HEAP("0"), {0, {1, 23}}},
        {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 24}}},
    };

    enum {
        TOKENS_LEN = ARR_LEN(tokens),
        EXPANSION_LEN = TOKENS_LEN - 3,
    };

    TokenOrArg expansion[EXPANSION_LEN];
    for (uint32_t i = 0; i < EXPANSION_LEN; ++i) {
        expansion[i] = (TokenOrArg){
            .is_arg = false,
            .token = tokens[i + 3],
        };
    }

    TokenArr arr = {
        .len = TOKENS_LEN,
        .cap = arr.len,
        .tokens = tokens,
    };
    
    PreprocErr err = PreprocErr_create();
    PreprocMacro got = parse_preproc_macro(&arr, 4, &err);

    PreprocMacro ex = {
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = EXPANSION_LEN,
        .expansion = expansion,
    };

    compare_preproc_macros(&got, &ex);
    mycc_free(got.expansion);
}

TEST_SUITE_BEGIN(preproc_macro_parser){
    REGISTER_TEST(parse_obj_like),
    REGISTER_TEST(parse_func_like),
    REGISTER_TEST(parse_variadic),
    REGISTER_TEST(parse_duplicate_arg_name),
    REGISTER_TEST(parse_obj_like_starting_with_bracket),
} TEST_SUITE_END()
