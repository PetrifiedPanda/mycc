#ifndef DECLARATION_H
#define DECLARATION_H

// TODO: update with new grammar

#include "ast/init_declarator_list.h"

struct declaration_specs;

struct declaration {
    struct declaration_specs* decl_specs;
    struct init_declarator_list init_decls;
};

struct declaration* create_declaration(struct declaration_specs* decl_specs, struct init_declarator_list init_decls);

void free_declaration_children(struct declaration* d);
void free_declaration(struct declaration* d);

#include "ast/declaration_specs.h"

#endif

