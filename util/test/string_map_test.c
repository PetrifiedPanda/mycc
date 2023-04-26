#include "util/string_map.h"

#include "testing/testing.h"
#include "testing/asserts.h"

enum {
    KEY_STR_LEN = 5,
    KEY_CHAR_ARR_LEN = KEY_STR_LEN + 1,
};

static size_t write_str_combos_rec(char strs[][KEY_CHAR_ARR_LEN],
                                   size_t num,
                                   size_t str_len,
                                   size_t str_idx,
                                   size_t current_idx) {
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
        size_t child_idx = current_idx;
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
static size_t write_str_combos(char strs[][KEY_CHAR_ARR_LEN],
                               size_t num,
                               size_t str_len) {
    for (size_t i = 0; i < num; ++i) {
        strs[i][str_len] = '\0';
    }

    return write_str_combos_rec(strs, num, str_len, 0, 0);
}

static struct str str_from_generated(char* str) {
    return (struct str){
        ._is_static_buf = false,
        ._len = KEY_STR_LEN,
        ._cap = KEY_STR_LEN + 1,
        ._data = str,
    };
}

static void insert_items(struct string_map* map,
                         char keys[][KEY_CHAR_ARR_LEN],
                         size_t num_inserts) {
    assert(map->_item_size == sizeof(size_t));
    for (size_t i = 0; i < num_inserts; ++i) {
        const struct str to_insert = str_from_generated(keys[i]);
        const size_t* ret = string_map_insert(map, &to_insert, &i);
        ASSERT(ret == &i);

        // check if the item is present directly after insertions
        const size_t* item = string_map_get(map, &to_insert);
        ASSERT_SIZE_T(*item, i);
    }
}

TEST(insert) {
    enum {
        INIT_CAP = 20,
        NUM_INSERTS = INIT_CAP * 4,
    };
    struct string_map map = create_string_map(sizeof(size_t),
                                              INIT_CAP,
                                              false,
                                              NULL);

    char keys[NUM_INSERTS][KEY_CHAR_ARR_LEN];

    const size_t written = write_str_combos(keys, NUM_INSERTS, KEY_STR_LEN);
    ASSERT_SIZE_T(written, (size_t)NUM_INSERTS);

    insert_items(&map, keys, NUM_INSERTS);

    const size_t to_insert = (size_t)-1;
    for (size_t i = 0; i < NUM_INSERTS; ++i) {
        // try to insert already existing item
        const struct str insert_key = str_from_generated(keys[i]);
        const size_t* ret = string_map_insert(&map, &insert_key, &to_insert);
        ASSERT_SIZE_T(*ret, i);

        // check if item was overwritten
        const size_t* item = string_map_get(&map, &insert_key);
        ASSERT_SIZE_T(*item, i);
    }

    free_string_map(&map);
}

TEST(remove) {
    enum {
        INIT_CAP = 20,
        NUM_INSERTS = INIT_CAP * 2
    };

    struct string_map map = create_string_map(sizeof(size_t),
                                              INIT_CAP,
                                              false,
                                              NULL);

    char keys[NUM_INSERTS][KEY_CHAR_ARR_LEN];

    const size_t written = write_str_combos(keys, NUM_INSERTS, KEY_STR_LEN);
    ASSERT_SIZE_T(written, (size_t)NUM_INSERTS);

    insert_items(&map, keys, NUM_INSERTS);

    for (size_t i = 0; i < NUM_INSERTS; ++i) {
        const struct str key = str_from_generated(keys[i]);
        string_map_remove(&map, &key);
        ASSERT_SIZE_T(map._len, (size_t)NUM_INSERTS - 1);

        const void* item = string_map_get(&map, &key);
        ASSERT_NULL(item);
        for (size_t j = 0; j < NUM_INSERTS; ++j) {
            if (j == i) {
                continue;
            }
            const struct str other_key = str_from_generated(keys[j]);
            const size_t* ret = string_map_get(&map, &other_key);
            ASSERT_NOT_NULL(ret);
            ASSERT_SIZE_T(*ret, j);
        }

        const size_t* insert_ret = string_map_insert(&map, &key, &i);
        ASSERT_SIZE_T(*insert_ret, i);
        ASSERT_SIZE_T(map._len, (size_t)NUM_INSERTS);
        for (size_t j = 0; j < NUM_INSERTS; ++j) {
            const struct str other_key = str_from_generated(keys[j]);
            const size_t* ret = string_map_get(&map, &other_key);
            ASSERT_NOT_NULL(ret);
            ASSERT_SIZE_T(*ret, j);
        }
    }

    for (size_t i = 0; i < NUM_INSERTS; ++i) {
        ASSERT_SIZE_T(map._len, (size_t)NUM_INSERTS - i);
        const struct str to_remove = str_from_generated(keys[i]);
        string_map_remove(&map, &to_remove);

        const void* item = string_map_get(&map, &to_remove);
        ASSERT_NULL(item);
        for (size_t j = i + 1; j < NUM_INSERTS; ++j) {
            const struct str other_key = str_from_generated(keys[j]);
            const size_t* ret = string_map_get(&map, &other_key);
            ASSERT_NOT_NULL(ret);
            ASSERT_SIZE_T(*ret, j);
        }
    }

    free_string_map(&map);
}

TEST_SUITE_BEGIN(string_map){
    REGISTER_TEST(insert),
    REGISTER_TEST(remove),
} TEST_SUITE_END()
