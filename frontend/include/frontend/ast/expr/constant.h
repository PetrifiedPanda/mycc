#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include "frontend/ast/ast_node_info.h"

struct constant {
    struct ast_node_info info;
    enum token_type type; // ENUM, F_CONSTANT or I_CONSTANT
    char* spelling;
};

struct constant create_constant(enum token_type,
                                char* spelling,
                                struct source_loc loc);

void free_constant(struct constant* c);

#endif

