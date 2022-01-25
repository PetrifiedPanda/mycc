#ifndef STRUCT_DECLARATOR_H
#define STRUCT_DECLARATOR_H

#include "parser/parser_state.h"

struct declarator;
struct const_expr;

struct struct_declarator {
    struct declarator* decl;
    struct const_expr* bit_field;
};

struct struct_declarator* parse_struct_declarator(struct parser_state* s);

void free_struct_declarator_children(struct struct_declarator* d);
void free_struct_declarator(struct struct_declarator* d);

#include "ast/declarator.h"
#include "ast/const_expr.h"

#endif

