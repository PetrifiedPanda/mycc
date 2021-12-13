#ifndef STRUCT_DECLARATION_H
#define STRUCT_DECLARATION_H

// TODO: update with new grammar

#include "ast/spec_qual_list.h"
#include "ast/struct_declarator_list.h"

struct struct_declaration {
    struct spec_qual_list spec_qual_list;
    struct struct_declarator_list decls;
};

struct struct_declaration* create_struct_declaration(struct spec_qual_list spec_qual_list, struct struct_declarator_list decls);

void free_struct_declaration_children(struct struct_declaration* d);
void free_struct_declaration(struct struct_declaration* d);

#endif
