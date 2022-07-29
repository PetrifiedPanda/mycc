#ifndef AST_STRING_LITERAL_H
#define AST_STRING_LITERAL_H

struct string_literal {
    char* spelling;
};

void free_string_literal(struct string_literal* l);

#endif

