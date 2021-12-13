#ifndef DECLARATOR_H
#define DECLARATOR_H

struct pointer;
struct direct_declarator;

struct declarator {
    struct pointer* ptr;
    struct direct_declarator* direct_decl;
};

struct declarator* create_declarator(struct pointer* ptr, struct direct_declarator* direct_decl);

void free_declarator(struct declarator* d);

#include "ast/pointer.h"
#include "ast/direct_declarator.h"

#endif

