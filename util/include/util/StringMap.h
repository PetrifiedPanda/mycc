#ifndef STRING_HASH_MAP_H
#define STRING_HASH_MAP_H

#include <stddef.h>
#include <stdbool.h>

#include "Str.h"

typedef struct StringMapKey StringMapKey;

typedef struct {
    size_t _len;
    size_t _cap;
    size_t _item_size;
    bool _free_keys;
    void (*_item_free)(void*);
    StringMapKey* _keys;
    void* _items;
} StringMap;

StringMap create_string_map(size_t elem_size,
                                    size_t init_cap,
                                    bool free_keys,
                                    void (*item_free)(void*));
void free_string_map(StringMap* map);

/**
 * @brief Inserts item and key into this map, if key is not already
 *        present
 *
 * @return If this key is not already in use, item, otherwise a pointer
 *         to the item associated with key
 */
const void* string_map_insert(StringMap* map,
                              const Str* key,
                              const void* item);

/**
 * @brief Inserts item and key into this map, overwriting the key
 *        if it is already present
 *
 * @return true if an existing entry was overwritten
 */
bool string_map_insert_overwrite(StringMap* map,
                                 const Str* key,
                                 const void* item);

/**
 * @brief Gets the item with the given key
 *
 * @return A pointer to the item associated with key, or null, if key is not
 *         present
 */
const void* string_map_get(const StringMap* map, const Str* key);

void string_map_remove(StringMap* map, const Str* key);

#endif
