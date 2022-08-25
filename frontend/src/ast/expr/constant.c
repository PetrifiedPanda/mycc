#include "frontend/ast/expr/constant.h"

#include <stdlib.h>
#include <assert.h>

struct constant create_constant(enum constant_type type,
                                char* spelling,
                                struct source_loc loc) {
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = type,
        .spelling = spelling,
    };
}

void free_constant(struct constant* c) {
    free(c->spelling);
}
