#include "ast/labeled_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

struct labeled_statement* create_labeled_statement_goto(struct identifier* identifier, struct statement* stat) {
    assert(identifier);
    assert(stat);
    struct labeled_statement* res = xmalloc(sizeof(struct labeled_statement));
    res->type = IDENTIFIER;
    res->identifier = identifier;
    res->stat = stat;
    
    return res;
}

struct labeled_statement* create_labeled_statement_case(struct const_expr* case_expr, struct statement* stat) { 
    assert(case_expr);
    assert(stat);
    struct labeled_statement* res = xmalloc(sizeof(struct labeled_statement));
    res->type = CASE;
    res->case_expr = case_expr;
    res->stat = stat;
    
    return res;
}

struct labeled_statement* craete_labeled_statement_default(struct statement* stat) {
    assert(stat); 
    struct labeled_statement* res = xmalloc(sizeof(struct labeled_statement));
    res->type = DEFAULT;
    res->stat = stat;
    res->identifier = NULL;
    
    return res;
}

static void free_children(struct labeled_statement* s) {
    switch (s->type) {
        case IDENTIFIER:
            free_identifier(s->identifier);
            break;
        case CASE:
            free_const_expr(s->case_expr);
            break;
        default:
            assert(false);
    }
    free_statement(s->stat);
}

void free_labeled_statement(struct labeled_statement* s) {
    free_children(s);
    free(s);
}

