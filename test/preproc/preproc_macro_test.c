#include <stdio.h>
#include <stdlib.h>

#include "token.h"
#include "token_type.h"

#include "util/mem.h"

#include "preproc/preproc.h"
#include "preproc/preproc_macro.h"

#include "../test_asserts.h"

static size_t get_tokens_len(const struct token* tokens) {
    size_t len = 0;
    const struct token* it = tokens;

    while (it->type != INVALID) {
        ++len;
        ++it;
    }

    return len;
}

static void test_preproc_macro(const struct preproc_macro* macro,
                               size_t macro_idx,
                               size_t macro_end_idx,
                               const char* input,
                               const char* output) {
    struct preproc_err input_err = create_preproc_err();
    struct token* tokens = preproc_string(input, "source_file.c", &input_err);
    ASSERT(input_err.type == PREPROC_ERR_NONE);

    const size_t tokens_len = get_tokens_len(tokens);

    struct preproc_err err = create_preproc_err();
    struct preproc_state state = {
        .res =
            {
                .len = tokens_len,
                .cap = tokens_len,
                .tokens = tokens,
            },
        .err = &err,
    };

    const struct token* macro_end = macro_end_idx != (size_t)-1
                                        ? tokens + macro_end_idx
                                        : NULL;
    ASSERT(expand_preproc_macro(&state, macro, macro_idx, macro_end));
    ASSERT(err.type == PREPROC_ERR_NONE);

    struct preproc_err output_err = create_preproc_err();
    struct token* expected = preproc_string(output,
                                            "source_file.c",
                                            &output_err);
    ASSERT(output_err.type == PREPROC_ERR_NONE);

    ASSERT_SIZE_T(state.res.len, get_tokens_len(expected));

    for (size_t i = 0; i < state.res.len; ++i) {
        ASSERT_TOKEN_TYPE(state.res.tokens[i].type, expected[i].type);
        ASSERT_STR(state.res.tokens[i].spelling, expected[i].spelling);
        free_token(&state.res.tokens[i]);
    }
    free(state.res.tokens);
    free_tokens(expected);
}

TEST(object_like) {
    char* macro_file = (char*)"macro_file.c";
    // #define MACRO 1 + 2
    struct token_or_arg expansion[] = {
        {.is_arg = false, .token = {I_CONSTANT, "1", {macro_file, {1, 15}}}},
        {.is_arg = false, .token = {ADD, NULL, {macro_file, {1, 17}}}},
        {.is_arg = false, .token = {I_CONSTANT, "2", {macro_file, {1, 19}}}},
    };
    enum {
        EXP_LEN = sizeof(expansion) / sizeof(struct token_or_arg)
    };

    struct preproc_macro macro = {
        .spelling = "MACRO",
        .is_func_macro = false,
        .num_args = 0,
        .expansion_len = EXP_LEN,
        .expansion = expansion,
    };

    test_preproc_macro(&macro,
                       3,
                       (size_t)-1,
                       "int var = MACRO;\nfunc();",
                       "int var = 1 + 2;\nfunc();");
    test_preproc_macro(&macro,
                       0,
                       (size_t)-1,
                       "MACRO; for (size_t i = 0; i < 42; ++i) continue;",
                       "1 + 2; for (size_t i = 0; i < 42; ++i) continue;");
    test_preproc_macro(&macro,
                       5,
                       (size_t)-1,
                       "int x = 1000; MACRO",
                       "int x = 1000; 1 + 2");
}

TEST(object_like_empty) {
    const char* macro_name = "EMPTY_MACRO";
    const struct preproc_macro macro = {
        .spelling = (char*)macro_name,
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = 0,
        .expansion = NULL,
    };

    test_preproc_macro(&macro,
                       1,
                       (size_t)-1,
                       "function EMPTY_MACRO (var, 2);",
                       "function (var, 2);");
    test_preproc_macro(&macro,
                       0,
                       (size_t)-1,
                       "EMPTY_MACRO int n = 1000;",
                       "int n = 1000;");
    test_preproc_macro(&macro,
                       10,
                       (size_t)-1,
                       "while (true) x *= 2 * 2;\nEMPTY_MACRO;",
                       "while (true) x *= 2 * 2;\n;");
}

TEST(func_like) {
    char* macro_file = "macro_file.c";

    // #define FUNC_LIKE_MACRO(x, y) x + y * 3 - y
    struct token_or_arg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false, .token = {ADD, NULL, {macro_file, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false, .token = {ASTERISK, NULL, {macro_file, {1, 37}}}},
        {.is_arg = false, .token = {I_CONSTANT, "3", {macro_file, {1, 39}}}},
        {.is_arg = false, .token = {SUB, NULL, {macro_file, {1, 41}}}},
        {.is_arg = true, .arg_num = 1},
    };

    const struct preproc_macro macro1 = {
        .spelling = (char*)"FUNC_LIKE_MACRO",
        .is_func_macro = true,
        .num_args = 2,
        .is_variadic = false,

        .expansion_len = sizeof(ex1) / sizeof(struct token_or_arg),
        .expansion = ex1,
    };

    test_preproc_macro(&macro1,
                       3,
                       14,
                       "int n = FUNC_LIKE_MACRO(2 * 2, x - 5 + 2) + 1;",
                       "int n = 2 * 2 + x - 5 + 2 * 3 - x - 5 + 2 + 1;");
    test_preproc_macro(&macro1,
                       3,
                       7,
                       "char c = FUNC_LIKE_MACRO(, 10);",
                       "char c = + 10 * 3 - 10;");
    test_preproc_macro(&macro1,
                       2,
                       6,
                       "f = FUNC_LIKE_MACRO(f,);",
                       "f = f + * 3 -;");

    // #define OTHER_FUNC_LIKE(x, y, z, a, b, c) x
    struct token_or_arg ex2[] = {
        {.is_arg = true, .arg_num = 0},
    };

    const struct preproc_macro macro2 = {
        .spelling = (char*)"OTHER_FUNC_LIKE",
        .is_func_macro = true,
        .num_args = 6,
        .is_variadic = false,

        .expansion_len = sizeof(ex2) / sizeof(struct token_or_arg),
        .expansion = ex2,
    };

    test_preproc_macro(&macro2,
                       0,
                       13,
                       "OTHER_FUNC_LIKE(var, 1, 2, 3, 4, 5) = 69;",
                       "var = 69;");

    // #define YET_ANOTHER_FUNC_LIKE() 1 + 1
    struct token_or_arg ex3[] = {
        {.is_arg = false, .token = {I_CONSTANT, "1", {macro_file, {1, 33}}}},
        {.is_arg = false, .token = {ADD, NULL, {macro_file, {1, 35}}}},
        {.is_arg = false, .token = {I_CONSTANT, "1", {macro_file, {1, 37}}}},
    };

    const struct preproc_macro macro3 = {
        .spelling = (char*)"YET_ANOTHER_FUNC_LIKE",
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = sizeof(ex3) / sizeof(struct token_or_arg),
        .expansion = ex3,
    };

    test_preproc_macro(&macro3,
                       4,
                       6,
                       "const float stuff = YET_ANOTHER_FUNC_LIKE();",
                       "const float stuff = 1 + 1;");
}

TEST(func_like_variadic) {
    char* macro_file = "macro_file.c";

    // #define CALL_FUNC(func, ...) func(__VA_ARGS__)
    struct token_or_arg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false, .token = {LBRACKET, NULL, {macro_file, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false, .token = {RBRACKET, NULL, {macro_file, {1, 45}}}},
    };

    const struct preproc_macro macro1 = {
        .spelling = (char*)"CALL_FUNC",
        .is_func_macro = true,
        .num_args = 1,
        .is_variadic = true,

        .expansion_len = sizeof(ex1) / sizeof(struct token_or_arg),
        .expansion = ex1,
    };

    test_preproc_macro(&macro1,
                       2,
                       9,
                       "res = CALL_FUNC(printf, \"Hello World %d\", 89);",
                       "res = printf(\"Hello World %d\", 89);");
    test_preproc_macro(&macro1, 0, 3, "CALL_FUNC(function);", "function();");

    // #define ONLY_VARARGS(...) 1, 2, __VA_ARGS__
    struct token_or_arg ex2[] = {
        {.is_arg = false, .token = {I_CONSTANT, "1", {macro_file, {1, 27}}}},
        {.is_arg = false, .token = {COMMA, NULL, {macro_file, {1, 28}}}},
        {.is_arg = false, .token = {I_CONSTANT, "2", {macro_file, {1, 30}}}},
        {.is_arg = false, .token = {COMMA, NULL, {macro_file, {1, 31}}}},
        {.is_arg = true, .arg_num = 0},
    };

    const struct preproc_macro macro2 = {
        .spelling = (char*)"ONLY_VARARGS",
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = true,

        .expansion_len = sizeof(ex2) / sizeof(struct token_or_arg),
        .expansion = ex2,
    };

    test_preproc_macro(&macro2,
                       3,
                       5,
                       "int n = ONLY_VARARGS();",
                       "int n = 1, 2,;");
    test_preproc_macro(&macro2,
                       2,
                       7,
                       "m = ONLY_VARARGS(3, 4);",
                       "m = 1, 2, 3, 4;");
}

static void compare_preproc_macros(const struct preproc_macro* got,
                                   const struct preproc_macro* ex) {
    ASSERT_STR(got->spelling, ex->spelling);

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

            ASSERT_STR(got_tok->loc.file, ex_tok->loc.file);
            ASSERT_SIZE_T(got_tok->loc.file_loc.line,
                          ex_tok->loc.file_loc.line);
            ASSERT_SIZE_T(got_tok->loc.file_loc.index,
                          ex_tok->loc.file_loc.index);
        }
    }
}

TEST(parse_object_like) {
    {
        // #define TEST_MACRO
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {"file.c", {1, 1}}},
            {IDENTIFIER, "define", {"file.c", {1, 2}}},
            {IDENTIFIER, "TEST_MACRO", {"file.c", {1, 9}}},
        };

        struct token_arr arr = {
            .len = sizeof tokens / sizeof(struct token),
            .cap = arr.len,
            .tokens = tokens,
        };

        struct preproc_macro got = parse_preproc_macro(&arr);

        struct preproc_macro ex = {
            .spelling = "TEST_MACRO",
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
        // #define ANOTHER_MACRO 1 + 2 * 3 - func(a, b)
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {"file.c", {1, 1}}},
            {IDENTIFIER, "define", {"file.c", {1, 2}}},
            {IDENTIFIER, "ANOTHER_MACRO", {"file.c", {1, 9}}},
            {I_CONSTANT, "1", {"file.c", {1, 23}}},
            {ADD, NULL, {"file.c", {1, 25}}},
            {I_CONSTANT, "2", {"file.c", {1, 27}}},
            {ASTERISK, NULL, {"file.c", {1, 29}}},
            {I_CONSTANT, "3", {"file.c", {1, 31}}},
            {SUB, NULL, {"file.c", {1, 33}}},
            {IDENTIFIER, "func", {"file.c", {1, 35}}},
            {LBRACKET, NULL, {"file.c", {1, 39}}},
            {IDENTIFIER, "a", {"file.c", {1, 40}}},
            {COMMA, NULL, {"file.c", {1, 41}}},
            {IDENTIFIER, "b", {"file.c", {1, 43}}},
            {RBRACKET, NULL, {"file.c", {1, 44}}},
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

        struct preproc_macro got = parse_preproc_macro(&arr);

        struct preproc_macro ex = {
            .spelling = "ANOTHER_MACRO",
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

TEST(parse_func_like) {
    {
        // #define FUNC_LIKE(a, b, c) a != 38 ? b * other_name : c + a
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {"other_file.c", {1, 1}}},
            {IDENTIFIER, "define", {"other_file.c", {1, 2}}},
            {IDENTIFIER, "FUNC_LIKE", {"other_file.c", {1, 9}}},
            {LBRACKET, NULL, {"other_file.c", {1, 18}}},
            {IDENTIFIER, "a", {"other_file.c", {1, 19}}},
            {COMMA, NULL, {"other_file.c", {1, 20}}},
            {IDENTIFIER, "b", {"other_file.c", {1, 22}}},
            {COMMA, NULL, {"other_file.c", {1, 23}}},
            {IDENTIFIER, "c", {"other_file.c", {1, 25}}},
            {RBRACKET, NULL, {"other_file.c", {1, 26}}},
            {IDENTIFIER, "a", {"other_file.c", {1, 28}}},
            {NE_OP, NULL, {"other_file.c", {1, 30}}},
            {I_CONSTANT, "38", {"other_file.c", {1, 33}}},
            {QMARK, NULL, {"other_file.c", {1, 36}}},
            {IDENTIFIER, "b", {"other_file.c", {1, 38}}},
            {ASTERISK, NULL, {"other_file.c", {1, 40}}},
            {IDENTIFIER, "other_name", {"other_file.c", {1, 42}}},
            {COLON, NULL, {"other_file.c", {1, 53}}},
            {IDENTIFIER, "c", {"other_file.c", {1, 55}}},
            {ADD, NULL, {"other_file.c", {1, 57}}},
            {IDENTIFIER, "a", {"other_file.c", {1, 59}}},
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
            .spelling = "FUNC_LIKE",
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
        struct preproc_macro got = parse_preproc_macro(&arr);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
    {
        // #define NO_PARAMS() 1 + 2 + 3
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {"other_file.c", {1, 1}}},
            {IDENTIFIER, "define", {"other_file.c", {1, 2}}},
            {IDENTIFIER, "NO_PARAMS", {"other_file.c", {1, 9}}},
            {LBRACKET, NULL, {"other_file.c", {1, 18}}},
            {RBRACKET, NULL, {"other_file.c", {1, 19}}},
            {I_CONSTANT, "1", {"other_file.c", {1, 21}}},
            {ADD, NULL, {"other_file.c", {1, 23}}},
            {I_CONSTANT, "2", {"other_file.c", {1, 25}}},
            {ADD, NULL, {"other_file.c", {1, 27}}},
            {I_CONSTANT, "3", {"other_file.c", {1, 29}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
            EXPANSION_LEN = TOKENS_LEN - 5
        };
    
        struct token_or_arg expansion[EXPANSION_LEN];
        for (size_t i = 0; i < EXPANSION_LEN; ++i) {
            expansion[i] = (struct token_or_arg) {
                .is_arg = false,
                .token = tokens[i + 5],
            };
        }

        struct preproc_macro ex = {
            .spelling = "NO_PARAMS",
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
        struct preproc_macro got = parse_preproc_macro(&arr);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
    {
        // #define NO_PARAMS_EMPTY()
        struct token tokens[] = {
            {STRINGIFY_OP, NULL, {"other_file.c", {1, 1}}},
            {IDENTIFIER, "define", {"other_file.c", {1, 2}}},
            {IDENTIFIER, "NO_PARAMS_EMPTY", {"other_file.c", {1, 9}}},
            {LBRACKET, NULL, {"other_file.c", {1, 24}}},
            {RBRACKET, NULL, {"other_file.c", {1, 25}}},
        };

        enum {
            TOKENS_LEN = sizeof tokens / sizeof(struct token),
        }; 

        struct preproc_macro ex = {
            .spelling = "NO_PARAMS_EMPTY",
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
        struct preproc_macro got = parse_preproc_macro(&arr);

        compare_preproc_macros(&got, &ex);
        free(got.expansion);
    }
}

TEST(parse_variadic) {
    ASSERT(false);
}

TEST_SUITE_BEGIN(preproc_macro, 7) {
    REGISTER_TEST(object_like);
    REGISTER_TEST(object_like_empty);
    REGISTER_TEST(func_like);
    REGISTER_TEST(func_like_variadic);
    REGISTER_TEST(parse_object_like);
    REGISTER_TEST(parse_func_like);
    REGISTER_TEST(parse_variadic);
}
TEST_SUITE_END()
