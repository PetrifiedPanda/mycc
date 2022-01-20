#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Calls malloc(), exiting when malloc() fails
 * 
 * @param bytes Size of allocation
 * @return void* Pointer to allocated storage
 */
void* xmalloc(size_t bytes);

/**
 * @brief Calls realloc(), exiting when realloc() fails
 * If bytes is zero, the given buffer is freed and NULL is returned
 * 
 * @param alloc Existing allocation, or NULL
 * @param bytes New size for the allocation
 * @return void* Resized allocation
 */
void* xrealloc(void* alloc, size_t bytes);

/**
 * @brief Grows an existing allocation, writing the new allocation and its size in the given pointers
 * 
 * @param alloc Pointer to existing allocation, to which the resulting allocation will be written
 * @param alloc_len Pointer to number of allocated elements, to which the new number of elements
 *                  will be written
 * @param elem_size Size of one element in bytes
 */
void grow_alloc(void** alloc, size_t* alloc_len, size_t elem_size);

/**
 * @brief Allocates a copy of the given string
 * 
 * @param str A zero-terminated string that must not be NULL
 * @return char* A heap allocated copy of str
 */
char* alloc_string_copy(const char* str);

#endif
