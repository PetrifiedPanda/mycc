#ifndef ENUMERATOR_H
#define ENUMERATOR_H

struct identifier;
struct const_expr;

struct enumerator {
    struct identifier* identifier;
    struct const_expr* enum_val;
};

struct enumerator* create_enumerator(struct identifier* identifier, struct const_expr* enum_val);

void free_enumerator_children(struct enumerator* e);

#include "ast/identifier.h"
#include "ast/const_expr.h"

#endif

