#include "util/string_hash_map.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"

struct string_hash_map create_string_hash_map(size_t elem_size) {
    enum {
        INIT_LEN = 100
    };
    return (struct string_hash_map){
        ._len = 0,
        ._cap = INIT_LEN,
        ._item_size = elem_size,
        ._keys = xcalloc(INIT_LEN, sizeof(const char*)),
        ._items = xcalloc(INIT_LEN, elem_size),
    };
}

void free_string_hash_map(struct string_hash_map* map) {
    free((void*)map->_keys);
    free(map->_items);
}

static size_t hash_string(const char* str);
static void resize_map(struct string_hash_map* map);

const void* string_hash_map_insert(struct string_hash_map* map,
                                   const char* key,
                                   const void* item) {
    assert(key);

    if (map->_len == map->_cap) {
        resize_map(map);
    }

    const size_t hash = hash_string(key);
    size_t i = hash != 0 ? hash % map->_cap : hash;
    while (map->_keys[i] != NULL && strcmp(map->_keys[i], key) != 0) {
        ++i;
        if (i == map->_cap) {
            i = 0;
        }
    }

    void* found = (char*)map->_items + i * map->_item_size;
    if (map->_keys[i] != NULL) {
        return found;
    }

    map->_keys[i] = key;
    memcpy(found, item, map->_item_size);
    ++map->_len;
    return item;
}

const void* string_hash_map_get(const struct string_hash_map* map,
                                const char* key) {
    assert(key);

    const size_t hash = hash_string(key);
    size_t i = hash != 0 ? hash % map->_cap : hash;
    while (map->_keys[i] != NULL && strcmp(map->_keys[i], key) != 0) {
        ++i;
        if (i == map->_cap) {
            i = 0;
        }
    }

    if (map->_keys[i] == NULL) {
        return NULL;
    }

    return (char*)map->_items + i * map->_item_size;
}

static void resize_map(struct string_hash_map* map) {
    const size_t prev_cap = map->_cap;
    const size_t prev_len = map->_len;
    const char** old_keys = map->_keys;
    void* old_items = map->_items;

    map->_len = 0;
    map->_cap += map->_cap / 2 + 1;
    map->_keys = xcalloc(map->_cap, sizeof(const char*));
    map->_items = xcalloc(map->_cap, map->_item_size);

    for (size_t i = 0; i < prev_cap; ++i) {
        if (old_keys[i] != NULL) {
            bool success = string_hash_map_insert(map,
                                                  old_keys[i],
                                                  (char*)old_items
                                                      + i * map->_item_size);
            UNUSED(success);
            assert(success);
        }
    }

    UNUSED(prev_len);
    assert(map->_len == prev_len);
    free(old_items);
    free((void*)old_keys);
}

// Hash function taken from K&R version 2 (page 144)
static size_t hash_string(const char* str) {
    size_t hash = 0;

    const char* it = str;
    while (*it != '\0') {
        hash = *it + 31 * hash;
        ++it;
    }

    return hash;
}

