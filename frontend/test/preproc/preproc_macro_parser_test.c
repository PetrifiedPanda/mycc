#include "frontend/preproc/preproc_macro.h"

#include "util/mem.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

static void compare_preproc_macros(const struct preproc_macro* got,
                                   const struct preproc_macro* ex) {
    ASSERT_BOOL(got->is_func_macro, ex->is_func_macro);
    ASSERT_SIZE_T(got->num_args, ex->num_args);
    ASSERT_BOOL(got->is_variadic, ex->is_variadic);

    ASSERT_SIZE_T(got->expansion_len, ex->expansion_len);

    for (size_t i = 0; i < got->expansion_len; ++i) {
        const struct token_or_arg* got_item = &got->expansion[i];
        const struct token_or_arg* ex_item = &ex->expansion[i];
        ASSERT_BOOL(got_item->is_arg, ex_item->is_arg);

        if (got_item->is_arg) {
            ASSERT_SIZE_T(got_item->arg_num, ex_item->arg_num);
        } else {
            const struct token* got_tok = &got_item->token;
            const struct token* ex_tok = &ex_item->token;

            ASSERT_TOKEN_TYPE(got_tok->type, ex_tok->type);
            ASSERT_STR(str_get_data(&got_tok->spelling),
                       str_get_data(&ex_tok->spelling));

            ASSERT_SIZE_T(got_tok->loc.file_idx, ex_tok->loc.file_idx);
            ASSERT_SIZE_T(got_tok->loc.file_loc.line,
                          ex_tok->loc.file_loc.line);
            ASSERT_SIZE_T(got_tok->loc.file_loc.index,
                          ex_tok->loc.file_loc.index);
        }
    }
}

TEST(object_like) {
    {
        // #define TEST_MACRO
        struct token tokens[] = {
            {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("TEST_MACRO"), {0, {1, 9}}},
        };

        struct token_arr arr = {
            .len = ARR_LEN(tokens),
            .cap = arr.len,
            .tokens = tokens,
        };

        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        struct preproc_macro ex = {
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
        struct token tokens[] = {
            {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
            {IDENTIFIER,
             .spelling = STR_NON_HEAP("ANOTHER_MACRO"),
             {0, {1, 9}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("1"), {0, {1, 23}}},
            {ADD, .spelling = create_null_str(), {0, {1, 25}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("2"), {0, {1, 27}}},
            {ASTERISK, .spelling = create_null_str(), {0, {1, 29}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("3"), {0, {1, 31}}},
            {SUB, .spelling = create_null_str(), {0, {1, 33}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("func"), {0, {1, 35}}},
            {LBRACKET, .spelling = create_null_str(), {0, {1, 39}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 40}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 41}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 43}}},
            {RBRACKET, .spelling = create_null_str(), {0, {1, 44}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 3
        };
        struct token_or_arg expansion[EXPANSION_LEN];
        for (size_t i = 0; i < EXPANSION_LEN; ++i) {
            expansion[i] = (struct token_or_arg){
                .is_arg = false,
                .token = tokens[i + 3],
            };
        }

        struct token_arr arr = {
            .len = TOKENS_LEN,
            .cap = arr.len,
            .tokens = tokens,
        };

        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);

        struct preproc_macro ex = {
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

TEST(func_like) {
    {
        // #define FUNC_LIKE(a, b, c) a != 38 ? b * other_name : c + a
        struct token tokens[] = {
            {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("FUNC_LIKE"), {0, {1, 9}}},
            {LBRACKET, .spelling = create_null_str(), {0, {1, 18}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 19}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 20}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 22}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 23}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 25}}},
            {RBRACKET, .spelling = create_null_str(), {0, {1, 26}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 28}}},
            {NE_OP, .spelling = create_null_str(), {0, {1, 30}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("38"), {0, {1, 33}}},
            {QMARK, .spelling = create_null_str(), {0, {1, 36}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 38}}},
            {ASTERISK, .spelling = create_null_str(), {0, {1, 40}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("other_name"), {0, {1, 42}}},
            {COLON, .spelling = create_null_str(), {0, {1, 53}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 55}}},
            {ADD, .spelling = create_null_str(), {0, {1, 57}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 59}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 10
        };

        struct token_or_arg expansion[EXPANSION_LEN] = {
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

        struct preproc_macro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        struct token_arr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
    {
        // #define NO_PARAMS() 1 + 2 + 3
        struct token tokens[] = {
            {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("NO_PARAMS"), {0, {1, 9}}},
            {LBRACKET, .spelling = create_null_str(), {0, {1, 18}}},
            {RBRACKET, .spelling = create_null_str(), {0, {1, 19}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("1"), {0, {1, 21}}},
            {ADD, .spelling = create_null_str(), {0, {1, 23}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("2"), {0, {1, 25}}},
            {ADD, .spelling = create_null_str(), {0, {1, 27}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("3"), {0, {1, 29}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 5
        };

        struct token_or_arg expansion[EXPANSION_LEN];
        for (size_t i = 0; i < EXPANSION_LEN; ++i) {
            expansion[i] = (struct token_or_arg){
                .is_arg = false,
                .token = tokens[i + 5],
            };
        }

        struct preproc_macro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        struct token_arr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
    {
        // #define NO_PARAMS_EMPTY()
        struct token tokens[] = {
            {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
            {IDENTIFIER,
             .spelling = STR_NON_HEAP("NO_PARAMS_EMPTY"),
             {0, {1, 9}}},
            {LBRACKET, .spelling = create_null_str(), {0, {1, 24}}},
            {RBRACKET, .spelling = create_null_str(), {0, {1, 25}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
        };

        struct preproc_macro ex = {
            .is_func_macro = true,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .expansion = NULL,
        };

        struct token_arr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
}

TEST(variadic) {
    {
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name : c + a
        struct token tokens[] = {
            {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("FUNC_LIKE"), {0, {1, 9}}},
            {LBRACKET, .spelling = create_null_str(), {0, {1, 18}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 19}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 20}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 22}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 23}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 25}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 26}}},
            {ELLIPSIS, .spelling = create_null_str(), {0, {1, 28}}},
            {RBRACKET, .spelling = create_null_str(), {0, {1, 31}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 33}}},
            {NE_OP, .spelling = create_null_str(), {0, {1, 35}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("38"), {0, {1, 38}}},
            {QMARK, .spelling = create_null_str(), {0, {1, 41}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 43}}},
            {ASTERISK, .spelling = create_null_str(), {0, {1, 45}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("other_name"), {0, {1, 47}}},
            {COLON, .spelling = create_null_str(), {0, {1, 58}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 60}}},
            {ADD, .spelling = create_null_str(), {0, {1, 62}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 64}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 12,
        };

        struct token_or_arg expansion[EXPANSION_LEN] = {
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

        struct preproc_macro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = true,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        struct token_arr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
    {
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name(__VA_ARGS__)
        // : c + a
        struct token tokens[] = {
            {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("FUNC_LIKE"), {0, {1, 9}}},
            {LBRACKET, .spelling = create_null_str(), {0, {1, 18}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 19}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 20}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 22}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 23}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 25}}},
            {COMMA, .spelling = create_null_str(), {0, {1, 26}}},
            {ELLIPSIS, .spelling = create_null_str(), {0, {1, 28}}},
            {RBRACKET, .spelling = create_null_str(), {0, {1, 31}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 33}}},
            {NE_OP, .spelling = create_null_str(), {0, {1, 35}}},
            {I_CONSTANT, .spelling = STR_NON_HEAP("38"), {0, {1, 38}}},
            {QMARK, .spelling = create_null_str(), {0, {1, 41}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 43}}},
            {ASTERISK, .spelling = create_null_str(), {0, {1, 45}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("other_name"), {0, {1, 47}}},
            {LBRACKET, .spelling = create_null_str(), {0, {1, 48}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("__VA_ARGS__"), {0, {1, 49}}},
            {RBRACKET, .spelling = create_null_str(), {0, {1, 60}}},
            {COLON, .spelling = create_null_str(), {0, {1, 62}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 64}}},
            {ADD, .spelling = create_null_str(), {0, {1, 66}}},
            {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 68}}},
        };

        enum {
            TOKENS_LEN = ARR_LEN(tokens),
            EXPANSION_LEN = TOKENS_LEN - 12,
        };

        struct token_or_arg expansion[EXPANSION_LEN] = {
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

        struct preproc_macro ex = {
            .is_func_macro = true,
            .num_args = 3,
            .is_variadic = true,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        struct token_arr arr = {
            .len = TOKENS_LEN,
            .cap = TOKENS_LEN,
            .tokens = tokens,
        };

        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        mycc_free(got.expansion);
    }
}

static void is_zeroed_macro(const struct preproc_macro* got) {
    ASSERT_BOOL(got->is_func_macro, false);
    ASSERT_BOOL(got->is_variadic, false);
    ASSERT_SIZE_T(got->num_args, 0);
    ASSERT_SIZE_T(got->expansion_len, 0);
    ASSERT_NULL(got->expansion);
}

TEST(duplicate_arg_name) {
    // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name(__VA_ARGS__)
    // : c + a
    struct token tokens[] = {
        {STRINGIFY_OP, .spelling = create_null_str(), {0, {1, 1}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("define"), {0, {1, 2}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("FUNC_LIKE"), {0, {1, 9}}},
        {LBRACKET, .spelling = create_null_str(), {0, {1, 18}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 19}}},
        {COMMA, .spelling = create_null_str(), {0, {1, 20}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 22}}},
        {COMMA, .spelling = create_null_str(), {0, {1, 23}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 25}}},
        {COMMA, .spelling = create_null_str(), {0, {1, 26}}},
        {ELLIPSIS, .spelling = create_null_str(), {0, {1, 28}}},
        {RBRACKET, .spelling = create_null_str(), {0, {1, 31}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 33}}},
        {NE_OP, .spelling = create_null_str(), {0, {1, 35}}},
        {I_CONSTANT, .spelling = STR_NON_HEAP("38"), {0, {1, 38}}},
        {QMARK, .spelling = create_null_str(), {0, {1, 41}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("b"), {0, {1, 43}}},
        {ASTERISK, .spelling = create_null_str(), {0, {1, 45}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("other_name"), {0, {1, 47}}},
        {LBRACKET, .spelling = create_null_str(), {0, {1, 48}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("__VA_ARGS__"), {0, {1, 49}}},
        {RBRACKET, .spelling = create_null_str(), {0, {1, 60}}},
        {COLON, .spelling = create_null_str(), {0, {1, 62}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("c"), {0, {1, 64}}},
        {ADD, .spelling = create_null_str(), {0, {1, 66}}},
        {IDENTIFIER, .spelling = STR_NON_HEAP("a"), {0, {1, 68}}},
    };
    enum {
        TOKENS_LEN = ARR_LEN(tokens),
    };
    struct token_arr arr = {
        .len = TOKENS_LEN,
        .cap = TOKENS_LEN,
        .tokens = tokens,
    };
    // change c to a
    tokens[8].spelling = STR_NON_HEAP("a");
    {
        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        is_zeroed_macro(&got);
        ASSERT(err.type == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(str_get_data(&err.duplicate_arg_name), "a");
        ASSERT_SIZE_T(err.base.loc.file_idx, 0);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, 1);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, 25);
    }

    tokens[8].spelling = STR_NON_HEAP("c");
    tokens[6].spelling = STR_NON_HEAP("c");
    {
        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        is_zeroed_macro(&got);
        ASSERT(err.type == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(str_get_data(&err.duplicate_arg_name), "c");
        ASSERT_SIZE_T(err.base.loc.file_idx, 0);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, 1);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, 25);
    }
    tokens[6].spelling = STR_NON_HEAP("a");
    {
        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, &err);
        is_zeroed_macro(&got);
        ASSERT(err.type == PREPROC_ERR_DUPLICATE_MACRO_PARAM);
        ASSERT_STR(str_get_data(&err.duplicate_arg_name), "a");
        ASSERT_SIZE_T(err.base.loc.file_idx, 0);
        ASSERT_SIZE_T(err.base.loc.file_loc.line, 1);
        ASSERT_SIZE_T(err.base.loc.file_loc.index, 22);
    }
}

TEST_SUITE_BEGIN(preproc_macro_parser, 4) {
    REGISTER_TEST(object_like);
    REGISTER_TEST(func_like);
    REGISTER_TEST(variadic);
    REGISTER_TEST(duplicate_arg_name);
}
TEST_SUITE_END()
