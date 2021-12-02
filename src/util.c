#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "error.h"

void* xmalloc(size_t bytes) {
    void* res = malloc(bytes);
    if (!res) {
        fprintf(stderr, "xmalloc():\n\tFailed to allocate %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void* xrealloc(void* alloc, size_t bytes) {
    void* res = realloc(alloc, bytes);
    if (!res) {
        fprintf(stderr, "xrealloc():\n\tFailed to realloc %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void grow_alloc(void** alloc, size_t* num_elems, size_t elem_size) {
    size_t new_num = *num_elems + *num_elems / 2 + 1;
    *alloc = xrealloc(*alloc, elem_size * new_num);
    *num_elems = new_num;
}

char* alloc_string_copy(const char* str) {
    assert(str);
    char* res = xmalloc(sizeof(char) * (strlen(str) + 1));
    strcpy(res, str);
    return res;
}
