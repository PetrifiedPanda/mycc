#include "util/StringMap.h"

#include "testing/testing.h"
#include "testing/asserts.h"

enum {
    KEY_STR_LEN = 5,
    KEY_CHAR_ARR_LEN = KEY_STR_LEN + 1,
};

static uint32_t write_str_combos_rec(char strs[][KEY_CHAR_ARR_LEN],
                                   uint32_t num,
                                   uint32_t str_len,
                                   uint32_t str_idx,
                                   uint32_t current_idx) {
    enum {
        START_CHAR = 'A',
        END_CHAR = 'z' + 1
    };
    static_assert(START_CHAR < END_CHAR,
                  "End char must be larger than start char");

    if (str_idx == str_len) {
        return current_idx + 1;
    } else {
        char tmp_str[KEY_CHAR_ARR_LEN] = {0};
        memcpy(tmp_str, strs[current_idx], sizeof *tmp_str * (str_idx));
        uint32_t child_idx = current_idx;
        for (char i = START_CHAR; i < END_CHAR; ++i) {
            if (child_idx == num) {
                return child_idx;
            }
            tmp_str[str_idx] = i;

            memcpy(strs[child_idx], tmp_str, sizeof *tmp_str * (str_idx + 1));
            child_idx = write_str_combos_rec(strs,
                                             num,
                                             str_len,
                                             str_idx + 1,
                                             child_idx);
        }
        return child_idx;
    }
}

// generates either all possible combinations of strings of length str_len, or
// num strings
static uint32_t write_str_combos(char strs[][KEY_CHAR_ARR_LEN],
                               uint32_t num,
                               uint32_t str_len) {
    for (uint32_t i = 0; i < num; ++i) {
        strs[i][str_len] = '\0';
    }

    return write_str_combos_rec(strs, num, str_len, 0, 0);
}

static StrBuf str_from_generated(char* str) {
    return (StrBuf){
        ._is_static_buf = false,
        ._len = KEY_STR_LEN,
        ._cap = KEY_STR_LEN + 1,
        ._data = str,
    };
}

static void insert_items(StringMap* map,
                         char keys[][KEY_CHAR_ARR_LEN],
                         uint32_t num_inserts) {
    assert(map->_item_size == sizeof(uint32_t));
    for (uint32_t i = 0; i < num_inserts; ++i) {
        const StrBuf to_insert = str_from_generated(keys[i]);
        const uint32_t* ret = StringMap_insert(map, &to_insert, &i);
        ASSERT(ret == &i);

        // check if the item is present directly after insertions
        const uint32_t* item = StringMap_get(map, StrBuf_as_str(&to_insert));
        ASSERT_UINT(*item, i);
    }
}

TEST(insert) {
    enum {
        INIT_CAP = 20,
        NUM_INSERTS = INIT_CAP * 4,
    };
    StringMap map = StringMap_create(sizeof(uint32_t),
                                              INIT_CAP,
                                              false,
                                              NULL);

    char keys[NUM_INSERTS][KEY_CHAR_ARR_LEN];

    const uint32_t written = write_str_combos(keys, NUM_INSERTS, KEY_STR_LEN);
    ASSERT_UINT(written, (uint32_t)NUM_INSERTS);

    insert_items(&map, keys, NUM_INSERTS);

    const uint32_t to_insert = UINT32_MAX;
    for (uint32_t i = 0; i < NUM_INSERTS; ++i) {
        // try to insert already existing item
        const StrBuf insert_key = str_from_generated(keys[i]);
        const uint32_t* ret = StringMap_insert(&map, &insert_key, &to_insert);
        ASSERT_UINT(*ret, i);

        // check if item was overwritten
        const uint32_t* item = StringMap_get(&map, StrBuf_as_str(&insert_key));
        ASSERT_UINT(*item, i);
    }

    StringMap_free(&map);
}

TEST(remove) {
    enum {
        INIT_CAP = 20,
        NUM_INSERTS = INIT_CAP * 2
    };

    StringMap map = StringMap_create(sizeof(uint32_t),
                                              INIT_CAP,
                                              false,
                                              NULL);

    char keys[NUM_INSERTS][KEY_CHAR_ARR_LEN];

    const uint32_t written = write_str_combos(keys, NUM_INSERTS, KEY_STR_LEN);
    ASSERT_UINT(written, (uint32_t)NUM_INSERTS);

    insert_items(&map, keys, NUM_INSERTS);

    for (uint32_t i = 0; i < NUM_INSERTS; ++i) {
        const StrBuf key = str_from_generated(keys[i]);
        StringMap_remove(&map, &key);
        ASSERT_UINT(map._len, (uint32_t)NUM_INSERTS - 1);

        const void* item = StringMap_get(&map, StrBuf_as_str(&key));
        ASSERT_NULL(item);
        for (uint32_t j = 0; j < NUM_INSERTS; ++j) {
            if (j == i) {
                continue;
            }
            const StrBuf other_key = str_from_generated(keys[j]);
            const uint32_t* ret = StringMap_get(&map, StrBuf_as_str(&other_key));
            ASSERT_NOT_NULL(ret);
            ASSERT_UINT(*ret, j);
        }

        const uint32_t* insert_ret = StringMap_insert(&map, &key, &i);
        ASSERT_UINT(*insert_ret, i);
        ASSERT_UINT(map._len, (uint32_t)NUM_INSERTS);
        for (uint32_t j = 0; j < NUM_INSERTS; ++j) {
            const StrBuf other_key = str_from_generated(keys[j]);
            const uint32_t* ret = StringMap_get(&map, StrBuf_as_str(&other_key));
            ASSERT_NOT_NULL(ret);
            ASSERT_UINT(*ret, j);
        }
    }

    for (uint32_t i = 0; i < NUM_INSERTS; ++i) {
        ASSERT_UINT(map._len, (uint32_t)NUM_INSERTS - i);
        const StrBuf to_remove = str_from_generated(keys[i]);
        StringMap_remove(&map, &to_remove);

        const void* item = StringMap_get(&map, StrBuf_as_str(&to_remove));
        ASSERT_NULL(item);
        for (uint32_t j = i + 1; j < NUM_INSERTS; ++j) {
            const StrBuf other_key = str_from_generated(keys[j]);
            const uint32_t* ret = StringMap_get(&map, StrBuf_as_str(&other_key));
            ASSERT_NOT_NULL(ret);
            ASSERT_UINT(*ret, j);
        }
    }

    StringMap_free(&map);
}

TEST(correct_rehashing) {
    enum {MAP_CAP = 20 };
    StringMap map = StringMap_create(sizeof(uint32_t), MAP_CAP, false, NULL);
    const Str to_insert[] = {
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
    static_assert(ARR_LEN(to_insert) == MAP_CAP, "Need same amount of items as map capacity");

    for (uint32_t i = 0; i < MAP_CAP; ++i) {
        const StrBuf buf = {
            ._is_static_buf = false,
            ._len = to_insert[i].len,
            ._data = (char*)to_insert[i].data,
        };
        const uint32_t* res = StringMap_insert(&map, &buf, &i);
        ASSERT_UINT(*res, i);
    }

    const uint32_t* res = StringMap_get(&map, STR_LIT("not_in_map"));
    ASSERT_NULL(res);

    StringMap_free(&map);
}

TEST_SUITE_BEGIN(string_map){
    REGISTER_TEST(insert),
    REGISTER_TEST(remove),
    REGISTER_TEST(correct_rehashing),
} TEST_SUITE_END()
