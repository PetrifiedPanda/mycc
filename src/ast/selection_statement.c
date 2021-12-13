#include "ast/selection_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static struct selection_statement* create(struct expr* sel_expr, struct statement* sel_stat, struct statement* else_stat) {
    assert(sel_expr);
    assert(sel_stat);
    struct selection_statement* res = xmalloc(sizeof(struct selection_statement));
    res->sel_expr = sel_expr;
    res->sel_stat = sel_stat;
    res->else_stat = else_stat;
    
    return res;
}

struct selection_statement* create_if_else_statement(struct expr* sel_expr, struct statement* sel_stat, struct statement* else_stat) {
    assert(sel_expr);
    assert(sel_stat);
    struct selection_statement* res = create(sel_expr, sel_stat, else_stat);
    res->is_if = true;
    
    return res;
}

struct selection_statement* create_switch_statement(struct expr* sel_expr, struct statement* sel_stat) {
    assert(sel_expr);
    assert(sel_stat);
    struct selection_statement* res = create(sel_expr, sel_stat, NULL);
    res->is_if = false;
    
    return res;
}

static void free_children(struct selection_statement* s) {  
    free_expr(s->sel_expr);
    free_statement(s->sel_stat);
    if (s->else_stat) {
        free_statement(s->else_stat);
    }
}

void free_selection_statement(struct selection_statement* s) {
    free_children(s);
    free(s);
}

