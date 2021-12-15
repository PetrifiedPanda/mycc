#include "ast/designation.h"

#include <stdlib.h>

static void free_children(struct designation* d) {
    free_designator_list(&d->designators);
}

void free_designation(struct designation* d) {
    free_children(d);
    free(d);
}
