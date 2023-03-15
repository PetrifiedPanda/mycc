#include "frontend/ast/string_literal_node.h"

struct string_literal_node create_string_literal_node(const struct str_lit* lit,
                                                      struct source_loc loc) {
    return (struct string_literal_node){
        .info = create_ast_node_info(loc),
        .lit = *lit,
    };
}

void free_string_literal(struct string_literal_node* l) {
    free_str_lit(&l->lit);
}
