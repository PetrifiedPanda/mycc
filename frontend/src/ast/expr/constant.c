#include "frontend/ast/expr/constant.h"

#include <stdlib.h>
#include <assert.h>

struct constant create_constant(struct value val, struct source_loc loc) {
    enum constant_type t = value_is_float(val.type) ? CONSTANT_FLOAT : CONSTANT_INT;
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = t,
        .val = val,
    };
}

struct constant create_enum_constant(char* spelling, struct source_loc loc) {
    assert(spelling);
    return (struct constant){
        .info = create_ast_node_info(loc),
        .type = CONSTANT_ENUM,
        .spelling = spelling,
    };
}

void free_constant(struct constant* c) {
    free(c->spelling);
}
