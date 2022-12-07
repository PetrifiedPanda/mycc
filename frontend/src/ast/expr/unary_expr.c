#include "frontend/ast/expr/unary_expr.h"

#include <stdlib.h>
#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static inline void assign_operators_before(struct unary_expr* res,
                                           enum unary_expr_op* ops_before,
                                           size_t len) {
    assert(res);
    if (len > 0) {
        assert(ops_before);
    } else {
        assert(ops_before == NULL);
    }
    res->len = len;
    res->ops_before = ops_before;
}

static struct unary_expr* create_unary_expr_postfix(
    enum unary_expr_op* ops_before,
    size_t len,
    struct postfix_expr* postfix,
    struct source_loc loc) {
    assert(postfix);
    struct unary_expr* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = UNARY_POSTFIX;
    res->postfix = postfix;

    return res;
}

static enum unary_expr_type token_type_to_unary_expr_type(enum token_type t) {
    assert(is_unary_op(t));
    switch (t) {
        case AND:
            return UNARY_ADDRESSOF;
        case ASTERISK:
            return UNARY_DEREF;
        case ADD:
            return UNARY_PLUS;
        case SUB:
            return UNARY_MINUS;
        case BNOT:
            return UNARY_BNOT;
        case NOT:
            return UNARY_NOT;
        default:
            UNREACHABLE();
    }
}

static struct unary_expr* create_unary_expr_unary_op(
    enum unary_expr_op* ops_before,
    size_t len,
    enum token_type unary_op,
    struct cast_expr* cast_expr,
    struct source_loc loc) {
    assert(is_unary_op(unary_op));
    assert(cast_expr);
    struct unary_expr* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = token_type_to_unary_expr_type(unary_op);
    res->cast_expr = cast_expr;

    return res;
}

static struct unary_expr* create_unary_expr_sizeof_type(
    enum unary_expr_op* ops_before,
    size_t len,
    struct type_name* type_name,
    struct source_loc loc) {
    assert(type_name);
    struct unary_expr* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = UNARY_SIZEOF_TYPE;
    res->type_name = type_name;

    return res;
}

static struct unary_expr* create_unary_expr_alignof(
    enum unary_expr_op* ops_before,
    size_t len,
    struct type_name* type_name,
    struct source_loc loc) {
    assert(type_name);
    struct unary_expr* res = xmalloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->type = UNARY_ALIGNOF;
    res->type_name = type_name;

    return res;
}

struct unary_expr* parse_unary_expr_type_name(
    struct parser_state* s,
    enum unary_expr_op* ops_before,
    size_t len,
    struct type_name* type_name,
    struct source_loc start_bracket_loc) {
    assert(type_name);

    struct postfix_expr* postfix = parse_postfix_expr_type_name(
        s,
        type_name,
        start_bracket_loc);
    if (!postfix) {
        return NULL;
    }

    return create_unary_expr_postfix(ops_before,
                                     len,
                                     postfix,
                                     start_bracket_loc);
}

enum unary_expr_op token_type_to_unary_expr_op(enum token_type t) {
    assert(t == INC_OP || t == DEC_OP || t == SIZEOF);
    switch (t) {
        case INC_OP:
            return UNARY_OP_INC;
        case DEC_OP:
            return UNARY_OP_DEC;
        case SIZEOF:
            return UNARY_OP_SIZEOF;

        default:
            UNREACHABLE();
    }
}

struct unary_expr* parse_unary_expr(struct parser_state* s) {
    size_t alloc_len = 0;
    enum unary_expr_op* ops_before = NULL;

    const struct source_loc loc = s->it->loc;
    size_t len = 0;
    while (s->it->type == INC_OP || s->it->type == DEC_OP
           || (s->it->type == SIZEOF && (s->it + 1)->type != LBRACKET)) {
        if (len == alloc_len) {
            grow_alloc((void**)&ops_before, &alloc_len, sizeof *ops_before);
        }

        ops_before[len] = token_type_to_unary_expr_op(s->it->type);

        ++len;
        accept_it(s);
    }

    if (ops_before) {
        ops_before = xrealloc(ops_before, len * sizeof *ops_before);
    }

    if (is_unary_op(s->it->type)) {
        const enum token_type unary_op = s->it->type;
        accept_it(s);
        struct cast_expr* cast = parse_cast_expr(s);
        if (!cast) {
            goto fail;
        }
        return create_unary_expr_unary_op(ops_before, len, unary_op, cast, loc);
    } else {
        switch (s->it->type) {
            case SIZEOF: {
                accept_it(s);
                assert(s->it->type == LBRACKET);
                struct source_loc start_bracket_loc = s->it->loc;
                if (next_is_type_name(s)) {
                    accept_it(s);

                    struct type_name* type_name = parse_type_name(s);
                    if (!type_name) {
                        goto fail;
                    }

                    if (!accept(s, RBRACKET)) {
                        goto fail;
                    }
                    if (s->it->type == LBRACE) {
                        ++len;
                        ops_before = xrealloc(ops_before,
                                              len * sizeof *ops_before);
                        ops_before[len - 1] = UNARY_OP_SIZEOF;

                        struct unary_expr* res = parse_unary_expr_type_name(
                            s,
                            ops_before,
                            len,
                            type_name,
                            start_bracket_loc);
                        if (!res) {
                            free_type_name(type_name);
                            goto fail;
                        }
                        return res;
                    } else {
                        return create_unary_expr_sizeof_type(ops_before,
                                                             len,
                                                             type_name,
                                                             loc);
                    }
                } else {
                    ++len;
                    ops_before = xrealloc(ops_before, sizeof *ops_before * len);
                    ops_before[len - 1] = UNARY_OP_SIZEOF;

                    struct postfix_expr* postfix = parse_postfix_expr(s);
                    if (!postfix) {
                        goto fail;
                    }
                    return create_unary_expr_postfix(ops_before,
                                                     len,
                                                     postfix,
                                                     loc);
                }
            }
            case ALIGNOF: {
                accept_it(s);
                if (!accept(s, LBRACKET)) {
                    goto fail;
                }

                struct type_name* type_name = parse_type_name(s);
                if (!type_name) {
                    goto fail;
                }

                if (!accept(s, RBRACKET)) {
                    goto fail;
                }
                return create_unary_expr_alignof(ops_before,
                                                 len,
                                                 type_name,
                                                 loc);
            }
            default: {
                struct postfix_expr* postfix = parse_postfix_expr(s);
                if (!postfix) {
                    goto fail;
                }
                return create_unary_expr_postfix(ops_before, len, postfix, loc);
            }
        }
    }
fail:
    free(ops_before);
    return NULL;
}

void free_unary_expr_children(struct unary_expr* u) {
    free(u->ops_before);
    switch (u->type) {
        case UNARY_POSTFIX:
            free_postfix_expr(u->postfix);
            break;
        case UNARY_ADDRESSOF:
        case UNARY_DEREF:
        case UNARY_PLUS:
        case UNARY_MINUS:
        case UNARY_BNOT:
        case UNARY_NOT:
            free_cast_expr(u->cast_expr);
            break;
        case UNARY_SIZEOF_TYPE:
        case UNARY_ALIGNOF:
            free_type_name(u->type_name);
            break;
    }
}

void free_unary_expr(struct unary_expr* u) {
    free_unary_expr_children(u);
    free(u);
}

