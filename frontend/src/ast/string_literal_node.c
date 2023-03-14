#include "frontend/ast/string_literal_node.h"

struct string_literal_node create_string_literal(const struct str* spelling,
                                                 struct source_loc loc) {
    return (struct string_literal_node){
        .info = create_ast_node_info(loc),
        .spelling = *spelling,
    };
}

void free_string_literal(struct string_literal_node* l) {
    free_str(&l->spelling);
}
