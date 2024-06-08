#include "util/IndexedStringSet.h"

#include "testing/testing.h"
#include "testing/asserts.h"

TEST(insert) {
    IndexedStringSet set = IndexedStringSet_create(20);
    const Str to_insert = STR_LIT("test");
    const uint32_t idx = IndexedStringSet_find_or_insert(&set, to_insert);
    // same index returned when we attempt to insert again
    ASSERT_UINT(idx, IndexedStringSet_find_or_insert(&set, to_insert));
    ASSERT_STR(IndexedStringSet_get(&set, idx), to_insert);
}

TEST(correct_rehashing) {
    enum {MAP_CAP = 20 };
    IndexedStringSet set = IndexedStringSet_create(MAP_CAP);
    const Str to_insert[MAP_CAP] = {
        STR_LIT("test"),
        STR_LIT("blah"),
        STR_LIT("something_a_bit_longer"),
        STR_LIT("SOMETHING_ELSE"),
        STR_LIT("long_identifier_ok_this_is_getting_ridiculous_stop"),
        STR_LIT("gleb"),
        STR_LIT("glurbidurb"),
        STR_LIT("identifier"),
        STR_LIT("chairoikoguma"),
        STR_LIT("jammie_dodgers"),
        STR_LIT("indento_switch"),
        STR_LIT("fuck_that_rock_in_tears_of_the_kingdom_that_fell_in_a_lake"),
        STR_LIT("IamRunningOutOfIdeas"),
        STR_LIT("ThatsIt"),
        STR_LIT("camelCase"),
        STR_LIT("PascalCase"),
        STR_LIT("snake_case"),
        STR_LIT("running_out_of_naming_conventions"),
        STR_LIT("lotsOfEffortToTestOneVeryDumbBug"),
        STR_LIT("it_is_done_we_have_enough"),
    };

    for (uint32_t i = 0; i < MAP_CAP; ++i) {
        const uint32_t idx = IndexedStringSet_find_or_insert(&set, to_insert[i]);
        ASSERT_STR(IndexedStringSet_get(&set, idx), to_insert[i]);
    }

    const uint32_t pre_search_len = set._len;
    for (uint32_t i = 0; i < MAP_CAP; ++i) {
        const uint32_t idx = IndexedStringSet_find_or_insert(&set, to_insert[i]);
        ASSERT_STR(IndexedStringSet_get(&set, idx), to_insert[i]);
        // check if we didn't just insert the same thing again
        ASSERT_UINT(set._len, pre_search_len);
    }
}

TEST_SUITE_BEGIN(IndexedStringsSet) {
    REGISTER_TEST(insert),
    REGISTER_TEST(correct_rehashing),
} TEST_SUITE_END()
