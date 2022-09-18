#include "frontend/preproc/preproc.h"
#include "frontend/preproc/preproc_macro.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

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
    struct preproc_res res = preproc_string(input, "source_file.c", &input_err);
    ASSERT(input_err.type == PREPROC_ERR_NONE);

    const size_t tokens_len = get_tokens_len(res.toks);

    struct preproc_err err = create_preproc_err();
    struct preproc_state state = {
        .res =
            {
                .len = tokens_len,
                .cap = tokens_len,
                .tokens = res.toks,
            },
        .err = &err,
    };

    ASSERT(expand_preproc_macro(&state, &state.res, macro, macro_idx, macro_end_idx));
    ASSERT(err.type == PREPROC_ERR_NONE);

    struct preproc_err output_err = create_preproc_err();
    struct preproc_res expected = preproc_string(output,
                                            "source_file.c",
                                            &output_err);
    ASSERT(output_err.type == PREPROC_ERR_NONE);

    ASSERT_SIZE_T(state.res.len, get_tokens_len(expected.toks));

    for (size_t i = 0; i < state.res.len; ++i) {
        ASSERT_TOKEN_TYPE(state.res.tokens[i].type, expected.toks[i].type);
        ASSERT_STR(state.res.tokens[i].spelling, expected.toks[i].spelling);
        free(state.res.tokens[i].spelling);
    }
    free(state.res.tokens);
    free_file_info(&res.file_info);
    free_preproc_res_preproc_tokens(&expected);
}

TEST(object_like) {
    // #define MACRO 1 + 2
    struct token_or_arg expansion[] = {
        {.is_arg = false, .token = {I_CONSTANT, .spelling = "1", {0, {1, 15}}}},
        {.is_arg = false, .token = {ADD, .spelling = NULL, {0, {1, 17}}}},
        {.is_arg = false, .token = {I_CONSTANT, .spelling = "2", {0, {1, 19}}}},
    };
    enum {
        EXP_LEN = sizeof(expansion) / sizeof *expansion
    };

    struct preproc_macro macro = {
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
    const struct preproc_macro macro = {
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
    // #define FUNC_LIKE_MACRO(x, y) x + y * 3 - y
    struct token_or_arg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false, .token = {ADD, .spelling = NULL, {0, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false, .token = {ASTERISK, .spelling = NULL, {0, {1, 37}}}},
        {.is_arg = false, .token = {I_CONSTANT, .spelling = "3", {0, {1, 39}}}},
        {.is_arg = false, .token = {SUB, .spelling = NULL, {0, {1, 41}}}},
        {.is_arg = true, .arg_num = 1},
    };

    const struct preproc_macro macro1 = {
        .is_func_macro = true,
        .num_args = 2,
        .is_variadic = false,

        .expansion_len = sizeof(ex1) / sizeof *ex1,
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
        .is_func_macro = true,
        .num_args = 6,
        .is_variadic = false,

        .expansion_len = sizeof(ex2) / sizeof *ex2,
        .expansion = ex2,
    };

    test_preproc_macro(&macro2,
                       0,
                       13,
                       "OTHER_FUNC_LIKE(var, 1, 2, 3, 4, 5) = 69;",
                       "var = 69;");

    // #define YET_ANOTHER_FUNC_LIKE() 1 + 1
    struct token_or_arg ex3[] = {
        {.is_arg = false, .token = {I_CONSTANT, .spelling = "1", {0, {1, 33}}}},
        {.is_arg = false, .token = {ADD, .spelling = NULL, {0, {1, 35}}}},
        {.is_arg = false, .token = {I_CONSTANT, .spelling = "1", {0, {1, 37}}}},
    };

    const struct preproc_macro macro3 = {
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = sizeof(ex3) / sizeof *ex3,
        .expansion = ex3,
    };

    test_preproc_macro(&macro3,
                       4,
                       6,
                       "const float stuff = YET_ANOTHER_FUNC_LIKE();",
                       "const float stuff = 1 + 1;");
}

TEST(func_like_variadic) {
    // #define CALL_FUNC(func, ...) func(__VA_ARGS__)
    struct token_or_arg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false, .token = {LBRACKET, .spelling = NULL, {0, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false, .token = {RBRACKET, .spelling = NULL, {0, {1, 45}}}},
    };

    const struct preproc_macro macro1 = {
        .is_func_macro = true,
        .num_args = 1,
        .is_variadic = true,

        .expansion_len = sizeof(ex1) / sizeof *ex1,
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
        {.is_arg = false, .token = {I_CONSTANT, .spelling = "1", {0, {1, 27}}}},
        {.is_arg = false, .token = {COMMA, .spelling = NULL, {0, {1, 28}}}},
        {.is_arg = false, .token = {I_CONSTANT, .spelling = "2", {0, {1, 30}}}},
        {.is_arg = false, .token = {COMMA, .spelling = NULL, {0, {1, 31}}}},
        {.is_arg = true, .arg_num = 0},
    };

    const struct preproc_macro macro2 = {
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = true,

        .expansion_len = sizeof(ex2) / sizeof *ex2,
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

TEST_SUITE_BEGIN(preproc_macro_expansion, 4) {
    REGISTER_TEST(object_like);
    REGISTER_TEST(object_like_empty);
    REGISTER_TEST(func_like);
    REGISTER_TEST(func_like_variadic);
}
TEST_SUITE_END()
