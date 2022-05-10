#ifndef DECLARATOR_H
#define DECLARATOR_H

#include "parser/parser_state.h"

struct pointer;
struct direct_declarator;

struct declarator {
    struct pointer* ptr;
    struct direct_declarator* direct_decl;
};

struct declarator* parse_declarator(struct parser_state* s);
struct declarator* parse_declarator_typedef(struct parser_state* s);

void free_declarator(struct declarator* d);

#include "ast/declaration/pointer.h"
#include "ast/declaration/direct_declarator.h"

#endif
