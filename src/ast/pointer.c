#include "ast/pointer.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

Pointer* create_pointer(TypeQualList* quals_after_ptr, size_t num_indirs) {
    assert(num_indirs > 0);
    Pointer* res = xmalloc(sizeof(Pointer));
    res->num_indirs = num_indirs;
    res->quals_after_ptr = quals_after_ptr;
    return res;
}

static void free_children(Pointer* p) {
    for (size_t i = 0; i < p->num_indirs; ++i) {
        free_type_qual_list(&p->quals_after_ptr[i]);
    }
    free(p->quals_after_ptr);
}

void free_pointer(Pointer* p) {
    free_children(p);
    free(p);
}

