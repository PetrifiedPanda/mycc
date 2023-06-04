#include "util/StringMap.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

typedef struct StringMapKey {
    bool was_deleted;
    Str str;
} StringMapKey;

StringMap StringMap_create(size_t elem_size,
                                    size_t init_cap,
                                    bool free_keys,
                                    void (*item_free)(void*)) {
    return (StringMap){
        ._len = 0,
        ._cap = init_cap,
        ._item_size = elem_size,
        ._free_keys = free_keys,
        ._item_free = item_free,
        ._keys = mycc_alloc_zeroed(init_cap, sizeof(StringMapKey)),
        ._items = mycc_alloc_zeroed(init_cap, elem_size),
    };
}

void StringMap_free(StringMap* map) {
    if (map->_item_free) {
        char* items_char = map->_items;
        for (size_t i = 0; i < map->_cap; ++i) {
            if (Str_is_valid(&map->_keys[i].str)) {
                void* item = items_char + i * map->_item_size;
                map->_item_free(item);
            }
        }
    }

    mycc_free(map->_items);

    if (map->_free_keys) {
        for (size_t i = 0; i < map->_cap; ++i) {
            Str_free(&map->_keys[i].str);
        }
    }
    mycc_free(map->_keys);
}

static size_t hash_string(const Str* str);
static void resize_map(StringMap* map);

static size_t find_item_index_insert(const StringMap* map, const Str* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    bool found_deleted = false;
    size_t deleted_idx = (size_t)-1;
    size_t it_count = 0;
    while (it_count != map->_cap
           && (map->_keys[i].was_deleted
               || (Str_is_valid(&map->_keys[i].str)
                   && !Str_eq(&map->_keys[i].str, key)))) {
        if (map->_keys[i].was_deleted && !found_deleted) {
            deleted_idx = i;
            found_deleted = true;
        }
        i = (i + 1) % map->_cap;
        ++it_count;
    }

    if (!Str_is_valid(&map->_keys[i].str) && found_deleted) {
        return deleted_idx;
    } else {
        return i;
    }
}

static size_t find_item_index(const StringMap* map, const Str* key) {
    const size_t hash = hash_string(key);
    size_t i = hash % map->_cap;
    while (map->_keys[i].was_deleted
           || (Str_is_valid(&map->_keys[i].str)
               && !Str_eq(&map->_keys[i].str, key))) {
        i = (i + 1) % map->_cap;
    }

    return i;
}

static void rehash_if_necessary(StringMap* map) {
    if (map->_len == map->_cap) {
        resize_map(map);
    }
}

const void* StringMap_insert(StringMap* map,
                              const Str* key,
                              const void* item) {
    assert(key);
    assert(item);
    rehash_if_necessary(map);

    const size_t idx = find_item_index_insert(map, key);

    void* found = (char*)map->_items + idx * map->_item_size;
    if (Str_is_valid(&map->_keys[idx].str)) {
        return found;
    }

    map->_keys[idx] = (StringMapKey){
        .was_deleted = false,
        .str = *key,
    };
    memcpy(found, item, map->_item_size);
    ++map->_len;

    return item;
}

bool StringMap_insert_overwrite(StringMap* map, const Str* key, const void* item) {
    assert(key);
    assert(item);

    rehash_if_necessary(map);

    const size_t idx = find_item_index_insert(map, key);

    bool overwritten;
    void* curr_item = (char*)map->_items + idx * map->_item_size;
    if (Str_is_valid(&map->_keys[idx].str)) {
        if (map->_free_keys) {
            Str_free(key);
        }

        if (map->_item_free) {
            map->_item_free(curr_item);
        }
        overwritten = true;
    } else {
        map->_keys[idx] = (StringMapKey){
            .was_deleted = false,
            .str = *key,
        };
        overwritten = false;
    }

    memcpy(curr_item, item, map->_item_size);
    ++map->_len;
    return overwritten;
}

const void* StringMap_get(const StringMap* map, const Str* key) {
    assert(key);
    const size_t idx = find_item_index(map, key);

    if (!Str_is_valid(&map->_keys[idx].str)) {
        return NULL;
    }

    return (char*)map->_items + idx * map->_item_size;
}

void StringMap_remove(StringMap* map, const Str* key) {
    const size_t idx = find_item_index(map, key);

    Str* key_to_remove = &map->_keys[idx].str;
    if (key_to_remove == NULL) {
        return;
    }
    if (map->_free_keys) {
        Str_free(key_to_remove);
    }
    map->_keys[idx] = (StringMapKey){
        .was_deleted = true,
        .str = Str_create_null(),
    };

    void* item_to_remove = (char*)map->_items + idx * map->_item_size;
    if (map->_item_free) {
        map->_item_free(item_to_remove);
    }
    --map->_len;
}

static void resize_map(StringMap* map) {
    const size_t prev_cap = map->_cap;
    const size_t prev_len = map->_len;
    StringMapKey* old_keys = map->_keys;
    void* old_items = map->_items;

    map->_len = 0;
    map->_cap += map->_cap / 2 + 1;
    map->_keys = mycc_alloc_zeroed(map->_cap, sizeof *map->_keys);
    map->_items = mycc_alloc_zeroed(map->_cap, map->_item_size);

    for (size_t i = 0; i < prev_cap; ++i) {
        if (Str_is_valid(&old_keys[i].str)) {
            const void* success = StringMap_insert(map,
                                                    &old_keys[i].str,
                                                    (char*)old_items
                                                        + i * map->_item_size);
            UNUSED(success);
            assert(success != NULL);
        }
    }

    UNUSED(prev_len);
    assert(map->_len == prev_len);
    mycc_free(old_items);
    mycc_free((void*)old_keys);
}

// Hash function taken from K&R version 2 (page 144)
static size_t hash_string(const Str* str) {
    size_t hash = 0;

    const char* it = Str_get_data(str);
    const char* limit = it + Str_len(str);
    while (it != limit) {
        hash = *it + 31 * hash;
        ++it;
    }

    return hash;
}

