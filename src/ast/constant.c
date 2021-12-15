#include "ast/constant.h"

#include <stdlib.h>
#include <assert.h>

struct constant create_constant(enum token_type type, char* spelling) {
    assert(type == ENUM || type == F_CONSTANT || type == I_CONSTANT);
    return (struct constant){.type = type, .spelling = spelling};
}

void free_constant(struct constant* c) {
    free(c->spelling);
}
