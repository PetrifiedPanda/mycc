#include "ast/compound_statement.h"

#include <stdlib.h>
#include <assert.h>

CompoundStatement* create_compound_statement(DeclarationList* decl_list, StatementList* stat_list) {
    assert(decl_list || stat_list);
    CompoundStatement* res = malloc(sizeof(CompoundStatement));
    if (res) {
        res->decl_list = decl_list;
        res->stat_list = stat_list;
    }
    return res;
}

void free_children(CompoundStatement* s) {
    if (s->decl_list) {
        free_declaration_list(s->decl_list);
    }
    if (s->stat_list) {
        free_statement_list(s->stat_list);
    }
}

void free_compound_statement(CompoundStatement* s) {
    free_children(s);
    free(s);
}
