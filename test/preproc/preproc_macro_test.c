#include <stdio.h>
#include <stdlib.h>

#include "token_type.h"
#include "util.h"

#include "preproc/preproc_macro.h"

#include "../test_asserts.h"

static void check_token(const struct token* got,
                        const struct token* ex) {
    ASSERT_TOKEN_TYPE(got->type, ex->type);
    ASSERT_STR(got->spelling, ex->spelling);
    ASSERT_STR(got->file, ex->file);
    ASSERT_SIZE_T(got->source_loc.line, ex->source_loc.line);
    ASSERT_SIZE_T(got->source_loc.index, ex->source_loc.index);
}

TEST(non_func_like) {
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

    char* file = (char*)"file.c";

    // int var = MACRO;
    // func();
    const struct token tokens[] = {
        {IDENTIFIER, "int", file, {1, 5}},
        {IDENTIFIER, "var", file, {1, 9}},
        {ASSIGN, NULL, file, {1, 13}},
        {IDENTIFIER,
         alloc_string_copy(macro.spelling),
         alloc_string_copy(file),
         {1, 15}}, // This is the only token freed by the function
        {SEMICOLON, NULL, file, {1, 20}},
        {IDENTIFIER, "func", file, {2, 5}},
        {LBRACKET, NULL, file, {2, 10}},
        {RBRACKET, NULL, file, {2, 11}},
        {SEMICOLON, NULL, file, {2, 12}},
    };

    enum { TOKENS_LEN = sizeof(tokens) / sizeof(struct token) };

    struct preproc_state state = {
        .len = TOKENS_LEN,
        .cap = TOKENS_LEN,
        .tokens = malloc(sizeof(tokens)),
    };
    ASSERT_NOT_NULL(state.tokens);

    memcpy(state.tokens, tokens, sizeof(tokens));

    const size_t prev_len = state.len;
    ASSERT(expand_preproc_macro(&state, &macro, 3, NULL));
    ASSERT_SIZE_T(state.len, prev_len + EXP_LEN - 1);
    ASSERT_SIZE_T(state.len, state.cap);

    for (size_t i = 0; i < 3; ++i) {
        const struct token* ex = &tokens[i];
        const struct token* got = &state.tokens[i];

        check_token(got, ex);
    }

    for (size_t i = 0; i < macro.expansion_len; ++i) {
        struct token* ex = &expansion[i].token;
        struct token* got = &state.tokens[i + 3];

        check_token(got, ex);
        // only the inserted functions have heap allocated strings
        free_token(got);
    }

    for (size_t i = 0; i < TOKENS_LEN - 4; ++i) {
        check_token(&state.tokens[3 + macro.expansion_len + i], &tokens[4 + i]);
    }

    free(state.tokens);
}

TEST(non_func_like_empty) {
    const char* macro_name = "EMPTY_MACRO";
    const struct preproc_macro macro = {
        .spelling = (char*)macro_name,
        .is_func_macro = false,
        .num_args = 0,
        .is_variadic = false,

        .expansion_len = 0,
        .expansion = NULL,
    };
    
    char* file = (char*)"not_a_file.c";

    // function EMPTY_MACRO (var, 2);
    const struct token tokens[] = {
        {IDENTIFIER, "function", file, {1, 1}},
        {IDENTIFIER, alloc_string_copy(macro_name), alloc_string_copy(file), {1, 9}},
        {LBRACKET, NULL, file, {1, 21}},
        {IDENTIFIER, "var", file, {1, 22}},
        {COMMA, NULL, file, {1, 25}},
        {I_CONSTANT, "2", file, {1, 25}},
        {RBRACKET, NULL, file, {1, 26}},
        {SEMICOLON, NULL, file, {1, 27}},
    };
    enum { TOKENS_LEN = sizeof(tokens) / sizeof(struct token) };
    
    struct preproc_state s = {
        .len = TOKENS_LEN,
        .cap = TOKENS_LEN,
        .tokens = malloc(sizeof(tokens)),
    };

    memcpy(s.tokens, tokens, sizeof(tokens));

    const struct token result[] = {
        {IDENTIFIER, "function", file, {1, 1}},
        {LBRACKET, NULL, file, {1, 21}},
        {IDENTIFIER, "var", file, {1, 22}},
        {COMMA, NULL, file, {1, 25}},
        {I_CONSTANT, "2", file, {1, 25}},
        {RBRACKET, NULL, file, {1, 26}},
        {SEMICOLON, NULL, file, {1, 27}},
    };
    enum { RESULT_LEN = sizeof(result) / sizeof(struct token) };

    const size_t prev_len = s.len;
    ASSERT(expand_preproc_macro(&s, &macro, 1, NULL));
    ASSERT_SIZE_T(s.len, prev_len - 1);
    
    for (size_t i = 0; i < RESULT_LEN; ++i) {
        const struct token* got = &s.tokens[i];
        const struct token* ex = &result[i];

        check_token(got, ex);
    }

    free(s.tokens);
}

TEST_SUITE_BEGIN(preproc_macro, 2) {
    REGISTER_TEST(non_func_like);
    REGISTER_TEST(non_func_like_empty);
}
TEST_SUITE_END()
