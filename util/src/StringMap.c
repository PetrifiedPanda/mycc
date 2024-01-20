#include "util/StringMap.h"

#include <string.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

typedef struct StringMapKey {
    bool was_deleted;
    StrBuf str;
} StringMapKey;

StringMap StringMap_create(uint32_t elem_size,
                           uint32_t init_cap,
                           bool free_keys,
                           void (*item_free)(void*)) {
    void* keys = mycc_alloc_zeroed(init_cap, sizeof(StringMapKey) + elem_size);
    void* items = (char*)keys + sizeof(StringMapKey) * init_cap;
    return (StringMap){
        ._len = 0,
        ._cap = init_cap,
        ._item_size = elem_size,
        ._free_keys = free_keys,
        ._item_free = item_free,
        ._keys = keys,
        ._items = items,
    };
}

static void StringMap_free_elements(StringMap* map) {
    if (map->_item_free) {
        char* items_char = map->_items;
        for (uint32_t i = 0; i < map->_cap; ++i) {
            if (StrBuf_valid(&map->_keys[i].str)) {
                void* item = items_char + i * map->_item_size;
                map->_item_free(item);
            }
        }
    }

    if (map->_free_keys) {
        for (uint32_t i = 0; i < map->_cap; ++i) {
            StrBuf_free(&map->_keys[i].str);
        }
    }
}

void StringMap_free(StringMap* map) {
    StringMap_free_elements(map);
    mycc_free(map->_keys);
}

void StringMap_clear(StringMap* map) {
    StringMap_free_elements(map);
    memset(map->_items, 0, map->_item_size * map->_cap);
    memset(map->_keys, 0, sizeof *map->_keys * map->_cap);
    map->_len = 0;
}

static uint32_t hash_string(Str str);
static void resize_map(StringMap* map);

static uint32_t find_item_index_insert(const StringMap* map, Str key) {
    const uint32_t hash = hash_string(key);
    uint32_t i = hash % map->_cap;
    bool found_deleted = false;
    uint32_t deleted_idx = UINT32_MAX;
    uint32_t it_count = 0;
    while (it_count != map->_cap
           && (map->_keys[i].was_deleted
               || (StrBuf_valid(&map->_keys[i].str)
                   && !Str_eq(StrBuf_as_str(&map->_keys[i].str), key)))) {
        if (map->_keys[i].was_deleted && !found_deleted) {
            deleted_idx = i;
            found_deleted = true;
        }
        i = (i + 1) % map->_cap;
        ++it_count;
    }

    if (!StrBuf_valid(&map->_keys[i].str) && found_deleted) {
        return deleted_idx;
    } else {
        return i;
    }
}

static uint32_t find_item_index(const StringMap* map, Str key) {
    const uint32_t hash = hash_string(key);
    uint32_t i = hash % map->_cap;
    while (map->_keys[i].was_deleted
           || (StrBuf_valid(&map->_keys[i].str)
               && !Str_eq(StrBuf_as_str(&map->_keys[i].str), key))) {
        i = (i + 1) % map->_cap;
    }

    return i;
}

static void rehash_if_necessary(StringMap* map) {
    if (map->_len == map->_cap - 1) {
        resize_map(map);
    }
}

const void* StringMap_insert(StringMap* map,
                             const StrBuf* key,
                             const void* item) {
    assert(key);
    assert(item);
    rehash_if_necessary(map);

    const uint32_t idx = find_item_index_insert(map, StrBuf_as_str(key));

    void* found = (char*)map->_items + idx * map->_item_size;
    if (StrBuf_valid(&map->_keys[idx].str)) {
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

bool StringMap_insert_overwrite(StringMap* map,
                                const StrBuf* key,
                                const void* item) {
    assert(key);
    assert(item);

    rehash_if_necessary(map);

    const uint32_t idx = find_item_index_insert(map, StrBuf_as_str(key));

    bool overwritten;
    void* curr_item = (char*)map->_items + idx * map->_item_size;
    if (StrBuf_valid(&map->_keys[idx].str)) {
        if (map->_free_keys) {
            StrBuf_free(key);
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

const void* StringMap_get(const StringMap* map, Str key) {
    assert(Str_valid(key));
    const uint32_t idx = find_item_index(map, key);

    if (!StrBuf_valid(&map->_keys[idx].str)) {
        return NULL;
    }

    return (char*)map->_items + idx * map->_item_size;
}

void StringMap_remove(StringMap* map, const StrBuf* key) {
    const uint32_t idx = find_item_index(map, StrBuf_as_str(key));

    StrBuf* key_to_remove = &map->_keys[idx].str;
    if (key_to_remove == NULL) {
        return;
    }
    if (map->_free_keys) {
        StrBuf_free(key_to_remove);
    }
    map->_keys[idx] = (StringMapKey){
        .was_deleted = true,
        .str = StrBuf_null(),
    };

    void* item_to_remove = (char*)map->_items + idx * map->_item_size;
    if (map->_item_free) {
        map->_item_free(item_to_remove);
    }
    --map->_len;
}

static void resize_map(StringMap* map) {
    const uint32_t prev_cap = map->_cap;
    const uint32_t prev_len = map->_len;
    StringMapKey* old_keys = map->_keys;
    void* old_items = map->_items;

    map->_len = 0;
    map->_cap += map->_cap / 2 + 1;
    map->_keys = mycc_alloc_zeroed(map->_cap, sizeof *map->_keys + map->_item_size);
    map->_items = (char*)map->_keys + sizeof *map->_keys * map->_cap;

    for (uint32_t i = 0; i < prev_cap; ++i) {
        if (StrBuf_valid(&old_keys[i].str)) {
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
    mycc_free(old_keys);
}

// Hash function taken from K&R version 2 (page 144)
static uint32_t hash_string(Str str) {
    uint32_t hash = 0;

    uint32_t i = 0;
    while (i != str.len) {
        hash = Str_at(str, i) + 32 * hash;
        ++i;
    }
    return hash;
}

