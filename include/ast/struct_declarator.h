#ifndef STRUCT_DECLARATOR_H
#define STRUCT_DECLARATOR_H

struct declarator;
struct const_expr;

struct struct_declarator {
    struct declarator* decl;
    struct const_expr* bit_field;
};

struct struct_declarator* create_struct_declarator(struct declarator* decl, struct const_expr* bit_field);

void free_struct_declarator_children(struct struct_declarator* d);
void free_struct_declarator(struct struct_declarator* d);

#include "ast/declarator.h"
#include "ast/const_expr.h"

#endif

