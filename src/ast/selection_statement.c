#include "ast/selection_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static SelectionStatement* create(Expr* sel_expr, Statement* sel_stat, Statement* else_stat) {
    assert(sel_expr);
    assert(sel_stat);
    SelectionStatement* res = xmalloc(sizeof(SelectionStatement));
    res->sel_expr = sel_expr;
    res->sel_stat = sel_stat;
    res->else_stat = else_stat;
    
    return res;
}

SelectionStatement* create_if_else_statement(Expr* sel_expr, Statement* sel_stat, Statement* else_stat) {
    assert(sel_expr);
    assert(sel_stat);
    SelectionStatement* res = create(sel_expr, sel_stat, else_stat);
    res->is_if = true;
    
    return res;
}

SelectionStatement* create_switch_statement(Expr* sel_expr, Statement* sel_stat) {
    assert(sel_expr);
    assert(sel_stat);
    SelectionStatement* res = create(sel_expr, sel_stat, NULL);
    res->is_if = false;
    
    return res;
}

static void free_children(SelectionStatement* s) {  
    free_expr(s->sel_expr);
    free_statement(s->sel_stat);
    if (s->else_stat) {
        free_statement(s->else_stat);
    }
}

void free_selection_statement(SelectionStatement* s) {
    free_children(s);
    free(s);
}

