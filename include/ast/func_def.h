#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include "ast/declaration_list.h"

#include "parser/parser_state.h"

struct declaration_specs;
struct declarator;
struct compound_statement;

struct func_def {
    struct declaration_specs* specs;
    struct declarator* decl;
    struct declaration_list decl_list;
    struct compound_statement* comp;
};

bool parse_func_def_inplace(struct parser_state* s, struct func_def* res);

void free_func_def_children(struct func_def* d);
void free_func_def(struct func_def* d);

#include "ast/declaration_specs.h"
#include "ast/declarator.h"
#include "ast/compound_statement.h"

#endif

