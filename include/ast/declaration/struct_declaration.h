#ifndef STRUCT_DECLARATION_H
#define STRUCT_DECLARATION_H

#include "ast/declaration/struct_declarator_list.h"

#include "parser/parser_state.h"

struct declaration_specs;
struct static_assert_declaration;

struct struct_declaration {
    bool is_static_assert;
    union {
        struct {
            struct declaration_specs* decl_specs;
            struct struct_declarator_list decls;
        };
        struct static_assert_declaration* assert;
    };
};

bool parse_struct_declaration_inplace(struct parser_state* s,
                                      struct struct_declaration* res);

void free_struct_declaration_children(struct struct_declaration* d);

#include "ast/declaration/declaration_specs.h"
#include "ast/declaration/static_assert_declaration.h"

#endif

