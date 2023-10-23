#ifndef MYCC_UTIL_MEM_H
#define MYCC_UTIL_MEM_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Calls malloc(), exiting when malloc() fails
 *
 * @param bytes Size of allocation
 * @return void* Pointer to allocated storage
 */
void* mycc_alloc(size_t bytes);

/**
 * @brief calls calloc(), exiting when calloc() fails
 *
 * @param len Number of elements to allocate
 * @param elem_size Size of one element in bytes
 * @return void* Pointer to allocated storage
 */
void* mycc_alloc_zeroed(size_t len, size_t elem_size);

/**
 * @brief Calls realloc(), exiting when realloc() fails
 * If bytes is zero, the given buffer is freed and NULL is returned
 *
 * @param alloc Existing allocation, or NULL
 * @param bytes New size for the allocation
 * @return void* Resized allocation
 */
void* mycc_realloc(void* alloc, size_t bytes);

/**
 * @brief Wrapper for free()
 */
void mycc_free(void* alloc);

/**
 * @brief Grows an existing allocation, writing the new allocation and its size
 * in the given pointers
 *
 * @param alloc Pointer to existing allocation, to which the resulting
 * allocation will be written
 * @param alloc_len Pointer to number of allocated elements, to which the new
 * number of elements will be written
 * @param elem_size Size of one element in bytes
 */
void mycc_grow_alloc(void** alloc, uint32_t* alloc_len, size_t elem_size);

#ifdef MYCC_ENABLE_MEMDEBUG

#include "util/Str.h"

void* mycc_memdebug_alloc_wrapper(size_t bytes,
                                  Str func,
                                  Str file,
                                  uint32_t line);
void* mycc_memdebug_alloc_zeroed_wrapper(size_t len,
                                         size_t elem_size,
                                         Str func,
                                         Str file,
                                         uint32_t line);
void* mycc_memdebug_realloc_wrapper(void* alloc,
                                    size_t bytes,
                                    Str func,
                                    Str file,
                                    uint32_t line);

void mycc_memdebug_free_wrapper(void* alloc, Str func, Str file, uint32_t line);

void mycc_memdebug_grow_alloc_wrapper(void** alloc,
                                      uint32_t* alloc_len,
                                      size_t elem_size,
                                      Str func,
                                      Str file,
                                      uint32_t line);

#define mycc_alloc(bytes)                                                      \
    mycc_memdebug_alloc_wrapper(bytes,                                         \
                                STR_LIT(__func__),                             \
                                STR_LIT(__FILE__),                             \
                                __LINE__)
#define mycc_alloc_zeroed(len, elem_size)                                      \
    mycc_memdebug_alloc_zeroed_wrapper(len,                                    \
                                       elem_size,                              \
                                       STR_LIT(__func__),                      \
                                       STR_LIT(__FILE__),                      \
                                       __LINE__)

#define mycc_realloc(alloc, bytes)                                             \
    mycc_memdebug_realloc_wrapper(alloc,                                       \
                                  bytes,                                       \
                                  STR_LIT(__func__),                           \
                                  STR_LIT(__FILE__),                           \
                                  __LINE__)
#define mycc_free(alloc)                                                       \
    mycc_memdebug_free_wrapper(alloc,                                          \
                               STR_LIT(__func__),                              \
                               STR_LIT(__FILE__),                              \
                               __LINE__)

#define mycc_grow_alloc(alloc, alloc_len, elem_size)                           \
    mycc_memdebug_grow_alloc_wrapper(alloc,                                    \
                                     alloc_len,                                \
                                     elem_size,                                \
                                     STR_LIT(__func__),                        \
                                     STR_LIT(__FILE__),                        \
                                     __LINE__)

#endif

#endif
