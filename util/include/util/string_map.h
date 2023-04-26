#ifndef STRING_HASH_MAP_H
#define STRING_HASH_MAP_H

#include <stddef.h>
#include <stdbool.h>

#include "util/str.h"

struct string_map_key;

struct string_map {
    size_t _len;
    size_t _cap;
    size_t _item_size;
    bool _free_keys;
    void (*_item_free)(void*);
    struct string_map_key* _keys;
    void* _items;
};

struct string_map create_string_map(size_t elem_size,
                                    size_t init_cap,
                                    bool free_keys,
                                    void (*item_free)(void*));
void free_string_map(struct string_map* map);

/**
 * @brief Inserts item and key into this map, if key is not already
 *        present
 *
 * @return If this key is not already in use, item, otherwise a pointer
 *         to the item associated with key
 */
const void* string_map_insert(struct string_map* map,
                              const struct str* key,
                              const void* item);

/**
 * @brief Inserts item and key into this map, overwriting the key
 *        if it is already present
 *
 * @return true if an existing entry was overwritten
 */
bool string_map_insert_overwrite(struct string_map* map,
                                 const struct str* key,
                                 const void* item);

/**
 * @brief Gets the item with the given key
 *
 * @return A pointer to the item associated with key, or null, if key is not
 *         present
 */
const void* string_map_get(const struct string_map* map, const struct str* key);

void string_map_remove(struct string_map* map, const struct str* key);

#endif
