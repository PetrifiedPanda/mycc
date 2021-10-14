#ifndef STATEMENT_LIST_H
#define STATEMENT_LIST_H

#include <stddef.h>

typedef struct Statement Statement;

typedef struct StatementList {
    size_t len;
    Statement* statements;
} StatementList;

StatementList create_statement_list(Statement* statements, size_t len);

void free_statement_list(StatementList* l);

#include "ast/statement.h"

#endif
