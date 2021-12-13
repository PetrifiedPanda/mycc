#ifndef POINTER_H
#define POINTER_H

#include <stddef.h>

struct type_qual_list;

struct pointer {
    size_t num_indirs;
    struct type_qual_list* quals_after_ptr;
};

struct pointer* create_pointer(struct type_qual_list* quals_after_ptr, size_t num_indirs);

void free_pointer(struct pointer* p);

#include "ast/type_qual_list.h"

#endif

