#ifndef STRING_HASH_MAP_H
#define STRING_HASH_MAP_H

#include <stddef.h>
#include <stdbool.h>

struct string_hash_map_key {
    bool was_deleted;
    char* str;
};

struct string_hash_map {
    size_t _len;
    size_t _cap;
    size_t _item_size;
    bool _free_keys;
    void (*_item_free)(void*);
    struct string_hash_map_key* _keys;
    void* _items;
};

struct string_hash_map create_string_hash_map(size_t elem_size,
                                              size_t init_cap,
                                              bool free_keys,
                                              void (*item_free)(void*));
void free_string_hash_map(struct string_hash_map* map);

/**
 * @brief Inserts item and key into this map, if key is not already
 *        present
 *
 * @return If this key is not already in use, item, otherwise a pointer
 *         to the item associated with key
 */
const void* string_hash_map_insert(struct string_hash_map* map,
                                   char* key,
                                   const void* item);

/**
 * @brief Inserts item and key into this map, overwriting the key
 *        if it is already present
 *
 */
void string_hash_map_insert_overwrite(struct string_hash_map* map,
                                      char* key,
                                      const void* item);

/**
 * @brief Gets the item with the given key
 *
 * @return A pointer to the item associated with key, or null, if key is not
 *         present
 */
const void* string_hash_map_get(const struct string_hash_map* map,
                                const char* key);

void string_hash_map_remove(struct string_hash_map* map, const char* key);

#endif
