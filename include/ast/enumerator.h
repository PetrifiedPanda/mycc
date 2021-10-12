#ifndef ENUMERATOR_H
#define ENUMERATOR_H

#include "ast/const_expr.h"

typedef struct Enumerator {
    char* identifier;
    ConstExpr* enum_val;
} Enumerator;

Enumerator* create_enumerator(char* identifier, ConstExpr* enum_val);

void free_enumerator_children(Enumerator* e);

#endif
