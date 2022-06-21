#ifndef STRING_HASH_MAP_H
#define STRING_HASH_MAP_H

#include <stddef.h>

struct string_hash_map {
    size_t _len;
    size_t _cap;
    size_t _item_size;
    const char** _keys;
    void* _items;
};

struct string_hash_map create_string_hash_map(size_t elem_size);
void free_string_hash_map(struct string_hash_map* map);

/**
 * @brief Inserts item and key into this map, if key is not already
 *        present
 *
 * @return If this key is not already in use, item, otherwise a pointer
 *         to the item associated with key
 */
const void* string_hash_map_insert(struct string_hash_map* map,
                                   const char* key,
                                   const void* item);

/**
 * @brief Gets the item with the given key
 *
 * @return A pointer to the item associated with key, or null, if key is not
 *         present
 */
const void* string_hash_map_get(const struct string_hash_map* map,
                                const char* key);
#endif

