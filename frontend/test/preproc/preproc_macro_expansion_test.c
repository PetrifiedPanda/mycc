#include "frontend/preproc/preproc.h"
#include "frontend/preproc/PreprocMacro.h"

#include "util/mem.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

static void test_preproc_macro(const PreprocMacro* macro,
                               Str spell,
                               Str input,
                               Str output) {
    const ArchTypeInfo info = get_arch_type_info(ARCH_X86_64, false);
    PreprocErr input_err = PreprocErr_create();
    PreprocRes res = preproc_string(input,
                                    STR_LIT("source_file.c"),
                                    &info,
                                    &input_err);
    ASSERT(input_err.kind == PREPROC_ERR_NONE);

    PreprocErr err = PreprocErr_create();
    PreprocState state = PreprocState_create_string(input,
                                                    STR_LIT("source_file.c"),
                                                    &err);
    state.res = res.toks;
    // Do not free stack allocated macros
    state._macro_map._item_free = NULL;
    StrBuf macro_str = StrBuf_create(spell);
    PreprocState_register_macro(&state, &macro_str, macro);

    ASSERT(expand_all_macros(&state, &state.res, 0, &info));
    ASSERT(err.kind == PREPROC_ERR_NONE);

    PreprocErr output_err = PreprocErr_create();
    PreprocRes expected = preproc_string(output,
                                         STR_LIT("source_file.c"),
                                         &info,
                                         &output_err);
    ASSERT(output_err.kind == PREPROC_ERR_NONE);

    ASSERT_UINT(state.res.len, expected.toks.len);

    for (uint32_t i = 0; i < state.res.len; ++i) {
        ASSERT_TOKEN_KIND(state.res.tokens[i].kind, expected.toks.tokens[i].kind);
        ASSERT_STR(StrBuf_as_str(&state.res.tokens[i].spelling),
                   StrBuf_as_str(&expected.toks.tokens[i].spelling));
        StrBuf_free(&state.res.tokens[i].spelling);
    }
    mycc_free(state.res.tokens);
    FileInfo_free(&res.file_info);
    PreprocRes_free_preproc_tokens(&expected);
    state.res = (TokenArr){
        .tokens = NULL,
        .len = 0,
        .cap = 0,
    };
    PreprocState_free(&state);
}

TEST(expand_obj_like) {
    // #define MACRO 1 + 2
    TokenOrArg expansion[] = {
        {.is_arg = false,
         .token = {TOKEN_I_CONSTANT,
                   .spelling = STR_BUF_NON_HEAP("1"),
                   {0, {1, 15}}}},
        {.is_arg = false,
         .token = {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 17}}}},
        {.is_arg = false,
         .token = {TOKEN_I_CONSTANT,
                   .spelling = STR_BUF_NON_HEAP("2"),
                   {0, {1, 19}}}},
    };
    enum {
        EXP_LEN = ARR_LEN(expansion)
    };

    PreprocMacro macro = {
        .is_func_macro = false,
        .num_args = 0,
        .expansion_len = EXP_LEN,
        .expansion = expansion,
    };

    test_preproc_macro(&macro,
                       STR_LIT("MACRO"),
                       STR_LIT("int var = MACRO;\nfunc();"),
                       STR_LIT("int var = 1 + 2;\nfunc();"));
    test_preproc_macro(
        &macro,
        STR_LIT("MACRO"),
        STR_LIT("MACRO; for (uint32_t i = 0; i < 42; ++i) continue;"),
        STR_LIT("1 + 2; for (uint32_t i = 0; i < 42; ++i) continue;"));
    test_preproc_macro(&macro,
                       STR_LIT("MACRO"),
                       STR_LIT("int x = 1000; MACRO"),
                       STR_LIT("int x = 1000; 1 + 2"));
}

TEST(expand_obj_like_empty) {
    const PreprocMacro macro = {
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = 0,
        .expansion = NULL,
    };

    test_preproc_macro(&macro,
                       STR_LIT("EMPTY_MACRO"),
                       STR_LIT("function EMPTY_MACRO (var, 2);"),
                       STR_LIT("function (var, 2);"));
    test_preproc_macro(&macro,
                       STR_LIT("EMPTY_MACRO"),
                       STR_LIT("EMPTY_MACRO int n = 1000;"),
                       STR_LIT("int n = 1000;"));
    test_preproc_macro(&macro,
                       STR_LIT("EMPTY_MACRO"),
                       STR_LIT("while (true) x *= 2 * 2;\nEMPTY_MACRO;"),
                       STR_LIT("while (true) x *= 2 * 2;\n;"));
}

TEST(expand_recursive) {
    TokenOrArg rec_obj_ex[] = {
        {.is_arg = false,
         .token = {TOKEN_IDENTIFIER,
                   .spelling = STR_BUF_NON_HEAP("REC_MACRO"),
                   {0, {1, 1}}}},
    };
    // #define REC_MACRO REC_MACRO
    const PreprocMacro rec_obj = {
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = ARR_LEN(rec_obj_ex),
        .expansion = rec_obj_ex,
    };

    test_preproc_macro(&rec_obj,
                       STR_LIT("REC_MACRO"),
                       STR_LIT("int x = REC_MACRO - 10;"),
                       STR_LIT("int x = REC_MACRO - 10;"));
    test_preproc_macro(&rec_obj,
                       STR_LIT("REC_MACRO"),
                       STR_LIT("REC_MACRO = REC_MACRO - 10;"),
                       STR_LIT("REC_MACRO = REC_MACRO - 10;"));
    test_preproc_macro(&rec_obj,
                       STR_LIT("REC_MACRO"),
                       STR_LIT("x = REC_MACRO - 10;REC_MACRO"),
                       STR_LIT("x = REC_MACRO - 10;REC_MACRO"));

    TokenOrArg rec_func_ex[] = {
        {.is_arg = false,
         .token = {TOKEN_IDENTIFIER,
                   .spelling = STR_BUF_NON_HEAP("REC_FUNC_MACRO"),
                   {0, {1, 1}}}},
        {.is_arg = false,
         .token = {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 1}}}},
        {.is_arg = false,
         .token = {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 1}}}},
    };

    // #define REC_FUNC_MACRO() REC_FUNC_MACRO()
    const PreprocMacro rec_func = {
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = ARR_LEN(rec_func_ex),
        .expansion = rec_func_ex,
    };
    test_preproc_macro(&rec_func,
                       STR_LIT("REC_FUNC_MACRO"),
                       STR_LIT("int x = REC_FUNC_MACRO() - 10;"),
                       STR_LIT("int x = REC_FUNC_MACRO() - 10;"));
    test_preproc_macro(&rec_func,
                       STR_LIT("REC_FUNC_MACRO"),
                       STR_LIT("REC_FUNC_MACRO() = REC_FUNC_MACRO() - 10;"),
                       STR_LIT("REC_FUNC_MACRO() = REC_FUNC_MACRO() - 10;"));
    test_preproc_macro(&rec_func,
                       STR_LIT("REC_FUNC_MACRO"),
                       STR_LIT("x = REC_FUNC_MACRO() - 10;REC_FUNC_MACRO()"),
                       STR_LIT("x = REC_FUNC_MACRO() - 10;REC_FUNC_MACRO()"));
}

TEST(expand_func_like) {
    // #define FUNC_LIKE_MACRO(x, y) x + y * 3 - y
    TokenOrArg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false,
         .token = {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false,
         .token = {TOKEN_ASTERISK, .spelling = StrBuf_null(), {0, {1, 37}}}},
        {.is_arg = false,
         .token = {TOKEN_I_CONSTANT,
                   .spelling = STR_BUF_NON_HEAP("3"),
                   {0, {1, 39}}}},
        {.is_arg = false,
         .token = {TOKEN_SUB, .spelling = StrBuf_null(), {0, {1, 41}}}},
        {.is_arg = true, .arg_num = 1},
    };

    const PreprocMacro macro1 = {
        .is_func_macro = true,
        .num_args = 2,
        .is_variadic = false,

        .expansion_len = ARR_LEN(ex1),
        .expansion = ex1,
    };

    test_preproc_macro(
        &macro1,
        STR_LIT("FUNC_LIKE_MACRO"),
        STR_LIT("int n = FUNC_LIKE_MACRO(2 * 2, x - 5 + 2) + 1;"),
        STR_LIT("int n = 2 * 2 + x - 5 + 2 * 3 - x - 5 + 2 + 1;"));
    test_preproc_macro(
        &macro1,
        STR_LIT("FUNC_LIKE_MACRO"),
        STR_LIT("int\n n = FUNC_LIKE_MACRO(\n2 * 2,\n x - 5 + 2) + 1;"),
        STR_LIT("int\n n = 2 * 2 + x - 5 + 2 * 3 - x - 5 + 2 + 1;"));
    test_preproc_macro(&macro1,
                       STR_LIT("FUNC_LIKE_MACRO"),
                       STR_LIT("char c = FUNC_LIKE_MACRO(, 10);"),
                       STR_LIT("char c = + 10 * 3 - 10;"));
    test_preproc_macro(&macro1,
                       STR_LIT("FUNC_LIKE_MACRO"),
                       STR_LIT("f = FUNC_LIKE_MACRO(f,);"),
                       STR_LIT("f = f + * 3 -;"));
    test_preproc_macro(&macro1,
                       STR_LIT("FUNC_LIKE_MACRO"),
                       STR_LIT("f = FUNC_LIKE_MACRO(f,\n);"),
                       STR_LIT("f = f + * 3 -;"));

    // #define OTHER_FUNC_LIKE(x, y, z, a, b, c) x
    TokenOrArg ex2[] = {
        {.is_arg = true, .arg_num = 0},
    };

    const PreprocMacro macro2 = {
        .is_func_macro = true,
        .num_args = 6,
        .is_variadic = false,

        .expansion_len = ARR_LEN(ex2),
        .expansion = ex2,
    };

    test_preproc_macro(&macro2,
                       STR_LIT("OTHER_FUNC_LIKE"),
                       STR_LIT("OTHER_FUNC_LIKE(var, 1, 2, 3, 4, 5) = 69;"),
                       STR_LIT("var = 69;"));
    test_preproc_macro(
        &macro2,
        STR_LIT("OTHER_FUNC_LIKE"),
        STR_LIT("OTHER_FUNC_LIKE(var,\n 1, 2,\n 3, 4\n, 5) = 69;"),
        STR_LIT("var = 69;"));
    test_preproc_macro(
        &macro2,
        STR_LIT("OTHER_FUNC_LIKE"),
        STR_LIT("int n = OTHER_FUNC_LIKE(OTHER_FUNC_LIKE(1, a, b, c, d, "
                "e), x, y, z, 1, 2);"),
        STR_LIT("int n = 1;"));

    // #define YET_ANOTHER_FUNC_LIKE() 1 + 1
    TokenOrArg ex3[] = {
        {.is_arg = false,
         .token = {TOKEN_I_CONSTANT,
                   .spelling = STR_BUF_NON_HEAP("1"),
                   {0, {1, 33}}}},
        {.is_arg = false,
         .token = {TOKEN_ADD, .spelling = StrBuf_null(), {0, {1, 35}}}},
        {.is_arg = false,
         .token = {TOKEN_I_CONSTANT,
                   .spelling = STR_BUF_NON_HEAP("1"),
                   {0, {1, 37}}}},
    };

    const PreprocMacro macro3 = {
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = ARR_LEN(ex3),
        .expansion = ex3,
    };

    test_preproc_macro(&macro3,
                       STR_LIT("YET_ANOTHER_FUNC_LIKE"),
                       STR_LIT("const float stuff = YET_ANOTHER_FUNC_LIKE();"),
                       STR_LIT("const float stuff = 1 + 1;"));
    test_preproc_macro(
        &macro3,
        STR_LIT("YET_ANOTHER_FUNC_LIKE"),
        STR_LIT("const float stuff = YET_ANOTHER_FUNC_LIKE\n(\n);"),
        STR_LIT("const float stuff = 1 + 1;"));

    // #define TEST_MACRON()
    const PreprocMacro macro4 = {
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = 0,
        .expansion = NULL,
    };

    test_preproc_macro(&macro4,
                       STR_LIT("TEST_MACRON"),
                       STR_LIT("TEST_MACRON() + 10"),
                       STR_LIT("+ 10"));
    test_preproc_macro(&macro4,
                       STR_LIT("TEST_MACRON"),
                       STR_LIT("TEST_MACRON\n(\n)\n + 10"),
                       STR_LIT("+ 10"));
}

TEST(expand_func_like_variadic) {
    // #define CALL_FUNC(func, ...) func(__VA_ARGS__)
    TokenOrArg ex1[] = {
        {.is_arg = true, .arg_num = 0},
        {.is_arg = false,
         .token = {TOKEN_LBRACKET, .spelling = StrBuf_null(), {0, {1, 33}}}},
        {.is_arg = true, .arg_num = 1},
        {.is_arg = false,
         .token = {TOKEN_RBRACKET, .spelling = StrBuf_null(), {0, {1, 45}}}},
    };

    const PreprocMacro macro1 = {
        .is_func_macro = true,
        .num_args = 1,
        .is_variadic = true,

        .expansion_len = ARR_LEN(ex1),
        .expansion = ex1,
    };

    test_preproc_macro(
        &macro1,
        STR_LIT("CALL_FUNC"),
        STR_LIT("res = CALL_FUNC(printf, \"Hello World %d\", 89);"),
        STR_LIT("res = printf(\"Hello World %d\", 89);"));
    test_preproc_macro(&macro1,
                       STR_LIT("CALL_FUNC"),
                       STR_LIT("CALL_FUNC(function);"),
                       STR_LIT("function();"));

    // #define ONLY_VARARGS(...) 1, 2, __VA_ARGS__
    TokenOrArg ex2[] = {
        {.is_arg = false,
         .token = {TOKEN_I_CONSTANT,
                   .spelling = STR_BUF_NON_HEAP("1"),
                   {0, {1, 27}}}},
        {.is_arg = false,
         .token = {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 28}}}},
        {.is_arg = false,
         .token = {TOKEN_I_CONSTANT,
                   .spelling = STR_BUF_NON_HEAP("2"),
                   {0, {1, 30}}}},
        {.is_arg = false,
         .token = {TOKEN_COMMA, .spelling = StrBuf_null(), {0, {1, 31}}}},
        {.is_arg = true, .arg_num = 0},
    };

    const PreprocMacro macro2 = {
        .is_func_macro = true,
        .num_args = 0,
        .is_variadic = true,

        .expansion_len = ARR_LEN(ex2),
        .expansion = ex2,
    };

    test_preproc_macro(&macro2,
                       STR_LIT("ONLY_VARARGS"),
                       STR_LIT("int n = ONLY_VARARGS();"),
                       STR_LIT("int n = 1, 2,;"));
    test_preproc_macro(&macro2,
                       STR_LIT("ONLY_VARARGS"),
                       STR_LIT("m = ONLY_VARARGS(3, 4);"),
                       STR_LIT("m = 1, 2, 3, 4;"));
}

TEST_SUITE_BEGIN(preproc_macro_expansion){
    REGISTER_TEST(expand_obj_like),
    REGISTER_TEST(expand_obj_like_empty),
    REGISTER_TEST(expand_recursive),
    REGISTER_TEST(expand_func_like),
    REGISTER_TEST(expand_func_like_variadic),
} TEST_SUITE_END()
