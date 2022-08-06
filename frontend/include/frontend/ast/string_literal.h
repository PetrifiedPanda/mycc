#ifndef AST_STRING_LITERAL_H
#define AST_STRING_LITERAL_H

#include "frontend/ast/ast_node_info.h"

struct string_literal {
    struct ast_node_info info;
    char* spelling;
};

struct string_literal create_string_literal(char* spelling, struct source_loc loc);

void free_string_literal(struct string_literal* l);

#endif

