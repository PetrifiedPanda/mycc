#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include <stdbool.h>

struct constant {
    bool is_float;
    char* spelling;
};

struct constant create_constant(bool is_float, char* spelling);

void free_constant(struct constant* c);

#endif
