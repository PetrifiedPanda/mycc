#include "frontend/ast/StringLiteralNode.h"

StringLiteralNode create_string_literal_node(const StrLit* lit, SourceLoc loc) {
    return (StringLiteralNode){
        .info = create_ast_node_info(loc),
        .lit = *lit,
    };
}

void free_string_literal(StringLiteralNode* l) {
    free_str_lit(&l->lit);
}
