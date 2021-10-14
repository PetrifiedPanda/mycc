#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <stdbool.h>

#include "ast/init_list.h"

typedef struct AssignExpr AssignExpr;

typedef struct Initializer {
    bool is_assign;
    union {
        AssignExpr* assign;
        InitList init_list;
    };
} Initializer;

Initializer* create_initializer_assign(AssignExpr* assign);
Initializer* create_initializer_init_list(InitList init_list);

void free_initializer_children(Initializer* i);
void free_initializer(Initializer* i);

#include "ast/assign_expr.h"

#endif
