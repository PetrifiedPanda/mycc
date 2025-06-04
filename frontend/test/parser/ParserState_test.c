#include "testing/asserts.h"

#include "frontend/parser/ParserState.h"

#include "../test_helpers.h"

TEST(ParserState) {
    uint8_t kinds[] = {
        TOKEN_IDENTIFIER,
    };
    uint32_t val_indices[] = {
        0,
    };
    SourceLoc locs[] = {
        {0, {0, 0}},
    };
    StrBuf identifiers[] = {
        STR_BUF_NON_HEAP("Test"),
    };
    TokenArr dummy_arr = {
        .len = 1,
        .cap = 1,
        .kinds = kinds,
        .val_indices = val_indices,
        .locs = locs,
        .identifiers_len = 1,
        .identifiers = identifiers,
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

    StrBuf dummy_strings[NUM_STRINGS] = {0};
    for (uint32_t i = 0; i < NUM_STRINGS; ++i) {
        if (i % SCOPE_INTERVAL == 0) {
            ParserState_push_scope(&s);
        }

        if (i % 2 == 0) {
            ASSERT(ParserState_register_enum_constant(&s, i, UINT32_MAX));
        } else {
            ASSERT(ParserState_register_typedef(&s, i, UINT32_MAX));
        }
        ASSERT(err.kind == PARSER_ERR_NONE);
    }

    for (uint32_t i = 0; i < NUM_STRINGS; ++i) {
        if (i % 2 == 0) {
            ASSERT(
                ParserState_is_enum_constant(&s, i));
            ASSERT(!ParserState_is_typedef(&s, i)); 
        } else {
            ASSERT(ParserState_is_typedef(&s, i));
            ASSERT(!ParserState_is_enum_constant(&s, i));
        }
        ASSERT(err.kind == PARSER_ERR_NONE);
    }

    const uint32_t num_steps = NUM_STRINGS / SCOPE_INTERVAL + 1;
    for (uint32_t i = 0; i < num_steps; ++i) {
        uint32_t j;
        for (j = 0; j < NUM_STRINGS - i * SCOPE_INTERVAL; ++j) {
            if (j % 2 == 0) {
                ASSERT(ParserState_is_enum_constant(&s, j));
                ASSERT(!ParserState_is_typedef(&s, j));
            } else {
                ASSERT(ParserState_is_typedef(&s, j));
                ASSERT(!ParserState_is_enum_constant(&s, j));
            }
            ASSERT(err.kind == PARSER_ERR_NONE);
        }

        // test if values from popped scopes are not present anymore
        for (; j < NUM_STRINGS; ++j) {
            ASSERT(!ParserState_is_enum_constant(&s, j));
            ASSERT(!ParserState_is_typedef(&s, j));

            ASSERT(err.kind == PARSER_ERR_NONE);
        }

        // do not pop last scope
        if (i != num_steps - 1) {
            ParserState_pop_scope(&s);
        }
    }

    ASSERT(s._len == 1);
    ASSERT(err.kind == PARSER_ERR_NONE);

    const uint32_t idx = 1;
    ASSERT(ParserState_register_enum_constant(&s, NUM_STRINGS, idx));
    ASSERT(!ParserState_register_typedef(&s, NUM_STRINGS, idx));

    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT_UINT(err.err_token_idx, idx);
    ASSERT(!err.was_typedef_name);

    err = ParserErr_create();

    ASSERT(!ParserState_register_enum_constant(&s, NUM_STRINGS, idx));

    ASSERT(err.kind == PARSER_ERR_REDEFINED_SYMBOL);
    ASSERT_UINT(err.err_token_idx, idx);
    ASSERT(!err.was_typedef_name);

    for (uint32_t i = 0; i < NUM_STRINGS; ++i) {
        StrBuf_free(&dummy_strings[i]);
    }
    ParserState_free(&s);
}

TEST_SUITE_BEGIN(ParserState){
    REGISTER_TEST(ParserState),
} TEST_SUITE_END()
