#include "ast/constant.h"

#include <stdlib.h>

struct constant create_constant(bool is_float, char* spelling) {
    return (struct constant){is_float, spelling};
}

void free_constant(struct constant* c) {
    free(c->spelling);
}
