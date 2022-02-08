#include "ast/statement_list.h"

#include <stdlib.h>
#include <assert.h>

struct statement_list create_statement_list(struct statement* statements,
                                            size_t len) {
    if (len > 0) {
        assert(statements);
    } else {
        assert(statements == NULL);
    }
    return (struct statement_list){.len = len, .statements = statements};
}

void free_statement_list(struct statement_list* l) {
    for (size_t i = 0; i < l->len; ++i) {
        free_statement_children(&l->statements[i]);
    }
    free(l->statements);
}
