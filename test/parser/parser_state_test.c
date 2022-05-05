#include <stdio.h>

#include "parser/parser_state.h"

#include "../test_asserts.h"

TEST(parser_state) {
    struct token dummy = {.type = INVALID};
    struct parser_state s = create_parser_state(&dummy);

    enum {
        NUM_STRINGS = 1000,
        STRLEN = NUM_STRINGS + 1,
        SCOPE_INTERVAL = 200
    };
    _Static_assert(
        NUM_STRINGS % SCOPE_INTERVAL == 0,
        "Number of test strings must be divisible by the scope interval");

    struct token dummy_string_tokens[NUM_STRINGS] = {0};
    char insert_string[STRLEN] = {0};
    for (size_t i = 0; i < NUM_STRINGS; ++i) {

        if (i % SCOPE_INTERVAL == 0) {
            parser_push_scope(&s);
        }

        insert_string[i] = 'a';

        struct token* item = &dummy_string_tokens[i];
        *item = create_token_copy(IDENTIFIER,
                                  insert_string,
                                  (struct source_location){0, 0},
                                  "file.c");
        if (i % 2 == 0) {
            ASSERT(register_enum_constant(&s, item));
        } else {
            ASSERT(register_typedef_name(&s, item));
        }
    }

    char test_string[STRLEN] = {0};
    for (size_t i = 0; i < NUM_STRINGS; ++i) {
        test_string[i] = 'a';

        if (i % 2 == 0) {
            ASSERT(is_enum_constant(&s, test_string));
            ASSERT(!is_typedef_name(&s, test_string));
        } else {
            ASSERT(is_typedef_name(&s, test_string));
            ASSERT(!is_enum_constant(&s, test_string));
        }
    }

    const size_t num_steps = NUM_STRINGS / SCOPE_INTERVAL + 1;
    for (size_t i = 0; i < num_steps; ++i) {
        char pop_test_string[STRLEN] = {0};
        size_t j;
        for (j = 0; j < NUM_STRINGS - i * SCOPE_INTERVAL; ++j) {
            pop_test_string[j] = 'a';

            if (j % 2 == 0) {
                ASSERT(is_enum_constant(&s, pop_test_string));
                ASSERT(!is_typedef_name(&s, pop_test_string));
            } else {
                ASSERT(is_typedef_name(&s, pop_test_string));
                ASSERT(!is_enum_constant(&s, pop_test_string));
            }
        }

        // test if values from popped scopes are not present anymore
        for (; j < NUM_STRINGS; ++j) {
            pop_test_string[j] = 'a';

            ASSERT(!is_enum_constant(&s, pop_test_string));
            ASSERT(!is_typedef_name(&s, pop_test_string));
        }

        // do not pop last scope
        if (i != num_steps - 1) {
            parser_pop_scope(&s);
        }
    }

    ASSERT(s.len == 1);

    struct token insert_test_token = {
        .type = IDENTIFIER,
        .spelling = "Test",
        .file = "file.c",
        .source_loc = {.line = 0, .index = 0},
    };
    ASSERT(register_enum_constant(&s, &insert_test_token));
    ASSERT(!register_typedef_name(&s, &insert_test_token));
    ASSERT_ERROR(get_last_error(), ERR_PARSER);

    clear_last_error();

    ASSERT(!register_enum_constant(&s, &insert_test_token));
    ASSERT_ERROR(get_last_error(), ERR_PARSER);

    clear_last_error();

    for (size_t i = 0; i < NUM_STRINGS; ++i) {
        free_token(&dummy_string_tokens[i]);
    }
    free_parser_state(&s);
}

TEST_SUITE_BEGIN(parser_state, 1) {
    REGISTER_TEST(parser_state);
}
TEST_SUITE_END()
