#include "util/mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void* mycc_alloc(size_t bytes) {
    assert(bytes != 0);

    void* res = malloc(bytes);
    if (!res) {
        fprintf(stderr, "mycc_alloc():\n\tFailed to allocate %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void* mycc_alloc_zeroed(size_t len, size_t elem_size) {
    assert(len != 0);
    assert(elem_size != 0);

    void* res = calloc(len, elem_size);
    if (!res) {
        fprintf(stderr,
                "mycc_alloc_zeroed():\n\tFailed to allocate %zu elements of size %zu "
                "bytes each\n",
                len,
                elem_size);
        exit(EXIT_FAILURE);
    }
    return res;
}

void* mycc_realloc(void* alloc, size_t bytes) {
    if (bytes == 0) {
        free(alloc);
        return NULL;
    }

    void* res = realloc(alloc, bytes);
    if (!res) {
        fprintf(stderr, "mycc_realloc():\n\tFailed to realloc %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void mycc_grow_alloc(void** alloc, size_t* alloc_len, size_t elem_size) {
    size_t new_num = *alloc_len + *alloc_len / 2 + 1;
    *alloc = mycc_realloc(*alloc, elem_size * new_num);
    *alloc_len = new_num;
}

void mycc_free(void* alloc) {
    free(alloc);
}

