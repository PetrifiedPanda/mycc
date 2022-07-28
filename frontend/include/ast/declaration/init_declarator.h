#ifndef INIT_DECLARATOR_H
#define INIT_DECLARATOR_H

#include "parser/parser_state.h"

struct declarator;
struct initializer;

struct init_declarator {
    struct declarator* decl;
    struct initializer* init;
};

bool parse_init_declarator_typedef_inplace(struct parser_state* s,
                                           struct init_declarator* res);
bool parse_init_declarator_inplace(struct parser_state* s,
                                   struct init_declarator* res);

void free_init_declarator_children(struct init_declarator* d);

#include "ast/declaration/declarator.h"

#include "ast/initializer/initializer.h"

#endif

