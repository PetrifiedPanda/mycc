#include "util/string_hash_map.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/annotations.h"

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
            if (map->_keys[i].str) {
                void* item = items_char + i * map->_item_size;
                map->_item_free(item);
            }
        }
    }

    free(map->_items);

    if (map->_free_keys) {
        for (size_t i = 0; i < map->_cap; ++i) {
            free(map->_keys[i].str);
        }
    }
    free(map->_keys);
}

static size_t hash_string(const char* str);
static void resize_map(struct string_hash_map* map);

static size_t find_item_index_insert(const struct string_hash_map* map,
                                     const char* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    bool found_deleted = false;
    size_t deleted_idx;
    size_t it_count = 0;
    while (it_count != map->_cap
           && (map->_keys[i].was_deleted
               || (map->_keys[i].str != NULL
                   && strcmp(map->_keys[i].str, key) != 0))) {
        if (map->_keys[i].was_deleted && !found_deleted) {
            deleted_idx = i;
            found_deleted = true;
        }
        i = (i + 1) % map->_cap;
        ++it_count;
    }

    if (map->_keys[i].str == NULL && found_deleted) {
        return deleted_idx;
    } else {
        return i;
    }
}

static size_t find_item_index(const struct string_hash_map* map,
                              const char* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    while (
        map->_keys[i].was_deleted
        || (map->_keys[i].str != NULL && strcmp(map->_keys[i].str, key) != 0)) {
        i = (i + 1) % map->_cap;
    }

    return i;
}

const void* string_hash_map_insert(struct string_hash_map* map,
                                   char* key,
                                   const void* item) {
    assert(key);
    assert(item);

    if (map->_len == map->_cap) {
        resize_map(map);
    }

    const size_t idx = find_item_index_insert(map, key);

    void* found = (char*)map->_items + idx * map->_item_size;
    if (map->_keys[idx].str != NULL) {
        return found;
    }

    map->_keys[idx] = (struct string_hash_map_key){
        .was_deleted = false,
        .str = key,
    };
    memcpy(found, item, map->_item_size);
    ++map->_len;
    return item;
}

void string_hash_map_insert_overwrite(struct string_hash_map* map,
                                      char* key,
                                      const void* item) {
    assert(key);
    assert(item);

    if (map->_len == map->_cap) {
        resize_map(map);
    }

    const size_t idx = find_item_index_insert(map, key);
    
    void* curr_item = (char*)map->_items + idx * map->_item_size;
    if (map->_keys[idx].str != NULL) {
        if (map->_free_keys) {
            free(key);
        }
        
        if (map->_item_free) {
            map->_item_free(curr_item);
        }
    } else {
        map->_keys[idx] = (struct string_hash_map_key) {
            .was_deleted = false,
            .str = key,
        };
    }

    memcpy(curr_item, item, map->_item_size);
    ++map->_len;
}



const void* string_hash_map_get(const struct string_hash_map* map,
                                const char* key) {
    assert(key);
    const size_t idx = find_item_index(map, key);

    if (map->_keys[idx].str == NULL) {
        return NULL;
    }

    return (char*)map->_items + idx * map->_item_size;
}

void string_hash_map_remove(struct string_hash_map* map, const char* key) {
    const size_t idx = find_item_index(map, key);

    char* key_to_remove = map->_keys[idx].str;
    if (key_to_remove == NULL) {
        return;
    }
    if (map->_free_keys) {
        free(key_to_remove);
    }
    map->_keys[idx] = (struct string_hash_map_key){
        .was_deleted = true,
        .str = NULL,
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
        if (old_keys[i].str != NULL) {
            const void* success = string_hash_map_insert(
                map,
                old_keys[i].str,
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
static size_t hash_string(const char* str) {
    size_t hash = 0;

    const char* it = str;
    while (*it != '\0') {
        hash = *it + 31 * hash;
        ++it;
    }

    return hash;
}
