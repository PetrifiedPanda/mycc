#include "frontend/ast/expr/StringConstant.h"

#include <assert.h>

StringConstant StringConstant_create(const StrLit* lit, SourceLoc loc) {
    assert(lit);
    return (StringConstant){
        .is_func = false,
        .lit = StringLiteralNode_create(lit, loc),
    };
}

StringConstant StringConstant_create_func_name(SourceLoc loc) {
    return (StringConstant){
        .is_func = true,
        .info = AstNodeInfo_create(loc),
    };
}

void StringConstant_free(StringConstant* c) {
    if (!c->is_func) {
        StringLiteralNode_free(&c->lit);
    }
}

