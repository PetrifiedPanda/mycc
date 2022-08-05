#ifndef ABSTRACT_DECL_H
#define ABSTRACT_DECL_H

#include "frontend/parser/parser_state.h"

struct pointer;
struct direct_abs_declarator;

struct abs_declarator {
    struct pointer* ptr;
    struct direct_abs_declarator* direct_abs_decl;
};

struct abs_declarator* parse_abs_declarator(struct parser_state* s);

void free_abs_declarator(struct abs_declarator* d);

#include "pointer.h"
#include "direct_abs_declarator.h"

#endif
