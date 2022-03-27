#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#include "preproc/preproc_macro.h"

#include "../test_asserts.h"

static void non_func_like_test();

void preproc_macro_test() {
    non_func_like_test();
    printf("\tPrepocessor macro test successful\n");
}

static void check_token(struct token* got, struct token* ex) {
    ASSERT_TOKEN_TYPE(got->type, ex->type);
    ASSERT_STR(got->spelling, ex->spelling);
    ASSERT_STR(got->file, ex->file);
    ASSERT_SIZE_T(got->source_loc.line, ex->source_loc.line);
    ASSERT_SIZE_T(got->source_loc.index, ex->source_loc.index);
}

static void non_func_like_test() {

    char* macro_file = (char*)"macro_file.c";
    struct token_or_num expansion[] = {
        {.is_arg_num = false, .token = {I_CONSTANT, "1", macro_file, {1, 15}}},
        {.is_arg_num = false, .token = {ADD, NULL, macro_file, {1, 17}}},
        {.is_arg_num = false, .token = {I_CONSTANT, "2", macro_file, {1, 19}}},
    };
    enum { EXP_LEN = sizeof(expansion) / sizeof(struct token_or_num) };

    struct preproc_macro macro = {
        .spelling = "MACRO",
        .is_func_macro = false,
        .num_args = 0,
        .expansion_len = EXP_LEN,
        .expansion = expansion,
    };

    char* file = (char*)"file.c";
    struct token tokens[] = {
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
    ASSERT(expand_preproc_macro(&state, &macro, 3));
    ASSERT_SIZE_T(state.len, prev_len + EXP_LEN - 1);
    ASSERT_SIZE_T(state.len, state.cap);

    for (size_t i = 0; i < 3; ++i) {
        struct token* ex = &tokens[i];
        struct token* got = &state.tokens[i];

        check_token(got, ex);
    }

    for (size_t i = 0; i < macro.expansion_len; ++i) {
        struct token* ex = &expansion[i].token;
        struct token* got = &state.tokens[3 + i];

        check_token(got, ex);
        // only the inserted functions have heap allocated strings
        free_token(got);
    }

    for (size_t i = 0; i < TOKENS_LEN - 4; ++i) {
        check_token(&state.tokens[3 + macro.expansion_len + i], &tokens[4 + i]);
    }

    free(state.tokens);
}
