#include "ast/compound_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct compound_statement* create_compound_statement(struct declaration_list decl_list, struct statement_list stat_list) {
    struct compound_statement* res = xmalloc(sizeof(struct compound_statement));
    res->decl_list = decl_list;
    res->stat_list = stat_list;

    return res;
}

void free_children(struct compound_statement* s) {
    free_declaration_list(&s->decl_list);
    free_statement_list(&s->stat_list);
}

void free_compound_statement(struct compound_statement* s) {
    free_children(s);
    free(s);
}

