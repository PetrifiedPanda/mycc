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

static void test_preproc_macro(const struct preproc_macro* macro, size_t macro_idx, const char* input, const char* output) {
    struct preproc_err input_err = create_preproc_err();
    struct token* tokens = preproc_string(input, "source_file.c", &input_err);
    ASSERT(input_err.type == PREPROC_ERR_NONE);

    const size_t tokens_len = get_tokens_len(tokens);
    
    struct preproc_state state = {
        .len = tokens_len,
        .cap = tokens_len,
        .tokens = tokens,
    };

    ASSERT(expand_preproc_macro(&state, macro, macro_idx, NULL));
    
    struct preproc_err output_err = create_preproc_err();
    struct token* expected = preproc_string(output, "source_file.c", &output_err);
    ASSERT(output_err.type == PREPROC_ERR_NONE);

    ASSERT_SIZE_T(state.len, get_tokens_len(expected));

    for (size_t i = 0; i < state.len; ++i) {
        ASSERT_TOKEN_TYPE(state.tokens[i].type, expected[i].type);
        ASSERT_STR(state.tokens[i].spelling, expected[i].spelling);
        free_token(&state.tokens[i]);
    }
    free(state.tokens);
    free_tokens(expected);
}

TEST(object_like) {
    char* macro_file = (char*)"macro_file.c";
    // #define MACRO 1 + 2
    struct token_or_arg expansion[] = {
        {.is_arg = false, .token = {I_CONSTANT, "1", macro_file, {1, 15}}},
        {.is_arg = false, .token = {ADD, NULL, macro_file, {1, 17}}},
        {.is_arg = false, .token = {I_CONSTANT, "2", macro_file, {1, 19}}},
    };
    enum { EXP_LEN = sizeof(expansion) / sizeof(struct token_or_arg) };

    struct preproc_macro macro = {
        .spelling = "MACRO",
        .is_func_macro = false,
        .num_args = 0,
        .expansion_len = EXP_LEN,
        .expansion = expansion,
    };

    test_preproc_macro(&macro, 3, "int var = MACRO;\nfunc();", "int var = 1 + 2;\nfunc();");
    test_preproc_macro(&macro, 0, "MACRO; for (size_t i = 0; i < 42; ++i) continue;", "1 + 2; for (size_t i = 0; i < 42; ++i) continue;");
    test_preproc_macro(&macro, 5, "int x = 1000; MACRO", "int x = 1000; 1 + 2");
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
    
    test_preproc_macro(&macro, 1, "function EMPTY_MACRO (var, 2);", "function (var, 2);");
    test_preproc_macro(&macro, 0, "EMPTY_MACRO int n = 1000;", "int n = 1000;");
    test_preproc_macro(&macro, 10, "while (true) x *= 2 * 2;\nEMPTY_MACRO;", "while (true) x *= 2 * 2;\n;");
}

TEST_SUITE_BEGIN(preproc_macro, 2) {
    REGISTER_TEST(object_like);
    REGISTER_TEST(object_like_empty);
}
TEST_SUITE_END()
