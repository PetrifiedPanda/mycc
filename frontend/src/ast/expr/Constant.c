#include "frontend/ast/expr/Constant.h"

#include <assert.h>

Constant create_int_constant(IntValue val,
                                    SourceLoc loc) {
    return (Constant){
        .info = create_ast_node_info(loc),
        .kind = CONSTANT_INT,
        .int_val = val,
    };
}

Constant create_float_constant(FloatValue val,
                                      SourceLoc loc) {
    return (Constant){
        .info = create_ast_node_info(loc),
        .kind = CONSTANT_FLOAT,
        .float_val = val,
    };
}

Constant create_enum_constant(const Str* spelling,
                                     SourceLoc loc) {
    assert(spelling);
    return (Constant){
        .info = create_ast_node_info(loc),
        .kind = CONSTANT_ENUM,
        .spelling = *spelling,
    };
}

void free_constant(Constant* c) {
    if (c->kind == CONSTANT_ENUM) {
        Str_free(&c->spelling);
    }
}
