#ifndef ENUMERATOR_H
#define ENUMERATOR_H

// TODO: update with new grammar

#include "ast/const_expr.h"

typedef struct Identifier Identifier;

typedef struct Enumerator {
    Identifier* identifier;
    ConstExpr* enum_val;
} Enumerator;

Enumerator* create_enumerator(Identifier* identifier, ConstExpr* enum_val);

void free_enumerator_children(Enumerator* e);

#include "ast/identifier.h"

#endif

