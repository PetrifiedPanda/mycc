#ifndef TYPE_NAME_H
#define TYPE_NAME_H

#include "ast/spec_qual_list.h"

struct abs_declarator;

struct type_name {
    struct spec_qual_list spec_qual_list;
    struct abs_declarator* abstract_decl;
};

struct type_name* create_type_name(struct spec_qual_list spec_qual_list, struct abs_declarator* abstract_decl);

void free_type_name_children(struct type_name* n);
void free_type_name(struct type_name* n);

#include "ast/abs_declarator.h"

#endif

