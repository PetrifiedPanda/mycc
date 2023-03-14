#ifndef AST_STRING_LITERAL_H
#define AST_STRING_LITERAL_H

#include "frontend/ast/ast_node_info.h"

struct string_literal_node {
    struct ast_node_info info;
    struct str spelling;
};

struct string_literal_node create_string_literal(const struct str* spelling,
                                                 struct source_loc loc);

void free_string_literal(struct string_literal_node* l);

#endif

