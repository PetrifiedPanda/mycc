#ifndef ABSTRACT_DECL_H
#define ABSTRACT_DECL_H

struct pointer;
struct direct_abs_declarator;

struct abs_declarator {
    struct pointer* ptr;
    struct direct_abs_declarator* direct_abs_decl;
};

struct abs_declarator* create_abs_declarator(struct pointer* ptr, struct direct_abs_declarator* direct_abs_decl);

void free_abs_declarator(struct abs_declarator* d);

#include "ast/pointer.h"
#include "ast/direct_abs_declarator.h"

#endif

