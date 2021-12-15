#include "ast/iteration_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util.h"

static inline void assign_do_or_while(struct iteration_statement* res, struct expr* while_cond, struct statement* loop_body) {
    assert(res);
    assert(while_cond);
    assert(loop_body);
    res->while_cond = while_cond;
    res->loop_body = loop_body;
}

struct iteration_statement* create_while_loop(struct expr* while_cond, struct statement* loop_body) {
    struct iteration_statement* res = xmalloc(sizeof(struct iteration_statement));
    res->type = WHILE;
    assign_do_or_while(res, while_cond, loop_body);
    
    return res;
}

struct iteration_statement* create_do_loop(struct expr* while_cond, struct statement* loop_body) {
    struct iteration_statement* res = xmalloc(sizeof(struct iteration_statement));
    res->type = DO;
    assign_do_or_while(res, while_cond, loop_body);
    
    return res;
}

struct iteration_statement* create_for_loop(struct expr_statement* init_expr, struct expr_statement* for_cond, struct expr* incr_expr, struct statement* loop_body) {
    assert(init_expr);
    assert(for_cond);
    assert(loop_body);
    struct iteration_statement* res = xmalloc(sizeof(struct iteration_statement));
    res->type = FOR;
    res->loop_body = loop_body;
    res->for_loop.is_decl = false;
    res->for_loop.init_expr = init_expr;
    res->for_loop.cond = for_cond;
    res->for_loop.incr_expr = incr_expr;
    
    return res;
}

struct iteration_statement* create_for_loop_decl(struct declaration* decl, struct expr_statement* for_cond, struct expr* incr_expr, struct statement* loop_body) {
    assert(decl);
    assert(for_cond);
    assert(loop_body);
    struct iteration_statement* res = xmalloc(sizeof(struct iteration_statement));
    res->type = FOR;
    res->loop_body = loop_body;
    res->for_loop.is_decl = true;
    res->for_loop.init_decl = decl;
    res->for_loop.cond = for_cond;
    res->for_loop.incr_expr = incr_expr;

    return res;
}

static void free_children(struct iteration_statement* s) {
    switch (s->type) {
        case WHILE:
        case DO:
            free_expr(s->while_cond);
            break;
        case FOR: {
            if (s->for_loop.is_decl) {
                free_declaration(s->for_loop.init_decl);
            } else {
                free_expr_statement(s->for_loop.init_expr);
            }
            free_expr_statement(s->for_loop.cond);
            if (s->for_loop.incr_expr) {
                free_expr(s->for_loop.incr_expr);
            }
            break;
        }
        default:
            assert(false);
    }
    free_statement(s->loop_body);
}

void free_iteration_statement(struct iteration_statement* s) {
    free_children(s);
    free(s);
}

