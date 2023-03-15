#ifndef AST_STRING_LITERAL_NODE_H
#define AST_STRING_LITERAL_NODE_H

#include "frontend/ast/ast_node_info.h"

#include "frontend/str_lit.h"

struct string_literal_node {
    struct ast_node_info info;
    struct str_lit lit;
};

struct string_literal_node create_string_literal_node(const struct str_lit* lit,
                                                      struct source_loc loc);

void free_string_literal(struct string_literal_node* l);

#endif

