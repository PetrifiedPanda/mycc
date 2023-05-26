#include "frontend/ast/expr/Constant.h"

#include <assert.h>

Constant Constant_create_int(IntValue val,
                                    SourceLoc loc) {
    return (Constant){
        .info = AstNodeInfo_create(loc),
        .kind = CONSTANT_INT,
        .int_val = val,
    };
}

Constant Constant_create_float(FloatValue val,
                                      SourceLoc loc) {
    return (Constant){
        .info = AstNodeInfo_create(loc),
        .kind = CONSTANT_FLOAT,
        .float_val = val,
    };
}

Constant Constant_create_enum(const Str* spelling,
                                     SourceLoc loc) {
    assert(spelling);
    return (Constant){
        .info = AstNodeInfo_create(loc),
        .kind = CONSTANT_ENUM,
        .spelling = *spelling,
    };
}

void Constant_free(Constant* c) {
    if (c->kind == CONSTANT_ENUM) {
        Str_free(&c->spelling);
    }
}
