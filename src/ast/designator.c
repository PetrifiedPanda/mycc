#include "ast/designator.h"

void free_designator_children(struct designator* d) {
    if (d->is_index) {
        free_const_expr(d->arr_index);
    } else {
        free_identifier(d->identifier);
    }
}
