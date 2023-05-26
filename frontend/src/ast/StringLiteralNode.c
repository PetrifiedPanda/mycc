#include "frontend/ast/StringLiteralNode.h"

StringLiteralNode StringLiteralNode_create(const StrLit* lit, SourceLoc loc) {
    return (StringLiteralNode){
        .info = AstNodeInfo_create(loc),
        .lit = *lit,
    };
}

void StringLiteralNode_free(StringLiteralNode* l) {
    StrLit_free(&l->lit);
}
