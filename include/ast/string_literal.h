#ifndef AST_STRING_LITERAL_H
#define AST_STRING_LITERAL_H

#include <stdbool.h>

struct string_literal {
    bool is_func;
    char* spelling;
};

struct string_literal create_string_literal(char* spelling);
struct string_literal create_func_name();

void free_string_literal(struct string_literal* l);

#endif
