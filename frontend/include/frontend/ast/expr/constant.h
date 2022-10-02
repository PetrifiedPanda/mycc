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
    enum constant_type type;
    union {
        struct str spelling;
        struct value val;
    };
};

struct constant create_constant(struct value val, struct source_loc loc);

struct constant create_enum_constant(const struct str* spelling,
                                     struct source_loc loc);

void free_constant(struct constant* c);

#endif

