#include <stdio.h>

#include "testing/asserts.h"

#include "frontend/parser/parser_state.h"

#include "../test_helpers.h"

TEST(parser_state) {
    struct token dummy = {.kind = TOKEN_INVALID};
    struct parser_err err = create_parser_err();
    struct parser_state s = create_parser_state(&dummy, &err);

    enum {
        NUM_STRINGS = 100,
        STRLEN = NUM_STRINGS + 1,
        SCOPE_INTERVAL = 20,
    };
    static_assert(
        NUM_STRINGS % SCOPE_INTERVAL == 0,
        "Number of test strings must be divisible by the scope interval");

    struct token dummy_string_tokens[NUM_STRINGS] = {{0}};
    char insert_string[STRLEN] = {0};
    for (size_t i = 0; i < NUM_STRINGS; ++i) {

        if (i % SCOPE_INTERVAL == 0) {
            parser_push_scope(&s);
        }

        insert_string[i] = 'a';
        const struct str to_insert = str_non_heap(i + 1, insert_string);

        struct token* item = &dummy_string_tokens[i];
        *item = create_token_copy(TOKEN_IDENTIFIER,
                                  &to_insert,
                                  (struct file_loc){0, 0},
                                  0);
        if (i % 2 == 0) {
            ASSERT(register_enum_constant(&s, item));
        } else {
            ASSERT(register_typedef_name(&s, item));
        }
        ASSERT(err.kind == PARSER_ERR_NONE);
    }

    char test_string[STRLEN] = {0};
    for (size_t i = 0; i < NUM_STRINGS; ++i) {
        test_string[i] = 'a';

        const struct str test_string_str = str_non_heap(i + 1, test_string);
        if (i % 2 == 0) {
            ASSERT(is_enum_constant(&s, &test_string_str));
            ASSERT(!is_typedef_name(&s, &test_string_str));
        } else {
            ASSERT(is_typedef_name(&s, &test_string_str));
            ASSERT(!is_enum_constant(&s, &test_string_str));
        }
        ASSERT(err.kind == PARSER_ERR_NONE);
    }

    const size_t num_steps = NUM_STRINGS / SCOPE_INTERVAL + 1;
    for (size_t i = 0; i < num_steps; ++i) {
        char pop_test_string[STRLEN] = {0};
        size_t j;
        for (j = 0; j < NUM_STRINGS - i * SCOPE_INTERVAL; ++j) {
            pop_test_string[j] = 'a';
            const struct str pop_test_str = str_non_heap(j + 1, pop_test_string);

            if (j % 2 == 0) {
                ASSERT(is_enum_constant(&s, &pop_test_str));
                ASSERT(!is_typedef_name(&s, &pop_test_str));
            } else {
                ASSERT(is_typedef_name(&s, &pop_test_str));
                ASSERT(!is_enum_constant(&s, &pop_test_str));
            }
            ASSERT(err.kind == PARSER_ERR_NONE);
        }

        // test if values from popped scopes are not present anymore
        for (; j < NUM_STRINGS; ++j) {
            pop_test_string[j] = 'a';
            const struct str pop_test_str = str_non_heap(j + 1, pop_test_string);

            ASSERT(!is_enum_constant(&s, &pop_test_str));
            ASSERT(!is_typedef_name(&s, &pop_test_str));

            ASSERT(err.kind == PARSER_ERR_NONE);
        }

        // do not pop last scope
        if (i != num_steps - 1) {
            parser_pop_scope(&s);
        }
    }

    ASSERT(s._len == 1);
    ASSERT(err.kind == PARSER_ERR_NONE);

    struct token insert_test_token = {
        .kind = TOKEN_IDENTIFIER,
        .spelling = STR_NON_HEAP("Test"),
        .loc =
            {
                .file_idx = 0,
                .file_loc = {.line = 0, .index = 0},
            },
    };
    ASSERT(register_enum_constant(&s, &insert_test_token));
    ASSERT(!register_typedef_name(&s, &insert_test_token));

    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT_STR(str_get_data(&err.redefined_symbol), "Test");
    ASSERT(!err.was_typedef_name);
    ASSERT_SIZE_T(err.prev_def_file, (size_t)0);
    ASSERT_SIZE_T(err.prev_def_loc.line, (size_t)0);
    ASSERT_SIZE_T(err.prev_def_loc.index, (size_t)0);

    free_parser_err(&err);
    err = create_parser_err();

    ASSERT(!register_enum_constant(&s, &insert_test_token));

    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT_STR(str_get_data(&err.redefined_symbol), "Test");
    ASSERT(!err.was_typedef_name);
    ASSERT_SIZE_T(err.prev_def_file, (size_t)0);
    ASSERT_SIZE_T(err.prev_def_loc.line, (size_t)0);
    ASSERT_SIZE_T(err.prev_def_loc.index, (size_t)0);

    for (size_t i = 0; i < NUM_STRINGS; ++i) {
        free_token(&dummy_string_tokens[i]);
    }
    free_parser_err(&err);
    free_parser_state(&s);
}

TEST_SUITE_BEGIN(parser_state, 1) {
    REGISTER_TEST(parser_state);
}
TEST_SUITE_END()
