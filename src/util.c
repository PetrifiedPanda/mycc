#include "util.h"

#include <stdlib.h>

#include "error.h"

bool grow_alloc(void** alloc, size_t* num_elems, size_t elem_size) {
    size_t new_num = *num_elems == 1 ? 2 : *num_elems + *num_elems / 2;
    void* tmp = realloc(*alloc, elem_size * new_num);
    if (tmp == NULL) {
        set_error(ERR_ALLOC_FAIL, "Failed to resize allocation from %zu to %zu elements of size %zu bytes", *num_elems, new_num, elem_size);
        return false;
    }
    *alloc = tmp;
    *num_elems = new_num;
    return true;
}
