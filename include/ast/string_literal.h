#ifndef AST_STRING_LITERAL_H
#define AST_STRING_LITERAL_H

#include <stdbool.h>

struct string_literal {
    char* spelling;
};

void free_string_literal(struct string_literal* l);

struct ast_visitor;

bool visit_string_literal(struct ast_visitor* visitor,
                          struct string_literal* l);

#endif

