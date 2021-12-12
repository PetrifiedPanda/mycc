#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include <stdbool.h>

typedef struct {
    bool is_float;
    char* spelling;
} Constant;

Constant create_constant(bool is_float, char* spelling);

void free_constant(Constant* c);

#endif
