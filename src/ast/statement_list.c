#include "ast/statement_list.h"

#include <stdlib.h>
#include <assert.h>

StatementList* create_statement_list(Statement* statements, size_t len) {
    assert(len > 0);
    assert(statements);
    StatementList* res = malloc(sizeof(StatementList));
    if (res) {
        res->len = len;
        res->statements = statements;
    }
    return res;
}

static void free_children(StatementList* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_statement_children(&l->statements[i]);
    }
    free(l->statements);
}

void free_statement_list(StatementList* l) {
    free_children(l);
    free(l);
}
