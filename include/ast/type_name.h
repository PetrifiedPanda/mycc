#ifndef TYPE_NAME_H
#define TYPE_NAME_H

struct spec_qual_list;

#include "parser/parser_state.h"

struct abs_declarator;

struct type_name {
    struct spec_qual_list* spec_qual_list;
    struct abs_declarator* abstract_decl;
};

bool parse_type_name_inplace(struct parser_state* s, struct type_name* res);
struct type_name* parse_type_name(struct parser_state* s);

void free_type_name_children(struct type_name* n);
void free_type_name(struct type_name* n);

#include "ast/spec_qual_list.h"
#include "ast/declaration/abs_declarator.h"

#endif
