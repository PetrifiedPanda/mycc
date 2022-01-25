#ifndef INIT_DECLARATOR_H
#define INIT_DECLARATOR_H

#include "parser/parser_state.h"

struct declarator;
struct initializer;

struct init_declarator {
    struct declarator* decl;
    struct initializer* init;
};

struct init_declarator* parse_init_declarator(struct parser_state* s);

void free_init_declarator_children(struct init_declarator* d);

#include "ast/declarator.h"
#include "ast/initializer.h"

#endif

