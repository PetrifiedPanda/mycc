#include "ast/designator_list.h"

#include <stdlib.h>

void free_designator_list(struct designator_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_designator_children(&l->designators[i]);
    }
    free(l->designators);
}
