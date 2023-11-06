#ifndef MYCC_UTIL_STRING_MAP_H
#define MYCC_UTIL_STRING_MAP_H

#include <stddef.h>
#include <stdbool.h>

#include "StrBuf.h"

typedef struct StringMapKey StringMapKey;

typedef struct StringMap {
    uint32_t _len;
    uint32_t _cap;
    uint32_t _item_size;
    bool _free_keys;
    void (*_item_free)(void*);
    StringMapKey* _keys;
    void* _items;
} StringMap;

StringMap StringMap_create(uint32_t elem_size,
                           uint32_t init_cap,
                           bool free_keys,
                           void (*item_free)(void*));
void StringMap_free(StringMap* map);

void StringMap_clear(StringMap* map);

/**
 * @brief Inserts item and key into this map, if key is not already
 *        present
 *
 * @return If this key is not already in use, item, otherwise a pointer
 *         to the item associated with key
 */
const void* StringMap_insert(StringMap* map,
                             const StrBuf* key,
                             const void* item);

/**
 * @brief Inserts item and key into this map, overwriting the key
 *        if it is already present
 *
 * @return true if an existing entry was overwritten
 */
bool StringMap_insert_overwrite(StringMap* map,
                                const StrBuf* key,
                                const void* item);

/**
 * @brief Gets the item with the given key
 *
 * @return A pointer to the item associated with key, or null, if key is not
 *         present
 */
const void* StringMap_get(const StringMap* map, Str key);

void StringMap_remove(StringMap* map, const StrBuf* key);

#endif
