#ifndef STRUCT_DECLARATION_H
#define STRUCT_DECLARATION_H

#include "ast/spec_qual_list.h"
#include "ast/struct_declarator_list.h"

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

struct struct_declaration* create_struct_declaration(struct spec_qual_list spec_qual_list, struct struct_declarator_list decls);
struct struct_declaration* create_struct_declaration_assert(struct static_assert_declaration* assert_decl);

void free_struct_declaration_children(struct struct_declaration* d);
void free_struct_declaration(struct struct_declaration* d);

#include "ast/static_assert_declaration.h"

#endif
