#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <stdbool.h>

#include "ast/init_list.h"

struct assign_expr;

struct initializer {
    bool is_assign;
    union {
        struct assign_expr* assign;
        struct init_list init_list;
    };
};

struct initializer* create_initializer_assign(struct assign_expr* assign);
struct initializer* create_initializer_init_list(struct init_list init_list);

void free_initializer_children(struct initializer* i);
void free_initializer(struct initializer* i);

#include "ast/assign_expr.h"

#endif

