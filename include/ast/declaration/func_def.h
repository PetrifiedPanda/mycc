#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include "ast/declaration/declaration_list.h"

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

struct ast_visitor;

bool visit_func_def(struct ast_visitor* visitor, struct func_def* d);

#include "ast/declaration/declaration_specs.h"
#include "ast/declaration/declarator.h"

#include "ast/statement/compound_statement.h"

#include "ast/ast_visitor.h"

#endif
