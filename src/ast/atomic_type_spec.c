#include "ast/atomic_type_spec.h"

void free_atomic_type_spec(struct atomic_type_spec* s) {
    free_type_name(s->type_name);
}
