#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include "declaration_list.h"

struct declaration_specs;
struct declarator;
struct compound_statement;

struct func_def {
    struct declaration_specs* specs;
    struct declarator* decl;
    struct declaration_list decl_list;
    struct compound_statement* comp;
};

void free_func_def_children(struct func_def* d);

#include "declaration_specs.h"
#include "declarator.h"

#include "frontend/ast/statement.h"

#endif

