#ifndef STATEMENT_LIST_H
#define STATEMENT_LIST_H

#include <stddef.h>

struct statement;

struct statement_list {
    size_t len;
    struct statement* statements;
};

struct statement_list create_statement_list(struct statement* statements,
                                            size_t len);

void free_statement_list(struct statement_list* l);

#include "ast/statement.h"

#endif
