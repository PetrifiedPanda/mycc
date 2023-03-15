#ifndef AST_CONSTANT_H
#define AST_CONSTANT_H

#include "frontend/ast/ast_node_info.h"

enum constant_kind {
    CONSTANT_ENUM,
    CONSTANT_INT,
    CONSTANT_FLOAT,
};

struct constant {
    struct ast_node_info info;
    enum constant_kind kind;
    union {
        struct str spelling;
        struct int_value int_val;
        struct float_value float_val;
    };
};

struct constant create_int_constant(struct int_value val,
                                    struct source_loc loc);
struct constant create_float_constant(struct float_value val,
                                      struct source_loc loc);

struct constant create_enum_constant(const struct str* spelling,
                                     struct source_loc loc);

void free_constant(struct constant* c);

#endif

