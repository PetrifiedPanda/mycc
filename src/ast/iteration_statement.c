#include "ast/iteration_statement.h"

#include <stdlib.h>
#include <assert.h>

static inline void assign_do_or_while(IterationStatement* res, Expr* while_cond, Statement* loop_body) {
    assert(res);
    assert(while_cond);
    assert(loop_body);
    res->while_cond = while_cond;
    res->loop_body = loop_body;
}

IterationStatement* create_while_loop(Expr* while_cond, Statement* loop_body) {
    IterationStatement* res = malloc(sizeof(IterationStatement));
    if (res) {
        res->type = WHILE;
        assign_do_or_while(res, while_cond, loop_body);
    }
    return res;
}

IterationStatement* create_do_loop(Expr* while_cond, Statement* loop_body) {
    IterationStatement* res = malloc(sizeof(IterationStatement));
    if (res) {
        res->type = DO;
        assign_do_or_while(res, while_cond, loop_body);
    }
    return res;
}

IterationStatement* create_for_loop(ExprStatement* init_expr, ExprStatement* for_cond, Expr* incr_expr, Statement* loop_body) {
    assert(init_expr);
    assert(for_cond);
    assert(loop_body);
    IterationStatement* res = malloc(sizeof(IterationStatement));
    if (res) {
        res->type = FOR;
        res->loop_body = loop_body;
        res->init_expr = init_expr;
        res->for_cond = for_cond;
        res->incr_expr = incr_expr;
    }
    return res;
}

static void free_children(IterationStatement* s) {
    switch (s->type) {
        case WHILE:
        case DO:
            free_expr(s->while_cond);
            break;
        case FOR:
            free_expr_statement(s->init_expr);
            free_expr_statement(s->for_cond);
            if (s->incr_expr) {
                free_expr(s->incr_expr);
            }
            break;
        default:
            assert(false);
    }
    free_statement(s->loop_body);
}

void free_iteration_statement(IterationStatement* s) {
    free_children(s);
    free(s);
}

