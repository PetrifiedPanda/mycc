#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include "token_type.h"

struct constant {
    enum token_type type; // ENUM, F_CONSTANT or I_CONSTANT
    char* spelling;
};

struct constant create_constant(enum token_type, char* spelling);

void free_constant(struct constant* c);

#endif

