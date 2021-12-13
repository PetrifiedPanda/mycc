#ifndef ABSTRACT_DECL_H
#define ABSTRACT_DECL_H

struct pointer;
struct direct_abstract_declarator;

struct abstract_declarator {
    struct pointer* ptr;
    struct direct_abstract_declarator* direct_abs_decl;
};

struct abstract_declarator* create_abstract_declarator(struct pointer* ptr, struct direct_abstract_declarator* direct_abs_decl);

void free_abstract_declarator(struct abstract_declarator* d);

#include "ast/pointer.h"
#include "ast/direct_abstract_declarator.h"

#endif

