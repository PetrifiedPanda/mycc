#include "frontend/ast/expr/AssignExpr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

typedef struct {
    bool is_cond;
    union {
        UnaryExpr* unary;
        CondExpr* cond;
    };
} UnaryOrCond;

static UnaryOrCond parse_unary_or_cond(ParserState* s) {
    UnaryOrCond res = {
        .is_cond = false,
        .cond = NULL,
    };
    if (s->it->kind == TOKEN_LBRACKET && next_is_type_name(s)) {
        const SourceLoc start_bracket_loc = s->it->loc;
        parser_accept_it(s);

        TypeName* type_name = parse_type_name(s);
        if (!type_name) {
            return res;
        }

        if (!parser_accept(s, TOKEN_RBRACKET)) {
            free_type_name(type_name);
            return res;
        }

        if (s->it->kind == TOKEN_LBRACE) {
            res.is_cond = false;
            res.unary = parse_unary_expr_type_name(s, NULL, 0, type_name, start_bracket_loc);
        } else {
            CastExpr* cast_expr = parse_cast_expr_type_name(s, type_name, start_bracket_loc);
            if (!cast_expr) {
                return res;
            }

            res.is_cond = true;
            res.cond = parse_cond_expr_cast(s, cast_expr);
        }
    } else {
        res.is_cond = false;
        res.unary = parse_unary_expr(s);
    }

    return res;
}

static bool is_assign_op(TokenKind k) {
    switch (k) {
        case TOKEN_ASSIGN:
        case TOKEN_MUL_ASSIGN:
        case TOKEN_DIV_ASSIGN:
        case TOKEN_MOD_ASSIGN:
        case TOKEN_ADD_ASSIGN:
        case TOKEN_SUB_ASSIGN:
        case TOKEN_LSHIFT_ASSIGN:
        case TOKEN_RSHIFT_ASSIGN:
        case TOKEN_AND_ASSIGN:
        case TOKEN_XOR_ASSIGN:
        case TOKEN_OR_ASSIGN:
            return true;
        default:
            return false;
    }
}

static AssignExprOp token_type_to_assign_op(TokenKind t) {
    assert(is_assign_op(t));
    switch (t) {
        case TOKEN_ASSIGN:
            return ASSIGN_EXPR_ASSIGN;
        case TOKEN_MUL_ASSIGN:
            return ASSIGN_EXPR_MUL;
        case TOKEN_DIV_ASSIGN:
            return ASSIGN_EXPR_DIV;
        case TOKEN_MOD_ASSIGN:
            return ASSIGN_EXPR_MOD;
        case TOKEN_ADD_ASSIGN:
            return ASSIGN_EXPR_ADD;
        case TOKEN_SUB_ASSIGN:
            return ASSIGN_EXPR_SUB;
        case TOKEN_LSHIFT_ASSIGN:
            return ASSIGN_EXPR_LSHIFT;
        case TOKEN_RSHIFT_ASSIGN:
            return ASSIGN_EXPR_RSHIFT;
        case TOKEN_AND_ASSIGN:
            return ASSIGN_EXPR_AND;
        case TOKEN_XOR_ASSIGN:
            return ASSIGN_EXPR_XOR;
        case TOKEN_OR_ASSIGN:
            return ASSIGN_EXPR_OR;

        default:
            UNREACHABLE();
    }
}

bool parse_assign_expr_inplace(ParserState* s, AssignExpr* res) {
    assert(res);

    res->len = 0;
    res->assign_chain = NULL;
    res->value = NULL;

    UnaryOrCond opt = parse_unary_or_cond(s);
    if (opt.unary == NULL) {
        return false;
    } else if (opt.is_cond) {
        res->value = opt.cond;
        return true;
    }

    UnaryExpr* last_unary = opt.unary;

    size_t alloc_len = res->len;
    while (is_assign_op(s->it->kind)) {
        TokenKind op = s->it->kind;
        parser_accept_it(s);

        opt = parse_unary_or_cond(s);
        if (opt.unary == NULL) {
            free_unary_expr(last_unary);
            goto fail;
        } else if (opt.is_cond) {
            res->value = opt.cond;
            ++res->len;
            res->assign_chain = mycc_realloc(res->assign_chain, sizeof *res->assign_chain * res->len);
            res->assign_chain[res->len - 1] = (UnaryAndOp){
                .op = token_type_to_assign_op(op),
                .unary = last_unary,
            };
            return true;
        }

        UnaryExpr* new_last = opt.unary;

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->assign_chain, &alloc_len, sizeof *res->assign_chain);
        }

        res->assign_chain[res->len] = (UnaryAndOp){
            .op = token_type_to_assign_op(op),
            .unary = last_unary,
        };
        last_unary = new_last;

        ++res->len;
    }

    res->assign_chain = mycc_realloc(res->assign_chain, sizeof *res->assign_chain * res->len);

    res->value = parse_cond_expr_cast(s, create_cast_expr_unary(last_unary));
    if (!res->value) {
        goto fail;
    }

    return true;
fail:
    for (size_t i = 0; i < res->len; ++i) {
        free_unary_expr(res->assign_chain[i].unary);
    }
    mycc_free(res->assign_chain);

    return false;
}

struct AssignExpr* parse_assign_expr(ParserState* s) {
    struct AssignExpr* res = mycc_alloc(sizeof *res);
    if (!parse_assign_expr_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

void free_assign_expr_children(struct AssignExpr* e) {
    for (size_t i = 0; i < e->len; ++i) {
        free_unary_expr(e->assign_chain[i].unary);
    }
    mycc_free(e->assign_chain);

    free_cond_expr(e->value);
}

void free_assign_expr(struct AssignExpr* e) {
    free_assign_expr_children(e);
    mycc_free(e);
}
