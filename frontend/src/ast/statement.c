#include "frontend/ast/statement.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/token_kind.h"

#include "frontend/parser/parser_util.h"

static struct labeled_statement* parse_labeled_statement(
    struct parser_state* s) {
    assert(s->it->kind == TOKEN_CASE || s->it->kind == TOKEN_IDENTIFIER || s->it->kind == TOKEN_DEFAULT);
    struct labeled_statement* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    switch (s->it->kind) {
        case TOKEN_CASE: {
            res->kind = LABELED_STATEMENT_CASE;
            parser_accept_it(s);
            struct const_expr* case_expr = parse_const_expr(s);
            if (!case_expr) {
                mycc_free(res);
                return NULL;
            }
            res->case_expr = case_expr;
            break;
        }

        case TOKEN_IDENTIFIER: {
            res->kind = LABELED_STATEMENT_LABEL;
            const struct str spelling = token_take_spelling(s->it);
            struct source_loc loc = s->it->loc;
            parser_accept_it(s);
            res->label = create_identifier(&spelling, loc);
            break;
        }

        case TOKEN_DEFAULT: {
            res->kind = LABELED_STATEMENT_DEFAULT;
            parser_accept_it(s);
            res->case_expr = NULL;
            break;
        }
        default:
            UNREACHABLE();
    }

    if (!parser_accept(s, TOKEN_COLON)) {
        goto fail;
    }

    res->stat = parse_statement(s);
    if (!res->stat) {
        goto fail;
    }

    return res;
fail:
    if (res->kind == LABELED_STATEMENT_CASE) {
        free_const_expr(res->case_expr);
    }
    mycc_free(res);
    return NULL;
}

static void free_labeled_statement_children(struct labeled_statement* s) {
    switch (s->kind) {
        case LABELED_STATEMENT_LABEL:
            free_identifier(s->label);
            break;
        case LABELED_STATEMENT_CASE:
            free_const_expr(s->case_expr);
            break;
        case LABELED_STATEMENT_DEFAULT:
            break;
        default:
            UNREACHABLE();
    }
    free_statement(s->stat);
}

void free_labeled_statement(struct labeled_statement* s) {
    free_labeled_statement_children(s);
    mycc_free(s);
}

static bool parse_block_item_inplace(struct parser_state* s,
                                     struct block_item* res) {
    if (is_declaration(s)) {
        res->is_decl = true;
        if (!parse_declaration_inplace(s, &res->decl)) {
            return false;
        }
    } else {
        res->is_decl = false;
        if (!parse_statement_inplace(s, &res->stat)) {
            return false;
        }
    }

    return true;
}

static void free_block_item_children(struct block_item* i) {
    if (i->is_decl) {
        free_declaration_children(&i->decl);
    } else {
        free_statement_children(&i->stat);
    }
}

void free_compound_statement_children(struct compound_statement* s) {
    for (size_t i = 0; i < s->len; ++i) {
        free_block_item_children(&s->items[i]);
    }
    mycc_free(s->items);
}

void free_compound_statement(struct compound_statement* s) {
    free_compound_statement_children(s);
    mycc_free(s);
}

bool parse_compound_statement_inplace(struct parser_state* s,
                                      struct compound_statement* res) {
    res->info = create_ast_node_info(s->it->loc);
    if (!parser_accept(s, TOKEN_LBRACE)) {
        return false;
    }

    parser_push_scope(s);
    res->items = NULL;
    res->len = 0;

    size_t alloc_len = res->len;
    while (s->it->kind != TOKEN_RBRACE) {
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->items,
                            &alloc_len,
                            sizeof *res->items);
        }

        if (!parse_block_item_inplace(s, &res->items[res->len])) {
            free_compound_statement_children(res);
            return false;
        }

        ++res->len;
    }

    res->items = mycc_realloc(res->items, sizeof *res->items * res->len);

    assert(s->it->kind == TOKEN_RBRACE);
    parser_accept_it(s);

    parser_pop_scope(s);

    return true;
}

static struct compound_statement* parse_compound_statement(
    struct parser_state* s) {
    struct compound_statement* res = mycc_alloc(sizeof *res);
    if (!parse_compound_statement_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void free_expr_statement(struct expr_statement* s);

static struct expr_statement* parse_expr_statement(struct parser_state* s) {
    struct expr_statement* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->kind == TOKEN_SEMICOLON) {
        parser_accept_it(s);
        res->expr.len = 0;
        res->expr.assign_exprs = NULL;
        return res;
    } else {
        if (!parse_expr_inplace(s, &res->expr)) {
            mycc_free(res);
            return NULL;
        }

        if (!parser_accept(s, TOKEN_SEMICOLON)) {
            free_expr_statement(res);
            return NULL;
        }

        return res;
    }
}

static void free_expr_statement_children(struct expr_statement* s) {
    free_expr_children(&s->expr);
}

void free_expr_statement(struct expr_statement* s) {
    free_expr_statement_children(s);
    mycc_free(s);
}

static struct selection_statement* parse_selection_statement(
    struct parser_state* s) {
    assert(s->it->kind == TOKEN_IF || s->it->kind == TOKEN_SWITCH);
    struct selection_statement* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(s->it->loc);
    if (s->it->kind == TOKEN_IF) {
        res->is_if = true;
    } else {
        res->is_if = false;
    }
    parser_accept_it(s);

    if (!parser_accept(s, TOKEN_LBRACKET)) {
        mycc_free(res);
        return NULL;
    }

    res->sel_expr = parse_expr(s);
    if (!res->sel_expr) {
        mycc_free(res);
        return NULL;
    }

    if (!parser_accept(s, TOKEN_RBRACKET)) {
        free_expr(res->sel_expr);
        mycc_free(res);
        return NULL;
    }

    res->sel_stat = parse_statement(s);
    if (!res->sel_stat) {
        free_expr(res->sel_expr);
        mycc_free(res);
        return NULL;
    }

    if (res->is_if && s->it->kind == TOKEN_ELSE) {
        parser_accept_it(s);
        res->else_stat = parse_statement(s);
        if (!res->else_stat) {
            free_statement(res->sel_stat);
            free_expr(res->sel_expr);
            mycc_free(res);
            return NULL;
        }
    } else {
        res->else_stat = NULL;
    }

    return res;
}

static void free_selection_statement_children(struct selection_statement* s) {
    free_expr(s->sel_expr);
    free_statement(s->sel_stat);
    if (s->else_stat) {
        free_statement(s->else_stat);
    }
}

void free_selection_statement(struct selection_statement* s) {
    free_selection_statement_children(s);
    mycc_free(s);
}

static void assign_do_or_while(struct source_loc loc,
                               struct iteration_statement* res,
                               struct expr* while_cond,
                               struct statement* loop_body) {
    assert(res);
    assert(while_cond);
    assert(loop_body);
    res->info = create_ast_node_info(loc);
    res->while_cond = while_cond;
    res->loop_body = loop_body;
}

static struct iteration_statement* create_while_loop(
    struct source_loc loc,
    struct expr* while_cond,
    struct statement* loop_body) {
    struct iteration_statement* res = mycc_alloc(sizeof *res);
    res->kind = ITERATION_STATEMENT_WHILE;
    assign_do_or_while(loc, res, while_cond, loop_body);

    return res;
}

static struct iteration_statement* create_do_loop(struct source_loc loc,
                                                  struct expr* while_cond,
                                                  struct statement* loop_body) {
    struct iteration_statement* res = mycc_alloc(sizeof *res);
    res->kind = ITERATION_STATEMENT_DO;
    assign_do_or_while(loc, res, while_cond, loop_body);

    return res;
}

static struct iteration_statement* create_for_loop(
    struct source_loc loc,
    struct for_loop for_loop,
    struct statement* loop_body) {
    assert(for_loop.cond);
    assert(loop_body);
    struct iteration_statement* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->kind = ITERATION_STATEMENT_FOR;
    res->loop_body = loop_body;
    res->for_loop = for_loop;

    return res;
}

static struct iteration_statement* parse_while_statement(
    struct parser_state* s,
    struct source_loc loc) {
    assert(s->it->kind == TOKEN_WHILE);

    parser_accept_it(s);

    if (!parser_accept(s, TOKEN_LBRACKET)) {
        return NULL;
    }

    struct expr* while_cond = parse_expr(s);
    if (!while_cond) {
        return NULL;
    }

    if (!parser_accept(s, TOKEN_RBRACKET)) {
        free_expr(while_cond);
        return NULL;
    }

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        free_expr(while_cond);
        return NULL;
    }

    return create_while_loop(loc, while_cond, loop_body);
}

static struct iteration_statement* parse_do_loop(struct parser_state* s,
                                                 struct source_loc loc) {
    assert(s->it->kind == TOKEN_DO);

    parser_accept_it(s);

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        return NULL;
    }

    if (!parser_accept(s, TOKEN_WHILE) || !parser_accept(s, TOKEN_LBRACKET)) {
        free_statement(loop_body);
        return NULL;
    }

    struct expr* while_cond = parse_expr(s);
    if (!(while_cond && parser_accept(s, TOKEN_RBRACKET) && parser_accept(s, TOKEN_SEMICOLON))) {
        free_statement(loop_body);
        return NULL;
    }

    return create_do_loop(loc, while_cond, loop_body);
}

static struct iteration_statement* parse_for_loop(struct parser_state* s,
                                                  struct source_loc loc) {
    assert(s->it->kind == TOKEN_FOR);

    parser_accept_it(s);

    if (!parser_accept(s, TOKEN_LBRACKET)) {
        return NULL;
    }

    struct for_loop loop;
    if (is_declaration(s)) {
        loop.is_decl = true;
        if (!parse_declaration_inplace(s, &loop.init_decl)) {
            return NULL;
        }
    } else {
        loop.is_decl = false;
        loop.init_expr = parse_expr_statement(s);
        if (!loop.init_expr) {
            return NULL;
        }
    }

    loop.cond = parse_expr_statement(s);
    if (!loop.cond) {
        if (loop.is_decl) {
            free_declaration_children(&loop.init_decl);
        } else {
            free_expr_statement(loop.init_expr);
        }
        return NULL;
    }

    if (s->it->kind != TOKEN_RBRACKET) {
        loop.incr_expr = parse_expr(s);
        if (!loop.incr_expr) {
            goto fail;
        }
    } else {
        loop.incr_expr = NULL;
    }

    if (!parser_accept(s, TOKEN_RBRACKET)) {
        goto fail;
    }

    struct statement* loop_body = parse_statement(s);
    if (!loop_body) {
        goto fail;
    }

    return create_for_loop(loc, loop, loop_body);
fail:
    if (loop.is_decl) {
        free_declaration_children(&loop.init_decl);
    } else {
        free_expr_statement(loop.init_expr);
    }
    free_expr_statement(loop.cond);
    return NULL;
}

static struct iteration_statement* parse_iteration_statement(
    struct parser_state* s) {
    assert(s->it->kind == TOKEN_WHILE || s->it->kind == TOKEN_DO || s->it->kind == TOKEN_FOR);
    const struct source_loc loc = s->it->loc;
    switch (s->it->kind) {
        case TOKEN_WHILE:
            return parse_while_statement(s, loc);
        case TOKEN_DO:
            return parse_do_loop(s, loc);
        case TOKEN_FOR:
            return parse_for_loop(s, loc);

        default:
            UNREACHABLE();
    }
}

static void free_iteration_statement_children(struct iteration_statement* s) {
    switch (s->kind) {
        case ITERATION_STATEMENT_WHILE:
        case ITERATION_STATEMENT_DO:
            free_expr(s->while_cond);
            break;
        case ITERATION_STATEMENT_FOR: {
            if (s->for_loop.is_decl) {
                free_declaration_children(&s->for_loop.init_decl);
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
    free_iteration_statement_children(s);
    mycc_free(s);
}

static struct jump_statement* create_jump_statement(
    struct source_loc loc,
    enum jump_statement_kind kind) {
    struct jump_statement* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    res->kind = kind;

    return res;
}

static struct jump_statement* create_goto_statement(
    struct source_loc loc,
    struct identifier* identifier) {
    assert(identifier);
    struct jump_statement* res = create_jump_statement(loc,
                                                       JUMP_STATEMENT_GOTO);
    res->goto_label = identifier;

    return res;
}

static struct jump_statement* create_return_statement(struct source_loc loc,
                                                      struct expr* ret_val) {
    struct jump_statement* res = create_jump_statement(loc,
                                                       JUMP_STATEMENT_RETURN);
    res->ret_val = ret_val;
    return res;
}

void free_jump_statement(struct jump_statement* s);

static struct jump_statement* parse_jump_statement(struct parser_state* s) {
    assert(s->it->kind == TOKEN_GOTO || s->it->kind == TOKEN_CONTINUE || s->it->kind == TOKEN_BREAK || s->it->kind == TOKEN_RETURN);
    const struct source_loc loc = s->it->loc;
    struct jump_statement* res = NULL;
    switch (s->it->kind) {
        case TOKEN_GOTO: {
            parser_accept_it(s);
            if (s->it->kind == TOKEN_IDENTIFIER) {
                const struct str spell = token_take_spelling(s->it);
                const struct source_loc id_loc = s->it->loc;
                parser_accept_it(s);
                res = create_goto_statement(loc,
                                            create_identifier(&spell, id_loc));
                break;
            } else {
                expected_token_error(s, TOKEN_IDENTIFIER);
                return NULL;
            }
        }
        case TOKEN_CONTINUE:
        case TOKEN_BREAK: {
            const enum token_kind t = s->it->kind;
            parser_accept_it(s);
            res = create_jump_statement(loc,
                                        t == TOKEN_CONTINUE ? JUMP_STATEMENT_CONTINUE
                                                      : JUMP_STATEMENT_BREAK);
            res->ret_val = NULL;
            break;
        }
        case TOKEN_RETURN: {
            parser_accept_it(s);
            struct expr* ret_val;
            if (s->it->kind == TOKEN_SEMICOLON) {
                ret_val = NULL;
            } else {
                ret_val = parse_expr(s);
                if (!ret_val) {
                    return NULL;
                }
            }

            res = create_return_statement(loc, ret_val);
            break;
        }

        default:
            UNREACHABLE();
    }

    if (!parser_accept(s, TOKEN_SEMICOLON)) {
        free_jump_statement(res);
        return NULL;
    }

    return res;
}

static void free_jump_statement_children(struct jump_statement* s) {
    switch (s->kind) {
        case JUMP_STATEMENT_GOTO:
            free_identifier(s->goto_label);
            break;
        case JUMP_STATEMENT_CONTINUE:
        case JUMP_STATEMENT_BREAK:
            break;
        case JUMP_STATEMENT_RETURN:
            if (s->ret_val) {
                free_expr(s->ret_val);
            }
            break;
    }
}

void free_jump_statement(struct jump_statement* s) {
    free_jump_statement_children(s);
    mycc_free(s);
}

bool parse_statement_inplace(struct parser_state* s, struct statement* res) {
    assert(res);

    switch (s->it->kind) {
        case TOKEN_LBRACE: {
            res->kind = STATEMENT_COMPOUND;
            res->comp = parse_compound_statement(s);
            if (!res->comp) {
                return false;
            }
            break;
        }

        case TOKEN_FOR:
        case TOKEN_WHILE:
        case TOKEN_DO: {
            res->kind = STATEMENT_ITERATION;
            res->it = parse_iteration_statement(s);
            if (!res->it) {
                return false;
            }
            break;
        }

        case TOKEN_GOTO:
        case TOKEN_CONTINUE:
        case TOKEN_BREAK:
        case TOKEN_RETURN: {
            res->kind = STATEMENT_JUMP;
            res->jmp = parse_jump_statement(s);
            if (!res->jmp) {
                return false;
            }
            break;
        }

        case TOKEN_IF:
        case TOKEN_SWITCH: {
            res->kind = STATEMENT_SELECTION;
            res->sel = parse_selection_statement(s);
            if (!res->sel) {
                return false;
            }
            break;
        }

        case TOKEN_CASE:
        case TOKEN_DEFAULT: {
            res->kind = STATEMENT_LABELED;
            res->labeled = parse_labeled_statement(s);
            if (!res->labeled) {
                return false;
            }
            break;
        }

        case TOKEN_IDENTIFIER: {
            if (s->it[1].kind == TOKEN_COLON) {
                res->kind = STATEMENT_LABELED;
                res->labeled = parse_labeled_statement(s);
                if (!res->labeled) {
                    return false;
                }
                break;
            }
            goto is_stat_expr; // to avoid fallthrough warning
        }

is_stat_expr:
        default: {
            res->kind = STATEMENT_EXPRESSION;
            res->expr = parse_expr_statement(s);
            if (!res->expr) {
                return false;
            }
            break;
        }
    }
    return true;
}

struct statement* parse_statement(struct parser_state* s) {
    struct statement* res = mycc_alloc(sizeof *res);
    if (!parse_statement_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void free_statement_children(struct statement* s) {
    switch (s->kind) {
        case STATEMENT_LABELED:
            free_labeled_statement(s->labeled);
            break;
        case STATEMENT_COMPOUND:
            free_compound_statement(s->comp);
            break;
        case STATEMENT_EXPRESSION:
            free_expr_statement(s->expr);
            break;
        case STATEMENT_SELECTION:
            free_selection_statement(s->sel);
            break;
        case STATEMENT_ITERATION:
            free_iteration_statement(s->it);
            break;
        case STATEMENT_JUMP:
            free_jump_statement(s->jmp);
            break;
    }
}

void free_statement(struct statement* s) {
    free_statement_children(s);
    mycc_free(s);
}
