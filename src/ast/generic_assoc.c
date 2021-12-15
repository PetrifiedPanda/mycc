#include "ast/generic_assoc.h"

void free_generic_assoc_children(struct generic_assoc* a) {
    if (a->type_name) {
        free_type_name(a->type_name);
    }
    free_assign_expr(a->assign);
}