#include "parser_test_util.h"

#include "util/macro_util.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

void check_value(Value got, Value expected) {
    ASSERT_VALUE_KIND(got.kind, expected.kind);
    if (ValueKind_is_sint(got.kind)) {
        ASSERT_INT(got.sint_val, expected.sint_val);
    } else if (ValueKind_is_uint(got.kind)) {
        ASSERT_UINT(got.uint_val, expected.uint_val);
    } else {
        ASSERT_DOUBLE(got.float_val, expected.float_val, 0.0001);
    }
}

void check_identifier(const Identifier* id, Str spell, const TokenArr* arr) {
    if (Str_valid(spell)) {
        ASSERT_STR(StrBuf_as_str(&arr->vals[id->info.token_idx].spelling), spell);
    } else {
        ASSERT_NULL(id);
    }
}

void check_primary_expr_id(const PrimaryExpr* e, Str spell, const TokenArr* arr) {
    ASSERT(e->kind == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_NOT_NULL(e->identifier);
    check_identifier(e->identifier, spell, arr);
}

void check_primary_expr_val(const PrimaryExpr* e,
                            Value val,
                            const TokenArr* arr) {
    ASSERT(e->kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(e->constant.kind != CONSTANT_ENUM);

    ASSERT(e->constant.kind == CONSTANT_VAL);
    check_value(arr->vals[e->constant.info.token_idx].val, val);
}

void check_postfix_expr_id(const PostfixExpr* e, Str spell, const TokenArr* arr) {
    ASSERT(e->is_primary);
    check_primary_expr_id(&e->primary, spell, arr);
}

void check_postfix_expr_val(const PostfixExpr* e, Value val, const TokenArr* arr) {
    ASSERT(e->is_primary);
    check_primary_expr_val(&e->primary, val, arr);
}

static void check_unary_expr_empty(const UnaryExpr* e) {
    ASSERT_UINT(e->len, (uint32_t)0);
    ASSERT_NULL(e->ops_before);

    ASSERT(e->kind == UNARY_POSTFIX);
}

void check_unary_expr_id(const UnaryExpr* e, Str spell, const TokenArr* arr) {
    check_unary_expr_empty(e);

    check_postfix_expr_id(&e->postfix, spell, arr);
}

void check_unary_expr_val(const UnaryExpr* e, Value val, const TokenArr* arr) {
    check_unary_expr_empty(e);

    check_postfix_expr_val(&e->postfix, val, arr);
}

static void check_cast_expr_empty(const CastExpr* e) {
    ASSERT_UINT(e->len, (uint32_t)0);
    ASSERT_NULL(e->type_names);
}

void check_cast_expr_id(const CastExpr* e, Str spell, const TokenArr* arr) {
    check_cast_expr_empty(e);
    check_unary_expr_id(&e->rhs, spell, arr);
}

void check_cast_expr_val(const CastExpr* e, Value val, const TokenArr* arr) {
    check_cast_expr_empty(e);
    check_unary_expr_val(&e->rhs, val, arr);
}

static void check_shift_expr_empty(const ShiftExpr* e) {
    const uint32_t zero = (uint32_t)0;
    ASSERT_UINT(e->len, (uint32_t)0);
    ASSERT_UINT(e->lhs.len, zero);
    ASSERT_UINT(e->lhs.lhs.len, zero);
}

void check_shift_expr_id(const ShiftExpr* e, Str spell, const TokenArr* arr) {
    check_shift_expr_empty(e);
    check_cast_expr_id(&e->lhs.lhs.lhs, spell, arr);
}

void check_shift_expr_val(const ShiftExpr* e, Value val, const TokenArr* arr) {
    check_shift_expr_empty(e);
    check_cast_expr_val(&e->lhs.lhs.lhs, val, arr);
}

static void check_cond_expr_empty(const CondExpr* e) {
    const uint32_t one = (uint32_t)1;
    const uint32_t zero = (uint32_t)0;
    ASSERT_UINT(e->len, zero);
    ASSERT_UINT(e->last_else.len, one);
    ASSERT_UINT(e->last_else.log_ands->len, one);
    ASSERT_UINT(e->last_else.log_ands->or_exprs->len, one);
    ASSERT_UINT(e->last_else.log_ands->or_exprs->xor_exprs->len, one);
    ASSERT_UINT(e->last_else.log_ands->or_exprs->xor_exprs->and_exprs->len,
                  one);
    ASSERT_UINT(
        e->last_else.log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->len,
        zero);
    ASSERT_UINT(e->last_else.log_ands->or_exprs->xor_exprs->and_exprs
                      ->eq_exprs->lhs.len,
                  zero);
    ASSERT_UINT(e->last_else.log_ands->or_exprs->xor_exprs->and_exprs
                      ->eq_exprs->lhs.lhs.len,
                  zero);
}

void check_cond_expr_id(const CondExpr* e, Str spell, const TokenArr* arr) {
    check_cond_expr_empty(e);
    check_shift_expr_id(&e->last_else.log_ands->or_exprs->xor_exprs->and_exprs
                            ->eq_exprs->lhs.lhs,
                        spell, arr);
}

void check_cond_expr_val(const CondExpr* e, Value val, const TokenArr* arr) {
    check_cond_expr_empty(e);
    check_shift_expr_val(&e->last_else.log_ands->or_exprs->xor_exprs->and_exprs
                             ->eq_exprs->lhs.lhs,
                         val, arr);
}

static void check_const_expr_empty(const ConstExpr* e) {
    ASSERT_UINT(e->expr.len, (uint32_t)0);
}

void check_const_expr_id(const ConstExpr* e, Str spell, const TokenArr* arr) {
    check_const_expr_empty(e);
    check_cond_expr_id(&e->expr, spell, arr);
}

void check_const_expr_val(const ConstExpr* e, Value val, const TokenArr* arr) {
    check_const_expr_empty(e);
    check_cond_expr_val(&e->expr, val, arr);
}

static void check_assign_expr_empty(const AssignExpr* e) {
    ASSERT_UINT(e->len, (uint32_t)0);
    ASSERT_NULL(e->assign_chain);
}

void check_assign_expr_id(const AssignExpr* e, Str spell, const TokenArr* arr) {
    check_assign_expr_empty(e);
    check_cond_expr_id(&e->value, spell, arr);
}

void check_assign_expr_val(const AssignExpr* e, Value val, const TokenArr* arr) {
    check_assign_expr_empty(e);
    check_cond_expr_val(&e->value, val, arr);
}

static void check_expr_empty(const Expr* e) {
    ASSERT_UINT(e->len, (uint32_t)1);
    ASSERT_NOT_NULL(e->assign_exprs);
}

void check_expr_id(const Expr* e, Str spell, const TokenArr* arr) {
    check_expr_empty(e);
    check_assign_expr_id(&e->assign_exprs[0], spell, arr);
}

void check_expr_val(const Expr* e, Value val, const TokenArr* arr) {
    check_expr_empty(e);
    check_assign_expr_val(&e->assign_exprs[0], val, arr);
}

