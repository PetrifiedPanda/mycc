#include "frontend/ast/expr/UnaryExpr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static inline void assign_operators_before(UnaryExpr* res,
                                           UnaryExprOp* ops_before,
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

static UnaryExpr* create_unary_expr_postfix(
    UnaryExprOp* ops_before,
    size_t len,
    PostfixExpr* postfix,
    SourceLoc loc) {
    assert(postfix);
    UnaryExpr* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = UNARY_POSTFIX;
    res->postfix = postfix;

    return res;
}

static bool is_unary_op(TokenKind k) {
    switch (k) {
        case TOKEN_AND:
        case TOKEN_ASTERISK:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_BNOT:
        case TOKEN_NOT:
            return true;
        default:
            return false;
    }
}

static UnaryExprKind token_type_to_unary_expr_type(TokenKind t) {
    assert(is_unary_op(t));
    switch (t) {
        case TOKEN_AND:
            return UNARY_ADDRESSOF;
        case TOKEN_ASTERISK:
            return UNARY_DEREF;
        case TOKEN_ADD:
            return UNARY_PLUS;
        case TOKEN_SUB:
            return UNARY_MINUS;
        case TOKEN_BNOT:
            return UNARY_BNOT;
        case TOKEN_NOT:
            return UNARY_NOT;
        default:
            UNREACHABLE();
    }
}

static UnaryExpr* create_unary_expr_unary_op(
    UnaryExprOp* ops_before,
    size_t len,
    TokenKind unary_op,
    CastExpr* cast_expr,
    SourceLoc loc) {
    assert(is_unary_op(unary_op));
    assert(cast_expr);
    UnaryExpr* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = token_type_to_unary_expr_type(unary_op);
    res->cast_expr = cast_expr;

    return res;
}

static UnaryExpr* create_unary_expr_sizeof_type(
    UnaryExprOp* ops_before,
    size_t len,
    TypeName* type_name,
    SourceLoc loc) {
    assert(type_name);
    UnaryExpr* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = UNARY_SIZEOF_TYPE;
    res->type_name = type_name;

    return res;
}

static UnaryExpr* create_unary_expr_alignof(
    UnaryExprOp* ops_before,
    size_t len,
    TypeName* type_name,
    SourceLoc loc) {
    assert(type_name);
    UnaryExpr* res = mycc_alloc(sizeof *res);
    res->info = create_ast_node_info(loc);
    assign_operators_before(res, ops_before, len);
    res->kind = UNARY_ALIGNOF;
    res->type_name = type_name;

    return res;
}

UnaryExpr* parse_unary_expr_type_name(
    ParserState* s,
    UnaryExprOp* ops_before,
    size_t len,
    TypeName* type_name,
    SourceLoc start_bracket_loc) {
    assert(type_name);

    PostfixExpr* postfix = parse_postfix_expr_type_name(
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

UnaryExprOp token_type_to_unary_expr_op(TokenKind t) {
    assert(t == TOKEN_INC || t == TOKEN_DEC || t == TOKEN_SIZEOF);
    switch (t) {
        case TOKEN_INC:
            return UNARY_OP_INC;
        case TOKEN_DEC:
            return UNARY_OP_DEC;
        case TOKEN_SIZEOF:
            return UNARY_OP_SIZEOF;

        default:
            UNREACHABLE();
    }
}

UnaryExpr* parse_unary_expr(ParserState* s) {
    size_t alloc_len = 0;
    UnaryExprOp* ops_before = NULL;

    const SourceLoc loc = s->it->loc;
    size_t len = 0;
    while (
        s->it->kind == TOKEN_INC || s->it->kind == TOKEN_DEC
        || (s->it->kind == TOKEN_SIZEOF && s->it[1].kind != TOKEN_LBRACKET)) {
        if (len == alloc_len) {
            mycc_grow_alloc((void**)&ops_before,
                            &alloc_len,
                            sizeof *ops_before);
        }

        ops_before[len] = token_type_to_unary_expr_op(s->it->kind);

        ++len;
        parser_accept_it(s);
    }

    if (ops_before) {
        ops_before = mycc_realloc(ops_before, len * sizeof *ops_before);
    }

    if (is_unary_op(s->it->kind)) {
        const TokenKind unary_op = s->it->kind;
        parser_accept_it(s);
        CastExpr* cast = parse_cast_expr(s);
        if (!cast) {
            goto fail;
        }
        return create_unary_expr_unary_op(ops_before, len, unary_op, cast, loc);
    } else {
        switch (s->it->kind) {
            case TOKEN_SIZEOF: {
                parser_accept_it(s);
                assert(s->it->kind == TOKEN_LBRACKET);
                const SourceLoc start_bracket_loc = s->it->loc;
                if (next_is_type_name(s)) {
                    parser_accept_it(s);

                    TypeName* type_name = parse_type_name(s);
                    if (!type_name) {
                        goto fail;
                    }

                    if (!parser_accept(s, TOKEN_RBRACKET)) {
                        goto fail;
                    }
                    if (s->it->kind == TOKEN_LBRACE) {
                        ++len;
                        ops_before = mycc_realloc(ops_before,
                                                  len * sizeof *ops_before);
                        ops_before[len - 1] = UNARY_OP_SIZEOF;

                        UnaryExpr* res = parse_unary_expr_type_name(
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
                    ops_before = mycc_realloc(ops_before,
                                              sizeof *ops_before * len);
                    ops_before[len - 1] = UNARY_OP_SIZEOF;

                    PostfixExpr* postfix = parse_postfix_expr(s);
                    if (!postfix) {
                        goto fail;
                    }
                    return create_unary_expr_postfix(ops_before,
                                                     len,
                                                     postfix,
                                                     loc);
                }
            }
            case TOKEN_ALIGNOF: {
                parser_accept_it(s);
                if (!parser_accept(s, TOKEN_LBRACKET)) {
                    goto fail;
                }

                TypeName* type_name = parse_type_name(s);
                if (!type_name) {
                    goto fail;
                }

                if (!parser_accept(s, TOKEN_RBRACKET)) {
                    goto fail;
                }
                return create_unary_expr_alignof(ops_before,
                                                 len,
                                                 type_name,
                                                 loc);
            }
            default: {
                PostfixExpr* postfix = parse_postfix_expr(s);
                if (!postfix) {
                    goto fail;
                }
                return create_unary_expr_postfix(ops_before, len, postfix, loc);
            }
        }
    }
fail:
    mycc_free(ops_before);
    return NULL;
}

void free_unary_expr_children(UnaryExpr* u) {
    mycc_free(u->ops_before);
    switch (u->kind) {
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

void free_unary_expr(UnaryExpr* u) {
    free_unary_expr_children(u);
    mycc_free(u);
}

