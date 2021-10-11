#ifndef STRUCT_DECLARATOR_H
#define STRUCT_DECLARATOR_H

typedef struct Declarator Declarator;
typedef struct ConstExpr ConstExpr;

typedef struct StructDeclarator {
    Declarator* decl;
    ConstExpr* bit_field;
} StructDeclarator;

StructDeclarator* create_struct_declarator(Declarator* decl, ConstExpr* bit_field);

void free_struct_declarator_children(StructDeclarator* d);
void free_struct_declarator(StructDeclarator* d);

#include "ast/declarator.h"
#include "ast/const_expr.h"

#endif