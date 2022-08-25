#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include "frontend/ast/ast_node_info.h"

enum constant_type {
    CONSTANT_ENUM,
    CONSTANT_INT,
    CONSTANT_FLOAT,
};

struct constant {
    struct ast_node_info info;
    enum constant_type type; // ENUM, F_CONSTANT or I_CONSTANT
    char* spelling;
};

struct constant create_constant(enum constant_type,
                                char* spelling,
                                struct source_loc loc);

void free_constant(struct constant* c);

#endif

