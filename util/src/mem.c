#include "util/mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifdef MYCC_MEMDEBUG
#undef mycc_alloc
#undef mycc_alloc_zeroed
#undef mycc_realloc
#undef mycc_free
#undef mycc_grow_alloc
#endif

void* mycc_alloc(size_t bytes) {
    assert(bytes != 0);

    void* res = malloc(bytes);
    if (!res) {
        fprintf(stderr,
                "mycc_alloc():\n\tFailed to allocate %zu bytes\n",
                bytes);
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
                "mycc_alloc_zeroed():\n\tFailed to allocate %zu elements of "
                "size %zu "
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
        fprintf(stderr,
                "mycc_realloc():\n\tFailed to realloc %zu bytes\n",
                bytes);
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

#ifdef MYCC_MEMDEBUG

struct alloc_loc {
    const char* file;
    size_t line;
};

struct alloc_entry {
    void* alloc;
    size_t bytes;
    bool freed;
    bool realloced; // if freed by realloc
    struct alloc_loc alloced_loc;
    struct alloc_loc freed_loc;
};

static bool g_cleanup_func_set = false;
static struct {
    struct alloc_entry* data;
    size_t len;
    size_t cap;
    size_t num_allocs;
    size_t num_frees;
    size_t bytes_alloced;
    size_t bytes_freed;
} g_alloc_stats = {
    .data = NULL,
    .len = 0,
    .cap = 0,
    .num_allocs = 0,
    .num_frees = 0,
    .bytes_alloced = 0,
    .bytes_freed = 0,
};

static size_t find_alloc_idx(void* alloc) {
    assert(alloc != NULL);
    if (g_alloc_stats.len == 0) {
        return 0;
    }
    size_t left = 0;
    size_t right = g_alloc_stats.len == 0 ? 0 : g_alloc_stats.len - 1;
    while (left <= right) {
        const size_t middle = floor((left + right) / 2.0);
        if (g_alloc_stats.data[middle].alloc < alloc) {
            left = middle + 1;
        } else if (g_alloc_stats.data[middle].alloc > alloc) {
            right = middle - 1;
        } else {
            return middle;
        }
    }
    return left;
}

static struct alloc_entry create_alloc_entry(void* alloc,
                                             size_t bytes,
                                             const char* file,
                                             size_t line) {
    return (struct alloc_entry){
        .alloc = alloc,
        .bytes = bytes,
        .freed = false,
        .realloced = false,
        .alloced_loc = {file, line},
        .freed_loc = {NULL, (size_t)-1},
    };
}

static void print_surrounding_lines(size_t num) {
    for (size_t i = 0; i < num; ++i) {
        fputc('-', stderr);
    }
}

static void memdebug_cleanup(void) {
    enum {
        LINE_COUNT = 20
    };
    print_surrounding_lines(LINE_COUNT);
    fputs("MEMDEBUG REPORT", stderr);
    print_surrounding_lines(LINE_COUNT);
    fputc('\n', stderr);
    for (size_t i = 0; i < g_alloc_stats.len; ++i) {
        struct alloc_entry* curr = &g_alloc_stats.data[i];
        if (!curr->freed) {
            fputs("Leak detected:\n", stderr);
            fprintf(stderr,
                    "\t%p of size %zu allocated in %s:%zu was never freed\n",
                    curr->alloc,
                    curr->bytes,
                    curr->alloced_loc.file,
                    curr->alloced_loc.line);
        }
    }
    // TODO: FIX STATS
    fprintf(stderr,
            "Of %zu allocations, %zu were freed\n",
            g_alloc_stats.num_allocs,
            g_alloc_stats.num_frees);
    fprintf(stderr,
            "Of %zu bytes allocated, %zu were freed\n",
            g_alloc_stats.bytes_alloced,
            g_alloc_stats.bytes_freed);
    mycc_free(g_alloc_stats.data);
}

static void insert_alloc(void* alloc,
                         size_t bytes,
                         const char* file,
                         size_t line) {
    assert(alloc != NULL);
    const size_t idx = find_alloc_idx(alloc);
    if (idx >= g_alloc_stats.len || g_alloc_stats.data[idx].alloc != alloc) {
        if (g_alloc_stats.len == g_alloc_stats.cap) {
            mycc_grow_alloc((void**)&g_alloc_stats.data,
                            &g_alloc_stats.cap,
                            sizeof *g_alloc_stats.data);
        }

        for (size_t i = g_alloc_stats.len; i > idx; --i) {
            assert(g_alloc_stats.data[i - 1].alloc > alloc);
            g_alloc_stats.data[i] = g_alloc_stats.data[i - 1];
        }
        ++g_alloc_stats.len;
    }

    g_alloc_stats.data[idx] = create_alloc_entry(alloc, bytes, file, line);
    g_alloc_stats.bytes_alloced += bytes;
    g_alloc_stats.num_allocs += 1;
    if (!g_cleanup_func_set) {
        atexit(memdebug_cleanup);
        g_cleanup_func_set = true;
    }
}

static void set_freed(size_t alloc_idx,
                      bool realloced,
                      const char* file,
                      size_t line) {
    struct alloc_entry* curr = &g_alloc_stats.data[alloc_idx];
    assert(!curr->freed);
    curr->freed = true;
    curr->realloced = realloced;
    curr->freed_loc = (struct alloc_loc){file, line};
    g_alloc_stats.bytes_freed += curr->bytes;
    g_alloc_stats.num_frees += 1;
}

static void set_alloc_bytes(size_t alloc_idx, size_t bytes) {
    struct alloc_entry* curr = &g_alloc_stats.data[alloc_idx];
    assert(!curr->freed);
    if (bytes > curr->bytes) {
        g_alloc_stats.bytes_alloced += bytes - curr->bytes;
    } else {
        g_alloc_stats.bytes_freed += curr->bytes - bytes;
    }
    curr->bytes = bytes;
}

static void check_if_freed(size_t alloc_idx) {
    const struct alloc_entry* entry = &g_alloc_stats.data[alloc_idx];
    if (entry->freed) {
        fprintf(stderr, "Double free detected, exiting...\n");
        const char* by_realloc = entry->realloced ? " by realloc" : "";
        fprintf(stderr,
                "\t%p with size %zu was already freed%s in %s:%zu\n",
                entry->alloc,
                entry->bytes,
                by_realloc,
                entry->freed_loc.file,
                entry->freed_loc.line);
        exit(EXIT_FAILURE);
    }
}

void* mycc_memdebug_alloc_wrapper(size_t bytes, const char* file, size_t line) {
    void* alloc = mycc_alloc(bytes);
    insert_alloc(alloc, bytes, file, line);
    return alloc;
}

void* mycc_memdebug_alloc_zeroed_wrapper(size_t len,
                                         size_t elem_size,
                                         const char* file,
                                         size_t line) {
    void* alloc = mycc_alloc_zeroed(len, elem_size);
    insert_alloc(alloc, len * elem_size, file, line);
    return alloc;
}

void* mycc_memdebug_realloc_wrapper(void* alloc,
                                    size_t bytes,
                                    const char* file,
                                    size_t line) {
    if (alloc == NULL) {
        void* new_alloc = mycc_realloc(alloc, bytes);
        if (new_alloc != NULL) {
            insert_alloc(new_alloc, bytes, file, line);
        }
        return new_alloc;
    } else {
        const size_t alloc_idx = find_alloc_idx(alloc);
        assert(g_alloc_stats.data[alloc_idx].alloc == alloc);
        check_if_freed(alloc_idx);
        void* new_alloc = mycc_realloc(alloc, bytes);
        if (new_alloc == alloc) {
            set_alloc_bytes(alloc_idx, bytes);
        } else {
            set_freed(alloc_idx, true, file, line);
            if (new_alloc != NULL) {
                insert_alloc(new_alloc, bytes, file, line);
            }
        }
        return new_alloc;
    }
}

void mycc_memdebug_free_wrapper(void* alloc, const char* file, size_t line) {
    if (alloc != NULL) {
        const size_t alloc_idx = find_alloc_idx(alloc);
        assert(g_alloc_stats.data[alloc_idx].alloc == alloc);
        check_if_freed(alloc_idx);
        mycc_free(alloc);
        set_freed(alloc_idx, false, file, line);
    } else {
        mycc_free(alloc);
    }
}

void mycc_memdebug_grow_alloc_wrapper(void** alloc,
                                      size_t* alloc_len,
                                      size_t elem_size,
                                      const char* file,
                                      size_t line) {
    if (*alloc == NULL) {
        mycc_grow_alloc(alloc, alloc_len, elem_size);
        insert_alloc(*alloc, *alloc_len * elem_size, file, line);
    } else {
        const size_t alloc_idx = find_alloc_idx(*alloc);
        assert(g_alloc_stats.data[alloc_idx].alloc == *alloc);
        check_if_freed(alloc_idx);
        void* old_alloc = *alloc;
        mycc_grow_alloc(alloc, alloc_len, elem_size);
        const size_t bytes = *alloc_len * elem_size;
        if (old_alloc == *alloc) {
            set_alloc_bytes(alloc_idx, bytes);
        } else {
            set_freed(alloc_idx, true, file, line);
            insert_alloc(*alloc, bytes, file, line);
        }
    }
}

#endif
