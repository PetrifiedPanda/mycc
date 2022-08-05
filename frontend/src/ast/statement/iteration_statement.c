#include "frontend/ast/statement/iteration_statement.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"
#include "util/annotations.h"

#include "frontend/parser/parser_util.h"

static inline void assign_do_or_while(struct iteration_statement* res,
                                      struct expr* while_cond,
                                      struct statement* loop_body) {
    assert(res);
    assert(while_cond);
    assert(loop_body);
    res->while_cond = while_cond;
    res->loop_body = loop_body;
}

static struct iteration_statement* create_while_loop(
    struct expr* while_cond,
    struct statement* loop_body) {
    struct iteration_statement* res = xmalloc(
        sizeof(struct iteration_statement));
    res->type = WHILE;
    assign_do_or_while(res, while_cond, loop_body);

    return res;
}

static struct iteration_statement* create_do_loop(struct expr* while_cond,
                                                  struct statement* loop_body) {
    struct iteration_statement* res = xmalloc(
        sizeof(struct iteration_statement));
    res->type = DO;
    assign_do_or_while(res, while_cond, loop_body);

    return res;
}

static struct iteration_statement* create_for_loop(
    struct for_loop for_loop,
    struct statement* loop_body) {
    if (for_loop.is_decl) {
        assert(for_loop.init_decl);
    } else {
        assert(for_loop.init_expr);
    }
    assert(for_loop.cond);
    assert(loop_body);
    struct iteration_statement* res = xmalloc(
        sizeof(struct iteration_statement));
    res->type = FOR;
    res->loop_body = loop_body;
    res->for_loop = for_loop;

    return res;
}

static struct iteration_statement* parse_while_statement(
    struct parser_state* s) {
    assert(s->it->type == WHILE);

    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct expr* while_cond = parse_expr(s);
    if (!while_cond) {
        return NULL;
    }

    if (!accept(s, RBRACKET)) {
        free_expr(while_cond);
        return NULL;
    }

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        free_expr(while_cond);
        return NULL;
    }

    return create_while_loop(while_cond, loop_body);
}

static struct iteration_statement* parse_do_loop(struct parser_state* s) {
    assert(s->it->type == DO);

    accept_it(s);

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        return NULL;
    }

    if (!accept(s, WHILE) || !accept(s, LBRACKET)) {
        free_statement(loop_body);
        return NULL;
    }

    struct expr* while_cond = parse_expr(s);
    if (!(while_cond && accept(s, RBRACKET) && accept(s, SEMICOLON))) {
        free_statement(loop_body);
        return NULL;
    }

    return create_do_loop(while_cond, loop_body);
}

static struct iteration_statement* parse_for_loop(struct parser_state* s) {
    assert(s->it->type == FOR);

    accept_it(s);

    if (!accept(s, LBRACKET)) {
        return NULL;
    }

    struct for_loop loop;
    if (is_declaration(s)) {
        loop.is_decl = true;
        loop.init_decl = parse_declaration(s);
        if (!loop.init_decl) {
            return NULL;
        }
    } else {
        loop.is_decl = false;
        loop.init_expr = parse_expr_statement(s);
        if (!loop.init_expr) {
            return false;
        }
    }

    loop.cond = parse_expr_statement(s);
    if (!loop.cond) {
        if (loop.is_decl) {
            free_declaration(loop.init_decl);
        } else {
            free_expr_statement(loop.init_expr);
        }
        return NULL;
    }

    if (s->it->type != RBRACKET) {
        loop.incr_expr = parse_expr(s);
        if (!loop.incr_expr) {
            goto fail;
        }
    } else {
        loop.incr_expr = NULL;
    }

    if (!accept(s, RBRACKET)) {
        goto fail;
    }

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        goto fail;
    }

    return create_for_loop(loop, loop_body);
fail:
    if (loop.is_decl) {
        free_declaration(loop.init_decl);
    } else {
        free_expr_statement(loop.init_expr);
    }
    free_expr_statement(loop.cond);
    return NULL;
}

struct iteration_statement* parse_iteration_statement(struct parser_state* s) {
    switch (s->it->type) {
        case WHILE:
            return parse_while_statement(s);
        case DO:
            return parse_do_loop(s);

        case FOR: {
            return parse_for_loop(s);
        }

        default: {
            enum token_type expected[] = {WHILE, DO, FOR};

            expected_tokens_error(s,
                                  expected,
                                  sizeof expected / sizeof(enum token_type));
            return NULL;
        }
    }
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
            UNREACHABLE();
    }
    free_statement(s->loop_body);
}

void free_iteration_statement(struct iteration_statement* s) {
    free_children(s);
    free(s);
}