#ifndef DECLARATION_H
#define DECLARATION_H

#include "init_declarator_list.h"

#include "frontend/parser/parser_state.h"

struct declaration_specs;
struct static_assert_declaration;

struct declaration {
    bool is_normal_decl;
    union {
        struct {
            struct declaration_specs* decl_specs;
            struct init_declarator_list init_decls;
        };
        struct static_assert_declaration* static_assert_decl;
    };
};

bool parse_declaration_inplace(struct parser_state* s, struct declaration* res);

void free_declaration_children(struct declaration* d);
void free_declaration(struct declaration* d);

#include "declaration_specs.h"
#include "static_assert_declaration.h"

#endif

