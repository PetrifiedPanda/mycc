#include "util/string_hash_map.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/annotations.h"

struct string_hash_map_key {
    bool was_deleted;
    struct str str;
};

struct string_hash_map create_string_hash_map(size_t elem_size,
                                              size_t init_cap,
                                              bool free_keys,
                                              void (*item_free)(void*)) {
    return (struct string_hash_map){
        ._len = 0,
        ._cap = init_cap,
        ._item_size = elem_size,
        ._free_keys = free_keys,
        ._item_free = item_free,
        ._keys = xcalloc(init_cap, sizeof(struct string_hash_map_key)),
        ._items = xcalloc(init_cap, elem_size),
    };
}

void free_string_hash_map(struct string_hash_map* map) {
    if (map->_item_free) {
        char* items_char = map->_items;
        for (size_t i = 0; i < map->_cap; ++i) {
            if (str_is_valid(&map->_keys[i].str)) {
                void* item = items_char + i * map->_item_size;
                map->_item_free(item);
            }
        }
    }

    free(map->_items);

    if (map->_free_keys) {
        for (size_t i = 0; i < map->_cap; ++i) {
            free_str(&map->_keys[i].str);
        }
    }
    free(map->_keys);
}

static size_t hash_string(const struct str* str);
static void resize_map(struct string_hash_map* map);

static size_t find_item_index_insert(const struct string_hash_map* map,
                                     const struct str* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    bool found_deleted = false;
    size_t deleted_idx = (size_t)-1;
    size_t it_count = 0;
    while (
        it_count != map->_cap
        && (map->_keys[i].was_deleted
            || (str_is_valid(&map->_keys[i].str)
                && strcmp(str_get_data(&map->_keys[i].str), str_get_data(key))
                       != 0))) {
        if (map->_keys[i].was_deleted && !found_deleted) {
            deleted_idx = i;
            found_deleted = true;
        }
        i = (i + 1) % map->_cap;
        ++it_count;
    }
    
    if (!str_is_valid(&map->_keys[i].str) && found_deleted) {
        return deleted_idx;
    } else {
        return i;
    }
}

static size_t find_item_index(const struct string_hash_map* map,
                              const struct str* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    while (map->_keys[i].was_deleted
           || (str_is_valid(&map->_keys[i].str)
               && strcmp(str_get_data(&map->_keys[i].str), str_get_data(key))
                      != 0)) {
        i = (i + 1) % map->_cap;
    }

    return i;
}

static void rehash_if_necessary(struct string_hash_map* map) {
    if (map->_len == map->_cap) {
        resize_map(map);
    }
}

const void* string_hash_map_insert(struct string_hash_map* map,
                                   const struct str* key,
                                   const void* item) {
    assert(key);
    assert(item);
    rehash_if_necessary(map);

    const size_t idx = find_item_index_insert(map, key);

    void* found = (char*)map->_items + idx * map->_item_size;
    if (str_is_valid(&map->_keys[idx].str)) {
        return found;
    }

    map->_keys[idx] = (struct string_hash_map_key){
        .was_deleted = false,
        .str = *key,
    };
    memcpy(found, item, map->_item_size);
    ++map->_len;

    return item;
}

bool string_hash_map_insert_overwrite(struct string_hash_map* map,
                                      const struct str* key,
                                      const void* item) {
    assert(key);
    assert(item);

    rehash_if_necessary(map);

    const size_t idx = find_item_index_insert(map, key);

    bool overwritten;
    void* curr_item = (char*)map->_items + idx * map->_item_size;
    if (str_is_valid(&map->_keys[idx].str)) {
        if (map->_free_keys) {
            free_str(key);
        }

        if (map->_item_free) {
            map->_item_free(curr_item);
        }
        overwritten = true;
    } else {
        map->_keys[idx] = (struct string_hash_map_key){
            .was_deleted = false,
            .str = *key,
        };
        overwritten = false;
    }

    memcpy(curr_item, item, map->_item_size);
    ++map->_len;
    return overwritten;
}

const void* string_hash_map_get(const struct string_hash_map* map,
                                const struct str* key) {
    assert(key);
    const size_t idx = find_item_index(map, key);

    if (!str_is_valid(&map->_keys[idx].str)) {
        return NULL;
    }

    return (char*)map->_items + idx * map->_item_size;
}

void string_hash_map_remove(struct string_hash_map* map,
                            const struct str* key) {
    const size_t idx = find_item_index(map, key);

    struct str* key_to_remove = &map->_keys[idx].str;
    if (key_to_remove == NULL) {
        return;
    }
    if (map->_free_keys) {
        free_str(key_to_remove);
    }
    map->_keys[idx] = (struct string_hash_map_key){
        .was_deleted = true,
        .str = create_null_str(),
    };

    void* item_to_remove = (char*)map->_items + idx * map->_item_size;
    if (map->_item_free) {
        map->_item_free(item_to_remove);
    }
    --map->_len;
}

static void resize_map(struct string_hash_map* map) {
    const size_t prev_cap = map->_cap;
    const size_t prev_len = map->_len;
    struct string_hash_map_key* old_keys = map->_keys;
    void* old_items = map->_items;

    map->_len = 0;
    map->_cap += map->_cap / 2 + 1;
    map->_keys = xcalloc(map->_cap, sizeof(struct string_hash_map_key));
    map->_items = xcalloc(map->_cap, map->_item_size);

    for (size_t i = 0; i < prev_cap; ++i) {
        if (str_is_valid(&old_keys[i].str)) {
            const void* success = string_hash_map_insert(
                map,
                &old_keys[i].str,
                (char*)old_items + i * map->_item_size);
            UNUSED(success);
            assert(success != NULL);
        }
    }

    UNUSED(prev_len);
    assert(map->_len == prev_len);
    free(old_items);
    free((void*)old_keys);
}

// Hash function taken from K&R version 2 (page 144)
static size_t hash_string(const struct str* str) {
    size_t hash = 0;

    const char* it = str_get_data(str);
    const char* limit = it + str_len(str);
    while (it != limit) {
        hash = *it + 31 * hash;
        ++it;
    }

    return hash;
}

