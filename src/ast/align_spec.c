#include "ast/align_spec.h"

void free_align_spec(struct align_spec* s) {
    if (s->is_type_name) {
        free_type_name(s->type_name);
    } else {
        free_const_expr(s->const_expr);
    }
}
