#include "util/mem.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "util/File.h"

#ifdef MYCC_ENABLE_MEMDEBUG
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
        File_printf(mycc_stderr,
                    "mycc_alloc():\n\tFailed to allocate {size_t} bytes\n",
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
        File_printf(
            mycc_stderr,
            "mycc_alloc_zeroed():\n\tFailed to allocate {size_t} elements of "
            "size {size_t} "
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
        File_printf(mycc_stderr,
                    "mycc_realloc():\n\tFailed to realloc {size_t} bytes\n",
                    bytes);
        exit(EXIT_FAILURE);
    }
    return res;
}

void mycc_grow_alloc(void** alloc, uint32_t* alloc_len, size_t elem_size) {
    uint32_t new_num = *alloc_len + *alloc_len / 2 + 1;
    *alloc = mycc_realloc(*alloc, elem_size * new_num);
    *alloc_len = new_num;
}

void mycc_free(void* alloc) {
    free(alloc);
}

#ifdef MYCC_ENABLE_MEMDEBUG

typedef struct {
    Str func;
    Str file;
    uint32_t line;
} AllocLoc;

typedef struct {
    void* alloc;
    size_t bytes;
    bool freed;
    bool realloced; // if freed by realloc
    AllocLoc alloced_loc;
    AllocLoc freed_loc;
} AllocEntry;

typedef struct {
    uint32_t len, cap;
    AllocEntry* data;
    size_t num_allocs;
    size_t num_frees;
    size_t num_reallocs;
    size_t num_reallocs_without_copy;
    size_t current_memory_usage;
    size_t max_memory_usage;
    size_t bytes_alloced;
    size_t bytes_freed;
} AllocStats;

static bool g_cleanup_func_set = false;
static AllocStats g_alloc_stats = {
    .data = NULL,
    .len = 0,
    .cap = 0,
    .num_allocs = 0,
    .num_frees = 0,
    .num_reallocs = 0,
    .num_reallocs_without_copy = 0,
    .current_memory_usage = 0,
    .max_memory_usage = 0,
    .bytes_alloced = 0,
    .bytes_freed = 0,
};

static uint32_t find_alloc_idx(const AllocStats* stats, void* alloc) {
    assert(alloc != NULL);
    uint32_t left = 0;
    uint32_t right = stats->len;
    while (left < right) {
        const uint32_t middle = (left + right) / 2;
        if (stats->data[middle].alloc < alloc) {
            left = middle + 1;
        } else {
            right = middle;
        }
    }
    return left;
}

static AllocEntry create_alloc_entry(void* alloc,
                                     size_t bytes,
                                     Str func,
                                     Str file,
                                     uint32_t line) {
    assert(alloc != NULL);
    return (AllocEntry){
        .alloc = alloc,
        .bytes = bytes,
        .freed = false,
        .realloced = false,
        .alloced_loc = {func, file, line},
        .freed_loc = {{0, NULL}, {0, NULL}, UINT32_MAX},
    };
}

static void print_surrounding_lines(size_t num) {
    for (size_t i = 0; i < num; ++i) {
        File_putc('-', mycc_stderr);
    }
}

static size_t get_num_digits(size_t n) {
    size_t num_digits = 0;
    size_t val = n;
    while (val != 0) {
        val /= 10;
        ++num_digits;
    }
    return num_digits;
}

static size_t pow_size_t(size_t x, size_t n) {
    size_t res = 1;
    for (size_t i = 0; i < n; ++i) {
        res *= x;
    }
    return res;
}

static void pretty_print_size_t(File out, size_t n) {
    if (n == 0) {
        File_putc('0', out);
        return;
    }
    const size_t num_digits = get_num_digits(n);
    size_t remainder = n;
    size_t num_digits_to_print = num_digits - 1;
    size_t pow = pow_size_t(10, num_digits_to_print);
    while (num_digits_to_print != 0) {
        const int to_print = (int)(remainder / pow);
        remainder -= pow * to_print;
        File_putc('0' + (char)to_print, out);
        if (num_digits_to_print % 3 == 0) {
            File_putc(',', out);
        }
        --num_digits_to_print;
        pow /= 10;
    }
    File_putc('0' + (char)remainder, out);
}

static void memdebug_cleanup(void) {
    enum {
        LINE_COUNT = 20
    };
    print_surrounding_lines(LINE_COUNT);
    File_put_str("MEMDEBUG REPORT", mycc_stderr);
    print_surrounding_lines(LINE_COUNT);
    File_putc('\n', mycc_stderr);
    bool leak_detected = false;
    for (size_t i = 0; i < g_alloc_stats.len; ++i) {
        AllocEntry* curr = &g_alloc_stats.data[i];
        if (!curr->freed) {
            leak_detected = true;
            File_put_str("Leak detected:\n", mycc_stderr);
            File_printf(mycc_stderr,
                        "\t{ptr} of size {size_t} allocated in {Str} in "
                        "{Str}:{u32} was never freed\n",
                        curr->alloc,
                        curr->bytes,
                        curr->alloced_loc.func,
                        curr->alloced_loc.file,
                        curr->alloced_loc.line);
        }
    }
    File_put_str("Of ", mycc_stderr);
    pretty_print_size_t(mycc_stderr, g_alloc_stats.num_allocs);
    File_put_str(" allocations, ", mycc_stderr);
    pretty_print_size_t(mycc_stderr, g_alloc_stats.num_frees);
    File_put_str(" were freed\n", mycc_stderr);

    File_put_str("Of ", mycc_stderr);
    pretty_print_size_t(mycc_stderr, g_alloc_stats.bytes_alloced);
    File_put_str(" bytes allocated, ", mycc_stderr);
    pretty_print_size_t(mycc_stderr, g_alloc_stats.bytes_freed);
    File_put_str(" bytes were freed\n", mycc_stderr);

    File_put_str("Of ", mycc_stderr);
    pretty_print_size_t(mycc_stderr, g_alloc_stats.num_reallocs);
    File_put_str(" realloc calls, ", mycc_stderr);
    pretty_print_size_t(mycc_stderr, g_alloc_stats.num_reallocs_without_copy);
    File_put_str(" resized an existing allocation\n", mycc_stderr);

    File_put_str("Max memory usage: ", mycc_stderr);
    pretty_print_size_t(mycc_stderr, g_alloc_stats.max_memory_usage);
    File_put_str(" bytes\n", mycc_stderr);
    mycc_free(g_alloc_stats.data);
    if (leak_detected) {
        _Exit(EXIT_FAILURE);
    }
}

static void insert_alloc(AllocStats* stats,
                         void* alloc,
                         size_t bytes,
                         Str func,
                         Str file,
                         uint32_t line) {
    assert(alloc != NULL);
    const uint32_t idx = find_alloc_idx(stats, alloc);
    if (idx >= stats->len || stats->data[idx].alloc != alloc) {
        if (stats->len == stats->cap) {
            mycc_grow_alloc((void**)&stats->data,
                            &stats->cap,
                            sizeof *stats->data);
        }

        for (size_t i = stats->len; i > idx; --i) {
            assert(stats->data[i - 1].alloc > alloc);
            stats->data[i] = stats->data[i - 1];
        }
        ++stats->len;
    }

    stats->data[idx] = create_alloc_entry(alloc, bytes, func, file, line);
    stats->bytes_alloced += bytes;
    stats->current_memory_usage += bytes;
    if (stats->current_memory_usage > stats->max_memory_usage) {
        stats->max_memory_usage = stats->current_memory_usage;
    }
    stats->num_allocs += 1;
    if (!g_cleanup_func_set) {
        atexit(memdebug_cleanup);
        g_cleanup_func_set = true;
    }
}

static void set_freed(AllocStats* stats,
                      uint32_t alloc_idx,
                      bool realloced,
                      Str func,
                      Str file,
                      uint32_t line) {
    AllocEntry* curr = &stats->data[alloc_idx];
    assert(!curr->freed);
    curr->freed = true;
    curr->realloced = realloced;
    curr->freed_loc = (AllocLoc){func, file, line};
    stats->bytes_freed += curr->bytes;
    stats->current_memory_usage -= curr->bytes;
    stats->num_frees += 1;
}

static void set_alloc_bytes(AllocStats* stats,
                            uint32_t alloc_idx,
                            size_t bytes) {
    AllocEntry* curr = &stats->data[alloc_idx];
    assert(!curr->freed);
    stats->num_reallocs_without_copy += 1;
    if (bytes > curr->bytes) {
        const size_t byte_difference = bytes - curr->bytes;
        stats->bytes_alloced += byte_difference;
        stats->current_memory_usage += byte_difference;
        if (stats->current_memory_usage > stats->max_memory_usage) {
            stats->max_memory_usage = stats->current_memory_usage;
        }
    } else {
        const size_t byte_difference = curr->bytes - bytes;
        stats->bytes_freed += byte_difference;
        stats->current_memory_usage -= byte_difference;
    }
    curr->bytes = bytes;
}

static void check_if_freed(const AllocStats* stats, uint32_t alloc_idx) {
    const AllocEntry* entry = &stats->data[alloc_idx];
    if (entry->freed) {
        File_put_str("Double free detected, exiting...\n", mycc_stderr);
        Str by_realloc = entry->realloced ? STR_LIT(" by realloc")
                                          : STR_LIT("");
        File_printf(mycc_stderr, "\t{ptr} with size ", entry->alloc);
        pretty_print_size_t(mycc_stderr, entry->bytes);
        File_printf(mycc_stderr,
                    " was already freed{Str} in {Str} in {Str}:{size_t}\n",
                    by_realloc,
                    entry->freed_loc.func,
                    entry->freed_loc.file,
                    entry->freed_loc.line);
        exit(EXIT_FAILURE);
    }
}

void* mycc_memdebug_alloc_wrapper(size_t bytes,
                                  Str func,
                                  Str file,
                                  uint32_t line) {
    void* alloc = mycc_alloc(bytes);
    insert_alloc(&g_alloc_stats, alloc, bytes, func, file, line);
    return alloc;
}

void* mycc_memdebug_alloc_zeroed_wrapper(size_t len,
                                         size_t elem_size,
                                         Str func,
                                         Str file,
                                         uint32_t line) {
    void* alloc = mycc_alloc_zeroed(len, elem_size);
    insert_alloc(&g_alloc_stats, alloc, len * elem_size, func, file, line);
    return alloc;
}

void* mycc_memdebug_realloc_wrapper(void* alloc,
                                    size_t bytes,
                                    Str func,
                                    Str file,
                                    uint32_t line) {
    g_alloc_stats.num_reallocs += 1;
    if (alloc == NULL) {
        void* new_alloc = mycc_realloc(alloc, bytes);
        if (new_alloc != NULL) {
            insert_alloc(&g_alloc_stats, new_alloc, bytes, func, file, line);
        }
        return new_alloc;
    } else {
        const uint32_t alloc_idx = find_alloc_idx(&g_alloc_stats, alloc);
        assert(g_alloc_stats.data[alloc_idx].alloc == alloc);
        check_if_freed(&g_alloc_stats, alloc_idx);
        void* new_alloc = mycc_realloc(alloc, bytes);
        if (new_alloc == alloc) {
            set_alloc_bytes(&g_alloc_stats, alloc_idx, bytes);
        } else {
            set_freed(&g_alloc_stats, alloc_idx, true, func, file, line);
            if (new_alloc != NULL) {
                insert_alloc(&g_alloc_stats,
                             new_alloc,
                             bytes,
                             func,
                             file,
                             line);
            }
        }
        return new_alloc;
    }
}

void mycc_memdebug_free_wrapper(void* alloc,
                                Str func,
                                Str file,
                                uint32_t line) {
    if (alloc != NULL) {
        const uint32_t alloc_idx = find_alloc_idx(&g_alloc_stats, alloc);
        assert(g_alloc_stats.data[alloc_idx].alloc == alloc
               && "Tried to free untracked allocation");
        check_if_freed(&g_alloc_stats, alloc_idx);
        mycc_free(alloc);
        set_freed(&g_alloc_stats, alloc_idx, false, func, file, line);
    } else {
        mycc_free(alloc);
    }
}

void mycc_memdebug_grow_alloc_wrapper(void** alloc,
                                      uint32_t* alloc_len,
                                      size_t elem_size,
                                      Str func,
                                      Str file,
                                      uint32_t line) {
    g_alloc_stats.num_reallocs += 1;
    if (*alloc == NULL) {
        mycc_grow_alloc(alloc, alloc_len, elem_size);
        insert_alloc(&g_alloc_stats,
                     *alloc,
                     *alloc_len * elem_size,
                     func,
                     file,
                     line);
    } else {
        const uint32_t alloc_idx = find_alloc_idx(&g_alloc_stats, *alloc);
        assert(g_alloc_stats.data[alloc_idx].alloc == *alloc);
        check_if_freed(&g_alloc_stats, alloc_idx);
        void* old_alloc = *alloc;
        mycc_grow_alloc(alloc, alloc_len, elem_size);
        const size_t bytes = *alloc_len * elem_size;
        if (old_alloc == *alloc) {
            set_alloc_bytes(&g_alloc_stats, alloc_idx, bytes);
        } else {
            set_freed(&g_alloc_stats, alloc_idx, true, func, file, line);
            insert_alloc(&g_alloc_stats, *alloc, bytes, func, file, line);
        }
    }
}

#endif
