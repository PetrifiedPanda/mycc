#ifndef INIT_DECLARATOR_H
#define INIT_DECLARATOR_H

struct declarator;
struct initializer;

struct init_declarator {
    struct declarator* decl;
    struct initializer* init;
};

struct init_declarator* create_init_declarator(struct declarator* decl, struct initializer* init);

void free_init_declarator_children(struct init_declarator* d);

#include "ast/declarator.h"
#include "ast/initializer.h"

#endif

