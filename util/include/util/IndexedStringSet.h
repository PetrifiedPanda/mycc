#ifndef MYCC_UTIL_INDEXED_STRING_SET
#define MYCC_UTIL_INDEXED_STRING_SET

#include "util/StrBuf.h"

typedef struct IndexedStringSet {
    uint32_t _len, _cap;
    uint32_t* _indices;
    StrBuf* _data;
} IndexedStringSet;

IndexedStringSet IndexedStringSet_create(uint32_t init_cap);

void IndexedStringSet_free(const IndexedStringSet* s);

uint32_t IndexedStringSet_find_or_insert(IndexedStringSet* s, Str str);

Str IndexedStringSet_get(const IndexedStringSet* s, uint32_t idx);

#endif
