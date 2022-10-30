#include "frontend/ast/expr/constant.h"

#include <stdlib.h>
#include <assert.h>

struct constant create_int_constant(struct int_value val,
                                    struct source_loc loc) {
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = CONSTANT_INT,
        .int_val = val,
    };
}

struct constant create_float_constant(struct float_value val,
                                      struct source_loc loc) {
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = CONSTANT_FLOAT,
        .float_val = val,
    };
}

struct constant create_enum_constant(const struct str* spelling,
                                     struct source_loc loc) {
    assert(spelling);
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = CONSTANT_ENUM,
        .spelling = *spelling,
    };
}

void free_constant(struct constant* c) {
    if (c->type == CONSTANT_ENUM) {
        free_str(&c->spelling);
    }
}
