#ifndef COMPOUND_STATEMENT_H
#define COMPOUND_STATEMENT_H

#include "ast/statement_list.h"
#include "ast/declaration_list.h"

struct compound_statement {
    struct declaration_list decl_list;
    struct statement_list stat_list;
};

struct compound_statement* create_compound_statement(struct declaration_list decl_list, struct statement_list stat_list);

void free_compound_statement(struct compound_statement* s);

#endif

