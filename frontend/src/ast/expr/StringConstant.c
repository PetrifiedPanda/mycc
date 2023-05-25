#include "frontend/ast/expr/StringConstant.h"

#include <assert.h>

StringConstant create_string_constant(const StrLit* lit, SourceLoc loc) {
    assert(lit);
    return (StringConstant){
        .is_func = false,
        .lit = create_string_literal_node(lit, loc),
    };
}

StringConstant create_func_name(SourceLoc loc) {
    return (StringConstant){
        .is_func = true,
        .info = create_ast_node_info(loc),
    };
}

void free_string_constant(StringConstant* c) {
    if (!c->is_func) {
        free_string_literal(&c->lit);
    }
}

