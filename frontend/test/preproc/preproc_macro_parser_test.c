#include "frontend/preproc/preproc_macro.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

static void compare_preproc_macros(const struct preproc_macro* got,
                                   const struct preproc_macro* ex) {
    ASSERT(ex->spell == got->spell);
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
            ASSERT_STR(got_tok->spelling, ex_tok->spelling);

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
        const char* spell = "TEST_MACRO";
        // #define TEST_MACRO
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {0, {1, 1}}},
            {IDENTIFIER, "define", {0, {1, 2}}},
            {IDENTIFIER, "TEST_MACRO", {0, {1, 9}}},
        };

        struct token_arr arr = {
            .len = sizeof tokens / sizeof(struct token),
            .cap = arr.len,
            .tokens = tokens,
        };
        
        struct preproc_err err = create_preproc_err();
        struct preproc_macro got = parse_preproc_macro(&arr, spell, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        struct preproc_macro ex = {
            .spell = spell,
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = 0,
            .expansion = NULL,
        };

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
    {
        const char* spell = "ANOTHER_MACRO";
        // #define ANOTHER_MACRO 1 + 2 * 3 - func(a, b)
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {0, {1, 1}}},
            {IDENTIFIER, "define", {0, {1, 2}}},
            {IDENTIFIER, "ANOTHER_MACRO", {0, {1, 9}}},
            {I_CONSTANT, "1", {0, {1, 23}}},
            {ADD, NULL, {0, {1, 25}}},
            {I_CONSTANT, "2", {0, {1, 27}}},
            {ASTERISK, NULL, {0, {1, 29}}},
            {I_CONSTANT, "3", {0, {1, 31}}},
            {SUB, NULL, {0, {1, 33}}},
            {IDENTIFIER, "func", {0, {1, 35}}},
            {LBRACKET, NULL, {0, {1, 39}}},
            {IDENTIFIER, "a", {0, {1, 40}}},
            {COMMA, NULL, {0, {1, 41}}},
            {IDENTIFIER, "b", {0, {1, 43}}},
            {RBRACKET, NULL, {0, {1, 44}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
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
        struct preproc_macro got = parse_preproc_macro(&arr, spell, &err);

        struct preproc_macro ex = {
            .spell = spell,
            .is_func_macro = false,
            .num_args = 0,
            .is_variadic = false,

            .expansion_len = EXPANSION_LEN,
            .expansion = expansion,
        };

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
}

TEST(func_like) {
    {
        const char* spell = "FUNC_LIKE";
        // #define FUNC_LIKE(a, b, c) a != 38 ? b * other_name : c + a
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {0, {1, 1}}},
            {IDENTIFIER, "define", {0, {1, 2}}},
            {IDENTIFIER, "FUNC_LIKE", {0, {1, 9}}},
            {LBRACKET, NULL, {0, {1, 18}}},
            {IDENTIFIER, "a", {0, {1, 19}}},
            {COMMA, NULL, {0, {1, 20}}},
            {IDENTIFIER, "b", {0, {1, 22}}},
            {COMMA, NULL, {0, {1, 23}}},
            {IDENTIFIER, "c", {0, {1, 25}}},
            {RBRACKET, NULL, {0, {1, 26}}},
            {IDENTIFIER, "a", {0, {1, 28}}},
            {NE_OP, NULL, {0, {1, 30}}},
            {I_CONSTANT, "38", {0, {1, 33}}},
            {QMARK, NULL, {0, {1, 36}}},
            {IDENTIFIER, "b", {0, {1, 38}}},
            {ASTERISK, NULL, {0, {1, 40}}},
            {IDENTIFIER, "other_name", {0, {1, 42}}},
            {COLON, NULL, {0, {1, 53}}},
            {IDENTIFIER, "c", {0, {1, 55}}},
            {ADD, NULL, {0, {1, 57}}},
            {IDENTIFIER, "a", {0, {1, 59}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
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
            .spell = spell,
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
        struct preproc_macro got = parse_preproc_macro(&arr, spell, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
    {
        const char* spell = "NO_PARAMS";
        // #define NO_PARAMS() 1 + 2 + 3
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {0, {1, 1}}},
            {IDENTIFIER, "define", {0, {1, 2}}},
            {IDENTIFIER, "NO_PARAMS", {0, {1, 9}}},
            {LBRACKET, NULL, {0, {1, 18}}},
            {RBRACKET, NULL, {0, {1, 19}}},
            {I_CONSTANT, "1", {0, {1, 21}}},
            {ADD, NULL, {0, {1, 23}}},
            {I_CONSTANT, "2", {0, {1, 25}}},
            {ADD, NULL, {0, {1, 27}}},
            {I_CONSTANT, "3", {0, {1, 29}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
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
            .spell = spell,
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
        struct preproc_macro got = parse_preproc_macro(&arr, spell, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
    {
        const char* spell = "NO_PARAMS_EMPTY";
        // #define NO_PARAMS_EMPTY()
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {0, {1, 1}}},
            {IDENTIFIER, "define", {0, {1, 2}}},
            {IDENTIFIER, "NO_PARAMS_EMPTY", {0, {1, 9}}},
            {LBRACKET, NULL, {0, {1, 24}}},
            {RBRACKET, NULL, {0, {1, 25}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
        };

        struct preproc_macro ex = {
            .spell = spell,
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
        struct preproc_macro got = parse_preproc_macro(&arr, spell, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
}

TEST(variadic) {
    {
        const char* spell = "FUNC_LIKE";
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name : c + a
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {0, {1, 1}}},
            {IDENTIFIER, "define", {0, {1, 2}}},
            {IDENTIFIER, "FUNC_LIKE", {0, {1, 9}}},
            {LBRACKET, NULL, {0, {1, 18}}},
            {IDENTIFIER, "a", {0, {1, 19}}},
            {COMMA, NULL, {0, {1, 20}}},
            {IDENTIFIER, "b", {0, {1, 22}}},
            {COMMA, NULL, {0, {1, 23}}},
            {IDENTIFIER, "c", {0, {1, 25}}},
            {COMMA, NULL, {0, {1, 26}}},
            {ELLIPSIS, NULL, {0, {1, 28}}},
            {RBRACKET, NULL, {0, {1, 31}}},
            {IDENTIFIER, "a", {0, {1, 33}}},
            {NE_OP, NULL, {0, {1, 35}}},
            {I_CONSTANT, "38", {0, {1, 38}}},
            {QMARK, NULL, {0, {1, 41}}},
            {IDENTIFIER, "b", {0, {1, 43}}},
            {ASTERISK, NULL, {0, {1, 45}}},
            {IDENTIFIER, "other_name", {0, {1, 47}}},
            {COLON, NULL, {0, {1, 58}}},
            {IDENTIFIER, "c", {0, {1, 60}}},
            {ADD, NULL, {0, {1, 62}}},
            {IDENTIFIER, "a", {0, {1, 64}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
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
            .spell = spell,
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
        struct preproc_macro got = parse_preproc_macro(&arr, spell, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
    {
        const char* spell = "FUNC_LIKE";
        // #define FUNC_LIKE(a, b, c, ...) a != 38 ? b * other_name(__VA_ARGS__)
        // : c + a
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {0, {1, 1}}},
            {IDENTIFIER, "define", {0, {1, 2}}},
            {IDENTIFIER, "FUNC_LIKE", {0, {1, 9}}},
            {LBRACKET, NULL, {0, {1, 18}}},
            {IDENTIFIER, "a", {0, {1, 19}}},
            {COMMA, NULL, {0, {1, 20}}},
            {IDENTIFIER, "b", {0, {1, 22}}},
            {COMMA, NULL, {0, {1, 23}}},
            {IDENTIFIER, "c", {0, {1, 25}}},
            {COMMA, NULL, {0, {1, 26}}},
            {ELLIPSIS, NULL, {0, {1, 28}}},
            {RBRACKET, NULL, {0, {1, 31}}},
            {IDENTIFIER, "a", {0, {1, 33}}},
            {NE_OP, NULL, {0, {1, 35}}},
            {I_CONSTANT, "38", {0, {1, 38}}},
            {QMARK, NULL, {0, {1, 41}}},
            {IDENTIFIER, "b", {0, {1, 43}}},
            {ASTERISK, NULL, {0, {1, 45}}},
            {IDENTIFIER, "other_name", {0, {1, 47}}},
            {LBRACKET, NULL, {0, {1, 48}}},
            {IDENTIFIER, "__VA_ARGS__", {0, {1, 49}}},
            {RBRACKET, NULL, {0, {1, 60}}},
            {COLON, NULL, {0, {1, 62}}},
            {IDENTIFIER, "c", {0, {1, 64}}},
            {ADD, NULL, {0, {1, 66}}},
            {IDENTIFIER, "a", {0, {1, 68}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
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
            .spell = spell,
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
        struct preproc_macro got = parse_preproc_macro(&arr, spell, &err);
        ASSERT(err.type == PREPROC_ERR_NONE);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
}

TEST_SUITE_BEGIN(preproc_macro_parser, 3) {
    REGISTER_TEST(object_like);
    REGISTER_TEST(func_like);
    REGISTER_TEST(variadic);
}
TEST_SUITE_END()
