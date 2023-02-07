#ifndef UTIL_MEM_H
#define UTIL_MEM_H

#include <stddef.h>
#include <stdbool.h>

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
 * @return void* Pointer to allocated storage, or NULL if len is zero
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
void mycc_grow_alloc(void** alloc, size_t* alloc_len, size_t elem_size);

#endif
