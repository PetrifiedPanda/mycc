#include "frontend/ast/expr/string_constant.h"

#include <assert.h>

struct string_constant create_string_constant(const struct str* spelling, struct source_loc loc) {
    assert(spelling);
    return (struct string_constant){
        .is_func = false,
        .lit = create_string_literal(spelling, loc),
    };
}

struct string_constant create_func_name(struct source_loc loc) {
    return (struct string_constant){
        .is_func = true,
        .info = create_ast_node_info(loc),
    };
}

void free_string_constant(struct string_constant* c) {
    if (!c->is_func) {
        free_string_literal(&c->lit);
    }
}

