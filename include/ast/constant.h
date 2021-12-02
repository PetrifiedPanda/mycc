#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

typedef struct {
    char* spelling;
} Constant;

Constant create_constant(char* spelling);

void free_constant(Constant* c);

#endif
