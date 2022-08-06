#include "frontend/ast/string_literal.h"

#include <stdlib.h>

struct string_literal create_string_literal(char* spelling, struct source_loc loc) {
    return (struct string_literal){
        .info = create_ast_node_info(loc),
        .spelling = spelling,
    };
}

void free_string_literal(struct string_literal* l) {
    free_ast_node_info(&l->info);
    free(l->spelling);
}
