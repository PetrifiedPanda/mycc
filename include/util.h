#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdbool.h>

void* xmalloc(size_t bytes);

void* xrealloc(void* alloc, size_t bytes);

void grow_alloc(void** alloc, size_t* num_elems, size_t elem_size);

#endif
