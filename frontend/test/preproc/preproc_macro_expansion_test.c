#include "frontend/preproc/preproc.h"
#include "frontend/preproc/preproc_macro.h"

#include "util/mem.h"

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
                               const char* input,
                               const char* output) {
    struct preproc_err input_err = create_preproc_err();
    struct preproc_res res = preproc_string(input, "source_file.c", &input_err);
    ASSERT(input_err.type == PREPROC_ERR_NONE);

    const size_t tokens_len = get_tokens_len(res.toks);

    struct preproc_err err = create_preproc_err();
    struct preproc_state state = create_preproc_state("source_file.c", &err);
    state.res = (struct token_arr){
        .len = tokens_len,
        .cap = tokens_len,
        .tokens = res.toks,
    };
    // Do not free stack allocated macros
    state._macro_map._item_free = NULL;
    struct str macro_str = create_str(strlen(macro->spell), macro->spell);
    register_preproc_macro(&state, &macro_str, macro);

    struct code_source src = {
        ._is_str = true,
        ._str = "",
    };
    ASSERT(expand_all_macros(&state, &state.res, 0, &src));
    ASSERT(err.type == PREPROC_ERR_NONE);

    struct preproc_err output_err = create_preproc_err();
    struct preproc_res expected = preproc_string(output,
                                                 "source_file.c",
                                                 &output_err);
    ASSERT(output_err.type == PREPROC_ERR_NONE);

    ASSERT_SIZE_T(state.res.len, get_tokens_len(expected.toks));

    for (size_t i = 0; i < state.res.len; ++i) {
        ASSERT_TOKEN_TYPE(state.res.tokens[i].type, expected.toks[i].type);
        ASSERT_STR(str_get_data(&state.res.tokens[i].spelling),
                   str_get_data(&expected.toks[i].spelling));
        free_str(&state.res.tokens[i].spelling);
    }
    mycc_free(state.res.tokens);
    free_file_info(&res.file_info);
    free_preproc_res_preproc_tokens(&expected);
    state.res = (struct token_arr){
        .tokens = NULL,
        .len = 0,
        .cap = 0,
    };
    free_preproc_state(&state);
}

TEST(object_like) {
    // #define MACRO 1 + 2
    struct token_or_arg expansion[] = {
        {.is_arg = false,
         .token = {I_CONSTANT, .spelling = STR_NON_HEAP("1"), {0, {1, 15}}}},
        {.is_arg = false,
         .token = {ADD, .spelling = create_null_str(), {0, {1, 17}}}},
        {.is_arg = false,
         .token = {I_CONSTANT, .spelling = STR_NON_HEAP("2"), {0, {1, 19}}}},
    };
    enum {
        EXP_LEN = ARR_LEN(expansion)
    };

    struct preproc_macro macro = {
        .spell = "MACRO",
        .is_func_macro = false,
        .num_args = 0,
        .expansion_len = EXP_LEN,
        .expansion = expansion,
    };

    test_preproc_macro(&macro,
                       "int var = MACRO;\nfunc();",
                       "int var = 1 + 2;\nfunc();");
    test_preproc_macro(&macro,
                       "MACRO; for (size_t i = 0; i < 42; ++i) continue;",
                       "1 + 2; for (size_t i = 0; i < 42; ++i) continue;");
    test_preproc_macro(&macro, "int x = 1000; MACRO", "int x = 1000; 1 + 2");
}

TEST(object_like_empty) {
    const struct preproc_macro macro = {
        .spell = "EMPTY_MACRO",
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = 0,
        .expansion = NULL,
    };

    test_preproc_macro(&macro,
                       "function EMPTY_MACRO (var, 2);",
                       "function (var, 2);");
    test_preproc_macro(&macro, "EMPTY_MACRO int n = 1000;", "int n = 1000;");
    test_preproc_macro(&macro,
                       "while (true) x *= 2 * 2;\nEMPTY_MACRO;",
                       "while (true) x *= 2 * 2;\n;");
}

TEST(recursive) {
    struct token_or_arg rec_obj_ex[] = {
        {.is_arg = false,
         .token = {IDENTIFIER,
                   .spelling = STR_NON_HEAP("REC_MACRO"),
                   {0, {1, 1}}}},
    };
    // #define REC_MACRO REC_MACRO
    const struct preproc_macro rec_obj = {
        .spell = "REC_MACRO",
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = ARR_LEN(rec_obj_ex),
        .expansion = rec_obj_ex,
    };

    test_preproc_macro(&rec_obj,
                       "int x = REC_MACRO - 10;",
                       "int x = REC_MACRO - 10;");
    test_preproc_macro(&rec_obj,
                       "REC_MACRO = REC_MACRO - 10;",
                       "REC_MACRO = REC_MACRO - 10;");
    test_preproc_macro(&rec_obj,
                       "x = REC_MACRO - 10;REC_MACRO",
                       "x = REC_MACRO - 10;REC_MACRO");

    struct token_or_arg rec_func_ex[] = {
        {.is_arg = false,
         .token = {IDENTIFIER,
                   .spelling = STR_NON_HEAP("REC_FUNC_MACRO"),
                   {0, {1, 1}}}},
        {.is_arg = false,
         .token = {LBRACKET, .spelling = create_null_str(), {0, {1, 1}}}},
        {.is_arg = false,
         .token = {RBRACKET, .spelling = create_null_str(), {0, {1, 1}}}},
    };

    // #define REC_FUNC_MACRO() REC_FUNC_MACRO()
    const struct preproc_macro rec_func = {
        .spell = "REC_FUNC_MACRO",
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = ARR_LEN(rec_func_ex),
        .expansion = rec_func_ex,
    };
    test_preproc_macro(&rec_func,
                       "int x = REC_FUNC_MACRO() - 10;",
                       "int x = REC_FUNC_MACRO() - 10;");
    test_preproc_macro(&rec_func,
                       "REC_FUNC_MACRO() = REC_FUNC_MACRO() - 10;",
                       "REC_FUNC_MACRO() = REC_FUNC_MACRO() - 10;");
    test_preproc_macro(&rec_func,
                       "x = REC_FUNC_MACRO() - 10;REC_FUNC_MACRO()",
                       "x = REC_FUNC_MACRO() - 10;REC_FUNC_MACRO()");
}

TEST(func_like) {
    // #define FUNC_LIKE_MACRO(x, y) x + y * 3 - y
    struct token_or_arg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false,
         .token = {ADD, .spelling = create_null_str(), {0, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false,
         .token = {ASTERISK, .spelling = create_null_str(), {0, {1, 37}}}},
        {.is_arg = false,
         .token = {I_CONSTANT, .spelling = STR_NON_HEAP("3"), {0, {1, 39}}}},
        {.is_arg = false,
         .token = {SUB, .spelling = create_null_str(), {0, {1, 41}}}},
        {.is_arg = true, .arg_num = 1},
    };

    const struct preproc_macro macro1 = {
        .spell = "FUNC_LIKE_MACRO",
        .is_func_macro = true,
        .num_args = 2,
        .is_variadic = false,

        .expansion_len = ARR_LEN(ex1),
        .expansion = ex1,
    };

    test_preproc_macro(&macro1,
                       "int n = FUNC_LIKE_MACRO(2 * 2, x - 5 + 2) + 1;",
                       "int n = 2 * 2 + x - 5 + 2 * 3 - x - 5 + 2 + 1;");
    test_preproc_macro(&macro1,
                       "int\n n = FUNC_LIKE_MACRO(\n2 * 2,\n x - 5 + 2) + 1;",
                       "int\n n = 2 * 2 + x - 5 + 2 * 3 - x - 5 + 2 + 1;");
    test_preproc_macro(&macro1,
                       "char c = FUNC_LIKE_MACRO(, 10);",
                       "char c = + 10 * 3 - 10;");
    test_preproc_macro(&macro1, "f = FUNC_LIKE_MACRO(f,);", "f = f + * 3 -;");
    test_preproc_macro(&macro1, "f = FUNC_LIKE_MACRO(f,\n);", "f = f + * 3 -;");

    // #define OTHER_FUNC_LIKE(x, y, z, a, b, c) x
    struct token_or_arg ex2[] = {
        {.is_arg = true, .arg_num = 0},
    };

    const struct preproc_macro macro2 = {
        .spell = "OTHER_FUNC_LIKE",
        .is_func_macro = true,
        .num_args = 6,
        .is_variadic = false,

        .expansion_len = ARR_LEN(ex2),
        .expansion = ex2,
    };

    test_preproc_macro(&macro2,
                       "OTHER_FUNC_LIKE(var, 1, 2, 3, 4, 5) = 69;",
                       "var = 69;");
    test_preproc_macro(&macro2,
                       "OTHER_FUNC_LIKE(var,\n 1, 2,\n 3, 4\n, 5) = 69;",
                       "var = 69;");
    test_preproc_macro(&macro2,
                       "int n = OTHER_FUNC_LIKE(OTHER_FUNC_LIKE(1, a, b, c, d, "
                       "e), x, y, z, 1, 2);",
                       "int n = 1;");

    // #define YET_ANOTHER_FUNC_LIKE() 1 + 1
    struct token_or_arg ex3[] = {
        {.is_arg = false,
         .token = {I_CONSTANT, .spelling = STR_NON_HEAP("1"), {0, {1, 33}}}},
        {.is_arg = false,
         .token = {ADD, .spelling = create_null_str(), {0, {1, 35}}}},
        {.is_arg = false,
         .token = {I_CONSTANT, .spelling = STR_NON_HEAP("1"), {0, {1, 37}}}},
    };

    const struct preproc_macro macro3 = {
        .spell = "YET_ANOTHER_FUNC_LIKE",
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = ARR_LEN(ex3),
        .expansion = ex3,
    };

    test_preproc_macro(&macro3,
                       "const float stuff = YET_ANOTHER_FUNC_LIKE();",
                       "const float stuff = 1 + 1;");
    test_preproc_macro(&macro3,
                       "const float stuff = YET_ANOTHER_FUNC_LIKE\n(\n);",
                       "const float stuff = 1 + 1;");

    // #define TEST_MACRON()
    const struct preproc_macro macro4 = {
        .spell = "TEST_MACRON",
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = 0,
        .expansion = NULL,
    };

    test_preproc_macro(&macro4, "TEST_MACRON() + 10", "+ 10");
    test_preproc_macro(&macro4, "TEST_MACRON\n(\n)\n + 10", "+ 10");
}

TEST(func_like_variadic) {
    // #define CALL_FUNC(func, ...) func(__VA_ARGS__)
    struct token_or_arg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false,
         .token = {LBRACKET, .spelling = create_null_str(), {0, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false,
         .token = {RBRACKET, .spelling = create_null_str(), {0, {1, 45}}}},
    };

    const struct preproc_macro macro1 = {
        .spell = "CALL_FUNC",
        .is_func_macro = true,
        .num_args = 1,
        .is_variadic = true,

        .expansion_len = ARR_LEN(ex1),
        .expansion = ex1,
    };

    test_preproc_macro(&macro1,
                       "res = CALL_FUNC(printf, \"Hello World %d\", 89);",
                       "res = printf(\"Hello World %d\", 89);");
    test_preproc_macro(&macro1, "CALL_FUNC(function);", "function();");

    // #define ONLY_VARARGS(...) 1, 2, __VA_ARGS__
    struct token_or_arg ex2[] = {
        {.is_arg = false,
         .token = {I_CONSTANT, .spelling = STR_NON_HEAP("1"), {0, {1, 27}}}},
        {.is_arg = false,
         .token = {COMMA, .spelling = create_null_str(), {0, {1, 28}}}},
        {.is_arg = false,
         .token = {I_CONSTANT, .spelling = STR_NON_HEAP("2"), {0, {1, 30}}}},
        {.is_arg = false,
         .token = {COMMA, .spelling = create_null_str(), {0, {1, 31}}}},
        {.is_arg = true, .arg_num = 0},
    };

    const struct preproc_macro macro2 = {
        .spell = "ONLY_VARARGS",
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = true,

        .expansion_len = ARR_LEN(ex2),
        .expansion = ex2,
    };

    test_preproc_macro(&macro2, "int n = ONLY_VARARGS();", "int n = 1, 2,;");
    test_preproc_macro(&macro2, "m = ONLY_VARARGS(3, 4);", "m = 1, 2, 3, 4;");
}

TEST_SUITE_BEGIN(preproc_macro_expansion, 5) {
    REGISTER_TEST(object_like);
    REGISTER_TEST(object_like_empty);
    REGISTER_TEST(recursive);
    REGISTER_TEST(func_like);
    REGISTER_TEST(func_like_variadic);
}
TEST_SUITE_END()
