#include "ast/statement_list.h"

#include <stdlib.h>
#include <assert.h>

StatementList create_statement_list(Statement* statements, size_t len) {
    if (len > 0) {
        assert(statements);
    } else {
        assert(statements == NULL);
    }

    StatementList res;

    res.len = len;
    res.statements = statements;
    return res;
}

void free_statement_list(StatementList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_statement_children(&l->statements[i]);
    }
    free(l->statements);
}

