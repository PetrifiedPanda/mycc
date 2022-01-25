#ifndef STRUCT_DECLARATION_H
#define STRUCT_DECLARATION_H

#include "ast/spec_qual_list.h"
#include "ast/struct_declarator_list.h"

#include "parser/parser_state.h"

struct static_assert_declaration;

struct struct_declaration {
    bool is_static_assert;
    union {
        struct {
            struct spec_qual_list spec_qual_list;
            struct struct_declarator_list decls;
        };
        struct static_assert_declaration* assert;
    };
};

bool parse_struct_declaration_inplace(struct parser_state* s, struct struct_declaration* res);

void free_struct_declaration_children(struct struct_declaration* d);
void free_struct_declaration(struct struct_declaration* d);

#include "ast/static_assert_declaration.h"

#endif
