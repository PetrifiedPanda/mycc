#ifndef AST_STRING_LITERAL_H
#define AST_STRING_LITERAL_H

typedef struct {
    char* spelling;
} StringLiteral;

StringLiteral create_string_literal(char* spelling);

void free_string_literal(StringLiteral* l);

#endif
