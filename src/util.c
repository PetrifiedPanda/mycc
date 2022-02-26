#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void* xmalloc(size_t bytes) {
    assert(bytes != 0);

    void* res = malloc(bytes);
    if (!res) {
        fprintf(stderr, "xmalloc():\n\tFailed to allocate %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void* xcalloc(size_t len, size_t elem_size) {
    if (len == 0 || elem_size == 0) {
        return NULL;
    }

    void* res = calloc(len, elem_size);
    if (!res) {
        fprintf(stderr,
                "xcalloc():\n\tFailed to allocate %zu elements of size %zu "
                "bytes each\n",
                len,
                elem_size);
        exit(EXIT_FAILURE);
    }
    return res;
}

void* xrealloc(void* alloc, size_t bytes) {
    if (bytes == 0) {
        free(alloc);
        return NULL;
    }

    void* res = realloc(alloc, bytes);
    if (!res) {
        fprintf(stderr, "xrealloc():\n\tFailed to realloc %zu bytes\n", bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void grow_alloc(void** alloc, size_t* alloc_len, size_t elem_size) {
    size_t new_num = *alloc_len + *alloc_len / 2 + 1;
    *alloc = xrealloc(*alloc, elem_size * new_num);
    *alloc_len = new_num;
}

char* alloc_string_copy(const char* str) {
    assert(str);
    char* res = xmalloc(sizeof(char) * (strlen(str) + 1));
    strcpy(res, str);
    return res;
}

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    assert(f);

    int result;

    result = fseek(f, 0, SEEK_END);
    assert(result == 0);

    size_t fsize = ftell(f);
    result = fseek(f, 0, SEEK_SET);
    assert(result == 0);

    char* res = malloc(sizeof(char) * (fsize + 1));
    assert(res);

    size_t chars_read = fread(res, 1, fsize, f);
    assert(chars_read == fsize);
    res[fsize] = '\0';

    fclose(f);
    return res;
}

