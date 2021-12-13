#ifndef DECLARATION_H
#define DECLARATION_H

#include "ast/init_declarator_list.h"

struct declaration_specs;
struct static_assert_declaration;

struct declaration {
    bool is_normal_decl;
    union {
        struct {
            struct declaration_specs *decl_specs;
            struct init_declarator_list init_decls;
        };
        struct static_assert_declaration* static_assert_decl;
    };
};

struct declaration* create_declaration(struct declaration_specs* decl_specs, struct init_declarator_list init_decls);

struct declaration* create_declaration_assert(struct static_assert_declaration* static_assert_decl);

void free_declaration_children(struct declaration* d);
void free_declaration(struct declaration* d);

#include "ast/declaration_specs.h"
#include "ast/static_assert_declaration.h"

#endif

