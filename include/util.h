#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdbool.h>

bool grow_alloc(void** alloc, size_t* num_elems, size_t elem_size);

#endif
