#ifndef DECLARATOR_H
#define DECLARATOR_H

#include "frontend/parser/parser_state.h"

struct pointer;
struct direct_declarator;

struct declarator {
    struct pointer* ptr;
    struct direct_declarator* direct_decl;
};

struct declarator* parse_declarator(struct parser_state* s);
struct declarator* parse_declarator_typedef(struct parser_state* s);

void free_declarator(struct declarator* d);

#include "pointer.h"
#include "direct_declarator.h"

#endif

