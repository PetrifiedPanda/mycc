#include "frontend/ast/expr/constant.h"

#include <stdlib.h>
#include <assert.h>

struct constant create_constant(enum token_type type,
                                char* spelling,
                                struct source_loc loc) {
    assert(type == ENUM || type == F_CONSTANT || type == I_CONSTANT);
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = type,
        .spelling = spelling,
    };
}

void free_constant(struct constant* c) {
    free(c->spelling);
}
