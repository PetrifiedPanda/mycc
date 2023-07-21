#include "frontend/ast/AssignExpr.h"

#include <assert.h>

#include "util/mem.h"
#include "util/macro_util.h"

#include "frontend/parser/parser_util.h"

static bool parse_cast_expr_rest(ParserState* s, CastExpr* res) {
    uint32_t alloc_len = res->len;
    SourceLoc last_lbracket_loc = {
        .file_idx = (uint32_t)-1,
        .file_loc = {0, 0},
    };
    while (ParserState_curr_kind(s) == TOKEN_LBRACKET
           && next_is_type_name(s)) {
        last_lbracket_loc = ParserState_curr_loc(s);
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->type_names,
                            &alloc_len,
                            sizeof *res->type_names);
        }

        if (!parse_type_name_inplace(s, &res->type_names[res->len])) {
            goto fail;
        }

        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            goto fail;
        }
        ++res->len;
    }

    if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
        assert(res->len > 0);
        TypeName* type_name = mycc_alloc(sizeof *type_name);

        --res->len;
        *type_name = res->type_names[res->len];

        if (res->type_names) {
            res->type_names = mycc_realloc(res->type_names,
                                           sizeof *res->type_names * res->len);
        }

        if (!parse_unary_expr_type_name(s,
                                        &res->rhs,
                                        NULL,
                                        0,
                                        type_name,
                                        last_lbracket_loc)) {
            goto fail;
        }
    } else {
        if (res->type_names) {
            res->type_names = mycc_realloc(res->type_names,
                                           sizeof *res->type_names * res->len);
        }
        if (!parse_unary_expr_inplace(s, &res->rhs)) {
            goto fail;
        }
    }

    return true;

fail:
    for (uint32_t i = 0; i < res->len; ++i) {
        TypeName_free_children(&res->type_names[i]);
    }
    mycc_free(res->type_names);
    return false;
}

static bool parse_cast_expr_inplace(ParserState* s, CastExpr* res) {
    res->info = AstNodeInfo_create(ParserState_curr_loc(s));
    res->type_names = NULL;
    res->len = 0;

    if (!parse_cast_expr_rest(s, res)) {
        return false;
    }
    return true;
}

CastExpr* parse_cast_expr(ParserState* s) {
    CastExpr* res = mycc_alloc(sizeof *res);
    if (!parse_cast_expr_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

static bool parse_cast_expr_type_name(ParserState* s,
                                      CastExpr* res,
                                      TypeName* type_name,
                                      SourceLoc start_bracket_loc) {
    assert(type_name);

    res->info = AstNodeInfo_create(start_bracket_loc);
    res->type_names = type_name;
    res->len = 1;

    if (!parse_cast_expr_rest(s, res)) {
        return false;
    }

    return true;
}

static CastExpr CastExpr_create_unary(const UnaryExpr* start) {
    assert(start);
    return (CastExpr){
        .info = AstNodeInfo_create(start->info.loc),
        .type_names = NULL,
        .len = 0,
        .rhs = *start,
    };
}

static MulExprOp TokenKind_to_mul_op(TokenKind t) {
    assert(TokenKind_is_mul_op(t));
    switch (t) {
        case TOKEN_ASTERISK:
            return MUL_EXPR_MUL;
        case TOKEN_DIV:
            return MUL_EXPR_DIV;
        case TOKEN_MOD:
            return MUL_EXPR_MOD;

        default:
            UNREACHABLE();
    }
}

static bool parse_mul_expr_mul_chain(ParserState* s, MulExpr* res) {
    res->len = 0;
    res->mul_chain = NULL;

    uint32_t alloc_len = res->len;
    while (TokenKind_is_mul_op(ParserState_curr_kind(s))) {
        const TokenKind op = ParserState_curr_kind(s);
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->mul_chain,
                            &alloc_len,
                            sizeof *res->mul_chain);
        }

        CastExprAndOp* curr = &res->mul_chain[res->len];
        if (!parse_cast_expr_inplace(s, &curr->rhs)) {
            MulExpr_free_children(res);
            return false;
        }
        curr->op = TokenKind_to_mul_op(op);

        ++res->len;
    }

    res->mul_chain = mycc_realloc(res->mul_chain,
                                  sizeof *res->mul_chain * res->len);

    return true;
}

static bool parse_mul_expr_inplace(ParserState* s, MulExpr* res) {
    if (!parse_cast_expr_inplace(s, &res->lhs)) {
        return false;
    }

    if (!parse_mul_expr_mul_chain(s, res)) {
        return false;
    }

    return true;
}

static bool parse_mul_expr_cast(ParserState* s,
                                MulExpr* res,
                                const CastExpr* start) {
    assert(start);

    res->lhs = *start;

    if (!parse_mul_expr_mul_chain(s, res)) {
        return false;
    }

    return res;
}

void AddExpr_free_children(AddExpr* e);

static bool parse_add_expr_add_chain(ParserState* s, AddExpr* res) {
    res->len = 0;
    res->add_chain = NULL;

    uint32_t alloc_len = res->len;
    while (TokenKind_is_add_op(ParserState_curr_kind(s))) {
        const TokenKind op = ParserState_curr_kind(s);
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->add_chain,
                            &alloc_len,
                            sizeof *res->add_chain);
        }

        MulExprAndOp* curr = &res->add_chain[res->len];
        if (!parse_mul_expr_inplace(s, &curr->rhs)) {
            AddExpr_free_children(res);
            return false;
        }

        curr->op = op == TOKEN_ADD ? ADD_EXPR_ADD : ADD_EXPR_SUB;

        ++res->len;
    }

    res->add_chain = mycc_realloc(res->add_chain,
                                  sizeof *res->add_chain * res->len);

    return true;
}

static bool parse_add_expr_inplace(ParserState* s, AddExpr* res) {
    if (!parse_mul_expr_inplace(s, &res->lhs)) {
        return false;
    }

    if (!parse_add_expr_add_chain(s, res)) {
        return false;
    }

    return res;
}

static bool parse_add_expr_cast(ParserState* s,
                                AddExpr* res,
                                const CastExpr* start) {
    assert(start);

    if (!parse_mul_expr_cast(s, &res->lhs, start)) {
        return false;
    }

    if (!parse_add_expr_add_chain(s, res)) {
        return false;
    }

    return true;
}

static bool parse_shift_expr_shift_chain(ParserState* s, ShiftExpr* res) {
    res->len = 0;
    res->shift_chain = NULL;

    uint32_t alloc_len = res->len;
    while (TokenKind_is_shift_op(ParserState_curr_kind(s))) {
        const TokenKind op = ParserState_curr_kind(s);
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->shift_chain,
                            &alloc_len,
                            sizeof *res->shift_chain);
        }

        AddExprAndOp* curr = &res->shift_chain[res->len];
        if (!parse_add_expr_inplace(s, &curr->rhs)) {
            goto fail;
        }
        curr->op = op == TOKEN_LSHIFT ? SHIFT_EXPR_LEFT : SHIFT_EXPR_RIGHT;

        ++res->len;
    }

    res->shift_chain = mycc_realloc(res->shift_chain,
                                    sizeof *res->shift_chain * res->len);

    return true;

fail:
    ShiftExpr_free_children(res);
    return false;
}

static bool parse_shift_expr_inplace(ParserState* s, ShiftExpr* res) {
    if (!parse_add_expr_inplace(s, &res->lhs)) {
        return false;
    }

    if (!parse_shift_expr_shift_chain(s, res)) {
        return false;
    }
    return true;
}

static bool parse_shift_expr_cast(ParserState* s,
                                  ShiftExpr* res,
                                  const CastExpr* start) {
    assert(start);

    if (!parse_add_expr_cast(s, &res->lhs, start)) {
        return false;
    }

    if (!parse_shift_expr_shift_chain(s, res)) {
        return false;
    }

    return true;
}

void ShiftExpr_free_children(ShiftExpr* e) {
    AddExpr_free_children(&e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        AddExpr_free_children(&e->shift_chain[i].rhs);
    }
    mycc_free(e->shift_chain);
}

static RelExprOp TokenKind_to_rel_op(TokenKind t) {
    assert(TokenKind_is_rel_op(t));
    switch (t) {
        case TOKEN_LT:
            return REL_EXPR_LT;
        case TOKEN_GT:
            return REL_EXPR_GT;
        case TOKEN_LE:
            return REL_EXPR_LE;
        case TOKEN_GE:
            return REL_EXPR_GE;

        default:
            UNREACHABLE();
    }
}

static bool parse_rel_expr_rel_chain(ParserState* s, RelExpr* res) {
    res->len = 0;
    res->rel_chain = NULL;

    uint32_t alloc_len = res->len;
    while (TokenKind_is_rel_op(ParserState_curr_kind(s))) {
        const TokenKind op = ParserState_curr_kind(s);
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->rel_chain,
                            &alloc_len,
                            sizeof *res->rel_chain);
        }

        ShiftExprAndOp* curr = &res->rel_chain[res->len];
        if (!parse_shift_expr_inplace(s, &curr->rhs)) {
            goto fail;
        }
        curr->op = TokenKind_to_rel_op(op);

        ++res->len;
    }

    res->rel_chain = mycc_realloc(res->rel_chain,
                                  sizeof *res->rel_chain * res->len);

    return true;
fail:
    RelExpr_free_children(res);
    return false;
}

static bool parse_rel_expr_inplace(ParserState* s, RelExpr* res) {
    if (!parse_shift_expr_inplace(s, &res->lhs)) {
        return false;
    }

    if (!parse_rel_expr_rel_chain(s, res)) {
        return false;
    }

    return true;
}

static bool parse_rel_expr_cast(ParserState* s,
                                RelExpr* res,
                                const CastExpr* start) {
    assert(start);

    if (!parse_shift_expr_cast(s, &res->lhs, start)) {
        return false;
    }

    if (!parse_rel_expr_rel_chain(s, res)) {
        return false;
    }

    return true;
}

static bool parse_eq_expr_eq_chain(ParserState* s, EqExpr* res) {
    res->len = 0;
    res->eq_chain = NULL;

    uint32_t alloc_len = res->len;
    while (TokenKind_is_eq_op(ParserState_curr_kind(s))) {
        const TokenKind op = ParserState_curr_kind(s);
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->eq_chain,
                            &alloc_len,
                            sizeof *res->eq_chain);
        }

        RelExprAndOp* curr = &res->eq_chain[res->len];
        if (!parse_rel_expr_inplace(s, &curr->rhs)) {
            goto fail;
        }
        curr->op = op == TOKEN_EQ ? EQ_EXPR_EQ : EQ_EXPR_NE;

        ++res->len;
    }

    res->eq_chain = mycc_realloc(res->eq_chain,
                                 sizeof *res->eq_chain * res->len);

    return true;
fail:
    EqExpr_free_children(res);
    return false;
}

static bool parse_eq_expr_inplace(ParserState* s, EqExpr* res) {
    assert(res);

    if (!parse_rel_expr_inplace(s, &res->lhs)) {
        return false;
    }

    if (!parse_eq_expr_eq_chain(s, res)) {
        return false;
    }

    return true;
}

static EqExpr* parse_eq_expr_cast(ParserState* s, const CastExpr* start) {
    assert(start);

    EqExpr* res = mycc_alloc(sizeof *res);
    if (!parse_rel_expr_cast(s, &res->lhs, start)) {
        mycc_free(res);
        return NULL;
    }

    if (!parse_eq_expr_eq_chain(s, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

static bool parse_and_expr_rest(ParserState* s, AndExpr* res) {
    assert(res->eq_exprs);

    res->len = 1;
    uint32_t alloc_len = res->len;

    while (ParserState_curr_kind(s) == TOKEN_AND) {
        ParserState_accept_it(s);
        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->eq_exprs,
                            &alloc_len,
                            sizeof *res->eq_exprs);
        }
        if (!parse_eq_expr_inplace(s, &res->eq_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->eq_exprs = mycc_realloc(res->eq_exprs,
                                 sizeof *res->eq_exprs * res->len);

    return true;
fail:
    AndExpr_free_children(res);
    return false;
}

static bool parse_and_expr_inplace(ParserState* s, AndExpr* res) {
    res->eq_exprs = mycc_alloc(sizeof *res->eq_exprs);
    if (!parse_eq_expr_inplace(s, res->eq_exprs)) {
        mycc_free(res->eq_exprs);
        return false;
    }

    if (!parse_and_expr_rest(s, res)) {
        return false;
    }
    return true;
}

static AndExpr* parse_and_expr_cast(ParserState* s, const CastExpr* start) {
    assert(start);

    EqExpr* eq_exprs = parse_eq_expr_cast(s, start);
    if (!eq_exprs) {
        return NULL;
    }

    AndExpr* res = mycc_alloc(sizeof *res);
    res->eq_exprs = eq_exprs;

    if (!parse_and_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

static bool parse_xor_expr_rest(ParserState* s, XorExpr* res) {
    assert(res->and_exprs);

    res->len = 1;

    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_XOR) {
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->and_exprs,
                            &alloc_len,
                            sizeof *res->and_exprs);
        }

        if (!parse_and_expr_inplace(s, &res->and_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->and_exprs = mycc_realloc(res->and_exprs,
                                  sizeof *res->and_exprs * res->len);
    return true;

fail:
    XorExpr_free_children(res);
    return false;
}

static bool parse_xor_expr_inplace(ParserState* s, XorExpr* res) {
    assert(res);

    res->and_exprs = mycc_alloc(sizeof *res->and_exprs);
    if (!parse_and_expr_inplace(s, res->and_exprs)) {
        mycc_free(res->and_exprs);
        return false;
    }

    if (!parse_xor_expr_rest(s, res)) {
        return false;
    }

    return true;
}

static XorExpr* parse_xor_expr_cast(ParserState* s, const CastExpr* start) {
    assert(start);

    AndExpr* and_exprs = parse_and_expr_cast(s, start);
    if (!and_exprs) {
        return NULL;
    }

    XorExpr* res = mycc_alloc(sizeof *res);
    res->and_exprs = and_exprs;

    if (!parse_xor_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

static bool parse_or_expr_rest(ParserState* s, OrExpr* res) {
    res->len = 1;

    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_OR) {
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->xor_exprs,
                            &alloc_len,
                            sizeof *res->xor_exprs);
        }

        if (!parse_xor_expr_inplace(s, &res->xor_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->xor_exprs = mycc_realloc(res->xor_exprs,
                                  sizeof *res->xor_exprs * res->len);

    return true;

fail:
    OrExpr_free_children(res);
    return false;
}

static bool parse_or_expr_inplace(ParserState* s, OrExpr* res) {
    assert(res);

    res->xor_exprs = mycc_alloc(sizeof *res->xor_exprs);
    if (!parse_xor_expr_inplace(s, res->xor_exprs)) {
        mycc_free(res->xor_exprs);
        return false;
    }

    if (!parse_or_expr_rest(s, res)) {
        return false;
    }

    return true;
}

static OrExpr* parse_or_expr_cast(ParserState* s, const CastExpr* start) {
    assert(start);

    XorExpr* xor_exprs = parse_xor_expr_cast(s, start);
    if (!xor_exprs) {
        return NULL;
    }

    OrExpr* res = mycc_alloc(sizeof *res);
    res->xor_exprs = xor_exprs;

    if (!parse_or_expr_rest(s, res)) {
        return NULL;
    }

    return res;
}

static bool parse_log_and_expr_rest(ParserState* s, LogAndExpr* res) {
    assert(res);
    res->len = 1;
    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_LAND) {
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->or_exprs,
                            &alloc_len,
                            sizeof *res->or_exprs);
        }

        if (!parse_or_expr_inplace(s, &res->or_exprs[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->or_exprs = mycc_realloc(res->or_exprs,
                                 sizeof *res->or_exprs * res->len);
    return true;
fail:
    LogAndExpr_free_children(res);
    return false;
}

static bool parse_log_and_expr_inplace(ParserState* s, LogAndExpr* res) {
    assert(res);

    res->or_exprs = mycc_alloc(sizeof *res->or_exprs);
    if (!parse_or_expr_inplace(s, res->or_exprs)) {
        mycc_free(res->or_exprs);
        return false;
    }

    if (!parse_log_and_expr_rest(s, res)) {
        return false;
    }

    return true;
}

static LogAndExpr* parse_log_and_expr_cast(ParserState* s,
                                           const CastExpr* start) {
    assert(start);

    OrExpr* or_exprs = parse_or_expr_cast(s, start);
    if (!or_exprs) {
        return NULL;
    }

    LogAndExpr* res = mycc_alloc(sizeof *res);
    res->or_exprs = or_exprs;

    if (!parse_log_and_expr_rest(s, res)) {
        mycc_free(res);
        return NULL;
    }

    return res;
}

static bool parse_log_or_expr_ops(ParserState* s, LogOrExpr* res) {
    assert(res);
    assert(res->len == 1);

    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_LOR) {
        ParserState_accept_it(s);

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->log_ands,
                            &alloc_len,
                            sizeof *res->log_ands);
        }

        if (!parse_log_and_expr_inplace(s, &res->log_ands[res->len])) {
            goto fail;
        }

        ++res->len;
    }

    res->log_ands = mycc_realloc(res->log_ands,
                                 sizeof *res->log_ands * res->len);

    return true;
fail:
    LogOrExpr_free_children(res);
    return false;
}

static bool parse_log_or_expr_inplace(ParserState* s, LogOrExpr* res) {
    res->log_ands = mycc_alloc(sizeof *res->log_ands);
    if (!parse_log_and_expr_inplace(s, res->log_ands)) {
        mycc_free(res->log_ands);
        return false;
    }
    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        return false;
    }

    return true;
}

static bool parse_log_or_expr_cast(ParserState* s,
                                   LogOrExpr* res,
                                   const CastExpr* start) {
    assert(start);

    res->log_ands = parse_log_and_expr_cast(s, start);
    if (!res->log_ands) {
        return false;
    }

    res->len = 1;

    if (!parse_log_or_expr_ops(s, res)) {
        return false;
    }

    return true;
}

static bool parse_cond_expr_conditionals(ParserState* s, CondExpr* res) {
    res->len = 0;
    res->conditionals = NULL;

    uint32_t alloc_len = res->len;
    while (ParserState_curr_kind(s) == TOKEN_QMARK) {
        ParserState_accept_it(s);

        Expr expr;
        if (!parse_expr_inplace(s, &expr)) {
            goto fail;
        }

        if (!ParserState_accept(s, TOKEN_COLON)) {
            Expr_free_children(&expr);
            goto fail;
        }

        LogOrExpr new_last;
        if (!parse_log_or_expr_inplace(s, &new_last)) {
            Expr_free_children(&expr);
            goto fail;
        }

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->conditionals,
                            &alloc_len,
                            sizeof *res->conditionals);
        }

        res->conditionals[res->len] = (LogOrAndExpr){
            .log_or = res->last_else,
            .expr = expr,
        };

        res->last_else = new_last;

        ++res->len;
    }

    res->conditionals = mycc_realloc(res->conditionals,
                                     sizeof *res->conditionals * res->len);
    return true;

fail:
    CondExpr_free_children(res);
    return false;
}

static bool parse_cond_expr_inplace(ParserState* s, CondExpr* res) {
    assert(res);

    if (!parse_log_or_expr_inplace(s, &res->last_else)) {
        return false;
    }

    if (!parse_cond_expr_conditionals(s, res)) {
        return false;
    }

    return true;
}

static bool parse_cond_expr_cast(ParserState* s,
                                 CondExpr* res,
                                 const CastExpr* start) {
    assert(start);

    if (!parse_log_or_expr_cast(s, &res->last_else, start)) {
        return false;
    }

    if (!parse_cond_expr_conditionals(s, res)) {
        return false;
    }
    return true;
}

bool parse_const_expr_inplace(ParserState* s, ConstExpr* res) {
    return parse_cond_expr_inplace(s, &res->expr);
}

ConstExpr* parse_const_expr(ParserState* s) {
    ConstExpr* res = mycc_alloc(sizeof *res);
    if (!parse_const_expr_inplace(s, res)) {
        mycc_free(res);
        return NULL;
    }
    return res;
}

typedef struct {
    bool is_valid;
    bool is_cond;
    union {
        UnaryExpr unary;
        CondExpr cond;
    };
} UnaryOrCond;

static UnaryOrCond parse_unary_or_cond(ParserState* s) {
    UnaryOrCond res = {
        .is_valid = false,
        .is_cond = false,
    };
    if (ParserState_curr_kind(s) == TOKEN_LBRACKET
        && next_is_type_name(s)) {
        const SourceLoc start_bracket_loc = ParserState_curr_loc(s);
        ParserState_accept_it(s);

        TypeName* type_name = parse_type_name(s);
        if (!type_name) {
            return res;
        }

        if (!ParserState_accept(s, TOKEN_RBRACKET)) {
            TypeName_free(type_name);
            return res;
        }

        if (ParserState_curr_kind(s) == TOKEN_LBRACE) {
            res.is_cond = false;
            if (!parse_unary_expr_type_name(s,
                                            &res.unary,
                                            NULL,
                                            0,
                                            type_name,
                                            start_bracket_loc)) {
                return res;
            }
        } else {
            CastExpr cast;
            if (!parse_cast_expr_type_name(s,
                                           &cast,
                                           type_name,
                                           start_bracket_loc)) {
                return res;
            }

            res.is_cond = true;
            if (!parse_cond_expr_cast(s, &res.cond, &cast)) {
                return res;
            }
        }
    } else {
        res.is_cond = false;
        if (!parse_unary_expr_inplace(s, &res.unary)) {
            return res;
        }
    }

    res.is_valid = true;
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

static AssignExprOp TokenKind_to_assign_op(TokenKind t) {
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

    UnaryOrCond opt = parse_unary_or_cond(s);
    if (!opt.is_valid) {
        return false;
    } else if (opt.is_cond) {
        res->value = opt.cond;
        return true;
    }

    UnaryExpr last_unary = opt.unary;

    uint32_t alloc_len = res->len;
    while (is_assign_op(ParserState_curr_kind(s))) {
        TokenKind op = ParserState_curr_kind(s);
        ParserState_accept_it(s);

        opt = parse_unary_or_cond(s);
        if (!opt.is_valid) {
            UnaryExpr_free_children(&last_unary);
            goto fail;
        } else if (opt.is_cond) {
            res->value = opt.cond;
            ++res->len;
            res->assign_chain = mycc_realloc(res->assign_chain,
                                             sizeof *res->assign_chain
                                                 * res->len);
            res->assign_chain[res->len - 1] = (UnaryAndOp){
                .op = TokenKind_to_assign_op(op),
                .unary = last_unary,
            };
            return true;
        }

        UnaryExpr new_last = opt.unary;

        if (res->len == alloc_len) {
            mycc_grow_alloc((void**)&res->assign_chain,
                            &alloc_len,
                            sizeof *res->assign_chain);
        }

        res->assign_chain[res->len] = (UnaryAndOp){
            .op = TokenKind_to_assign_op(op),
            .unary = last_unary,
        };
        last_unary = new_last;

        ++res->len;
    }

    res->assign_chain = mycc_realloc(res->assign_chain,
                                     sizeof *res->assign_chain * res->len);

    const CastExpr cast = CastExpr_create_unary(&last_unary);
    if (!parse_cond_expr_cast(s, &res->value, &cast)) {
        goto fail;
    }

    return true;
fail:
    for (uint32_t i = 0; i < res->len; ++i) {
        UnaryExpr_free_children(&res->assign_chain[i].unary);
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

void CastExpr_free_children(CastExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        TypeName_free_children(&e->type_names[i]);
    }
    mycc_free(e->type_names);
    UnaryExpr_free_children(&e->rhs);
}

void CastExpr_free(CastExpr* e) {
    CastExpr_free_children(e);
    mycc_free(e);
}

void MulExpr_free_children(MulExpr* e) {
    CastExpr_free_children(&e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        CastExpr_free_children(&e->mul_chain[i].rhs);
    }
    mycc_free(e->mul_chain);
}

void AddExpr_free_children(AddExpr* e) {
    MulExpr_free_children(&e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        MulExpr_free_children(&e->add_chain[i].rhs);
    }
    mycc_free(e->add_chain);
}

void RelExpr_free_children(RelExpr* e) {
    ShiftExpr_free_children(&e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        ShiftExpr_free_children(&e->rel_chain[i].rhs);
    }
    mycc_free(e->rel_chain);
}

void EqExpr_free_children(EqExpr* e) {
    RelExpr_free_children(&e->lhs);
    for (uint32_t i = 0; i < e->len; ++i) {
        RelExpr_free_children(&e->eq_chain[i].rhs);
    }
    mycc_free(e->eq_chain);
}

void AndExpr_free_children(AndExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        EqExpr_free_children(&e->eq_exprs[i]);
    }
    mycc_free(e->eq_exprs);
}

void XorExpr_free_children(XorExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        AndExpr_free_children(&e->and_exprs[i]);
    }
    mycc_free(e->and_exprs);
}

void OrExpr_free_children(OrExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        XorExpr_free_children(&e->xor_exprs[i]);
    }
    mycc_free(e->xor_exprs);
}

void LogAndExpr_free_children(LogAndExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        OrExpr_free_children(&e->or_exprs[i]);
    }
    mycc_free(e->or_exprs);
}

void LogOrExpr_free_children(LogOrExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        LogAndExpr_free_children(&e->log_ands[i]);
    }
    mycc_free(e->log_ands);
}

void CondExpr_free_children(CondExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        LogOrAndExpr* item = &e->conditionals[i];
        LogOrExpr_free_children(&item->log_or);
        Expr_free_children(&item->expr);
    }
    mycc_free(e->conditionals);

    LogOrExpr_free_children(&e->last_else);
}

void ConstExpr_free_children(ConstExpr* e) {
    CondExpr_free_children(&e->expr);
}

void ConstExpr_free(ConstExpr* e) {
    ConstExpr_free_children(e);
    mycc_free(e);
}

void AssignExpr_free_children(struct AssignExpr* e) {
    for (uint32_t i = 0; i < e->len; ++i) {
        UnaryExpr_free_children(&e->assign_chain[i].unary);
    }
    mycc_free(e->assign_chain);

    CondExpr_free_children(&e->value);
}

void AssignExpr_free(struct AssignExpr* e) {
    AssignExpr_free_children(e);
    mycc_free(e);
}
