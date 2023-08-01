#include "testing/asserts.h"

#include "frontend/parser/ParserState.h"

#include "../test_helpers.h"

TEST(ParserState) {
    Token dummy = {.kind = TOKEN_INVALID};
    TokenArr dummy_arr = {
        .len = 1,
        .cap = 1,
        .tokens = &dummy,
    };
    ParserErr err = ParserErr_create();
    ParserState s = ParserState_create(&dummy_arr, &err);

    enum {
        NUM_STRINGS = 100,
        STRLEN = NUM_STRINGS + 1,
        SCOPE_INTERVAL = 20,
    };
    static_assert(
        NUM_STRINGS % SCOPE_INTERVAL == 0,
        "Number of test strings must be divisible by the scope interval");

    Token dummy_string_tokens[NUM_STRINGS] = {{0}};
    char insert_string[STRLEN] = {0};
    for (uint32_t i = 0; i < NUM_STRINGS; ++i) {

        if (i % SCOPE_INTERVAL == 0) {
            ParserState_push_scope(&s);
        }

        insert_string[i] = 'a';
        const StrBuf to_insert = StrBuf_non_heap(i + 1, insert_string);

        Token* item = &dummy_string_tokens[i];
        const SourceLoc loc = {0, {0, 0}};
        *item = Token_create_copy(TOKEN_IDENTIFIER,
                                  &to_insert,
                                  (FileLoc){0, 0},
                                  0);
        if (i % 2 == 0) {
            ASSERT(ParserState_register_enum_constant(&s, &to_insert, loc));
        } else {
            ASSERT(ParserState_register_typedef(&s, &to_insert, loc));
        }
        ASSERT(err.kind == PARSER_ERR_NONE);
    }

    char test_string[STRLEN] = {0};
    for (uint32_t i = 0; i < NUM_STRINGS; ++i) {
        test_string[i] = 'a';

        const StrBuf test_string_str = StrBuf_non_heap(i + 1, test_string);
        if (i % 2 == 0) {
            ASSERT(ParserState_is_enum_constant(&s, StrBuf_as_str(&test_string_str)));
            ASSERT(!ParserState_is_typedef(&s, StrBuf_as_str(&test_string_str)));
        } else {
            ASSERT(ParserState_is_typedef(&s, StrBuf_as_str(&test_string_str)));
            ASSERT(!ParserState_is_enum_constant(&s, StrBuf_as_str(&test_string_str)));
        }
        ASSERT(err.kind == PARSER_ERR_NONE);
    }

    const uint32_t num_steps = NUM_STRINGS / SCOPE_INTERVAL + 1;
    for (uint32_t i = 0; i < num_steps; ++i) {
        char pop_test_string[STRLEN] = {0};
        uint32_t j;
        for (j = 0; j < NUM_STRINGS - i * SCOPE_INTERVAL; ++j) {
            pop_test_string[j] = 'a';
            const StrBuf pop_test_str = StrBuf_non_heap(j + 1, pop_test_string);

            if (j % 2 == 0) {
                ASSERT(ParserState_is_enum_constant(&s, StrBuf_as_str(&pop_test_str)));
                ASSERT(!ParserState_is_typedef(&s, StrBuf_as_str(&pop_test_str)));
            } else {
                ASSERT(ParserState_is_typedef(&s, StrBuf_as_str(&pop_test_str)));
                ASSERT(!ParserState_is_enum_constant(&s, StrBuf_as_str(&pop_test_str)));
            }
            ASSERT(err.kind == PARSER_ERR_NONE);
        }

        // test if values from popped scopes are not present anymore
        for (; j < NUM_STRINGS; ++j) {
            pop_test_string[j] = 'a';
            const StrBuf pop_test_str = StrBuf_non_heap(j + 1, pop_test_string);

            ASSERT(!ParserState_is_enum_constant(&s, StrBuf_as_str(&pop_test_str)));
            ASSERT(!ParserState_is_typedef(&s, StrBuf_as_str(&pop_test_str)));

            ASSERT(err.kind == PARSER_ERR_NONE);
        }

        // do not pop last scope
        if (i != num_steps - 1) {
            ParserState_pop_scope(&s);
        }
    }

    ASSERT(s._len == 1);
    ASSERT(err.kind == PARSER_ERR_NONE);
    
    const StrBuf insert_test_spell = STR_BUF_NON_HEAP("Test");
    const SourceLoc loc = {0, {0, 0}};
    ASSERT(ParserState_register_enum_constant(&s, &insert_test_spell, loc));
    ASSERT(!ParserState_register_typedef(&s, &insert_test_spell, loc));

    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT_STR(StrBuf_as_str(&err.redefined_symbol), STR_LIT("Test"));
    ASSERT(!err.was_typedef_name);
    ASSERT_UINT(err.prev_def_file, (uint32_t)0);
    ASSERT_UINT(err.prev_def_loc.line, (uint32_t)0);
    ASSERT_UINT(err.prev_def_loc.index, (uint32_t)0);

    ParserErr_free(&err);
    err = ParserErr_create();

    ASSERT(!ParserState_register_enum_constant(&s, &insert_test_spell, loc));

    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT_STR(StrBuf_as_str(&err.redefined_symbol), STR_LIT("Test"));
    ASSERT(!err.was_typedef_name);
    ASSERT_UINT(err.prev_def_file, (uint32_t)0);
    ASSERT_UINT(err.prev_def_loc.line, (uint32_t)0);
    ASSERT_UINT(err.prev_def_loc.index, (uint32_t)0);

    for (uint32_t i = 0; i < NUM_STRINGS; ++i) {
        Token_free(&dummy_string_tokens[i]);
    }
    ParserErr_free(&err);
    ParserState_free(&s);
}

TEST_SUITE_BEGIN(ParserState){
    REGISTER_TEST(ParserState),
} TEST_SUITE_END()
