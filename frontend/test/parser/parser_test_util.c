#include "parser_test_util.h"

#include "util/macro_util.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

void check_value(Value got, Value expected) {
    ASSERT_VALUE_KIND(got.kind, expected.kind);
    if (ValueKind_is_sint(got.kind)) {
        ASSERT_INTMAX_T(got.sint_val, expected.sint_val);
    } else if (ValueKind_is_uint(got.kind)) {
        ASSERT_UINTMAX_T(got.uint_val, expected.uint_val);
    } else {
        ASSERT_DOUBLE(got.float_val, expected.float_val, 0.0001);
    }
}

void check_identifier(Identifier* id, const char* spell) {
    if (spell != NULL) {
        ASSERT_STR(Str_get_data(&id->spelling), spell);
    } else {
        ASSERT_NULL(id);
    }
}

void check_primary_expr_id(const PrimaryExpr* e, const char* spell) {
    ASSERT(e->kind == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_NOT_NULL(e->identifier);
    ASSERT_STR(Str_get_data(&e->identifier->spelling), spell);
}

void check_primary_expr_val(const PrimaryExpr* e,
                            Value val) {
    ASSERT(e->kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(e->constant.kind != CONSTANT_ENUM);

    ASSERT(e->constant.kind == CONSTANT_VAL);
    check_value(e->constant.val, val);
}

void check_postfix_expr_id(const PostfixExpr* e, const char* spell) {
    ASSERT(e->is_primary);
    check_primary_expr_id(e->primary, spell);
}

void check_postfix_expr_val(const PostfixExpr* e, Value val) {
    ASSERT(e->is_primary);
    check_primary_expr_val(e->primary, val);
}

static void check_unary_expr_empty(const UnaryExpr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->ops_before);

    ASSERT(e->kind == UNARY_POSTFIX);
}

void check_unary_expr_id(const UnaryExpr* e, const char* spell) {
    check_unary_expr_empty(e);

    check_postfix_expr_id(e->postfix, spell);
}

void check_unary_expr_val(const UnaryExpr* e, Value val) {
    check_unary_expr_empty(e);

    check_postfix_expr_val(e->postfix, val);
}

static void check_cast_expr_empty(const CastExpr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->type_names);
}

void check_cast_expr_id(const CastExpr* e, const char* spell) {
    check_cast_expr_empty(e);
    check_unary_expr_id(e->rhs, spell);
}

void check_cast_expr_val(const CastExpr* e, Value val) {
    check_cast_expr_empty(e);
    check_unary_expr_val(e->rhs, val);
}

static void check_shift_expr_empty(const ShiftExpr* e) {
    const size_t zero = (size_t)0;
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NOT_NULL(e->lhs);
    ASSERT_SIZE_T(e->lhs->len, zero);
    ASSERT_NOT_NULL(e->lhs->lhs);
    ASSERT_SIZE_T(e->lhs->lhs->len, zero);
    ASSERT_NOT_NULL(e->lhs->lhs->lhs);
}

void check_shift_expr_id(const ShiftExpr* e, const char* spell) {
    check_shift_expr_empty(e);
    check_cast_expr_id(e->lhs->lhs->lhs, spell);
}

void check_shift_expr_val(const ShiftExpr* e, Value val) {
    check_shift_expr_empty(e);
    check_cast_expr_val(e->lhs->lhs->lhs, val);
}

static void check_cond_expr_empty(const CondExpr* e) {
    const size_t one = (size_t)1;
    const size_t zero = (size_t)0;
    ASSERT_SIZE_T(e->len, zero);
    ASSERT_SIZE_T(e->last_else->len, one);
    ASSERT_SIZE_T(e->last_else->log_ands->len, one);
    ASSERT_SIZE_T(e->last_else->log_ands->or_exprs->len, one);
    ASSERT_SIZE_T(e->last_else->log_ands->or_exprs->xor_exprs->len, one);
    ASSERT_SIZE_T(e->last_else->log_ands->or_exprs->xor_exprs->and_exprs->len,
                  one);
    ASSERT_SIZE_T(
        e->last_else->log_ands->or_exprs->xor_exprs->and_exprs->eq_exprs->len,
        zero);
    ASSERT_SIZE_T(e->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                      ->eq_exprs->lhs->len,
                  zero);
    ASSERT_SIZE_T(e->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                      ->eq_exprs->lhs->lhs->len,
                  zero);
}

void check_cond_expr_id(const CondExpr* e, const char* spell) {
    check_cond_expr_empty(e);
    check_shift_expr_id(e->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                            ->eq_exprs->lhs->lhs,
                        spell);
}

void check_cond_expr_val(const CondExpr* e, Value val) {
    check_cond_expr_empty(e);
    check_shift_expr_val(e->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                             ->eq_exprs->lhs->lhs,
                         val);
}

static void check_const_expr_empty(const ConstExpr* e) {
    ASSERT_SIZE_T(e->expr.len, (size_t)0);
    ASSERT_NOT_NULL(e->expr.last_else);
}

void check_const_expr_id(const ConstExpr* e, const char* spell) {
    check_const_expr_empty(e);
    check_cond_expr_id(&e->expr, spell);
}

void check_const_expr_val(const ConstExpr* e, Value val) {
    check_const_expr_empty(e);
    check_cond_expr_val(&e->expr, val);
}

static void check_assign_expr_empty(const AssignExpr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->assign_chain);
    ASSERT_NOT_NULL(e->value);
}

void check_assign_expr_id(const AssignExpr* e, const char* spell) {
    check_assign_expr_empty(e);
    check_cond_expr_id(e->value, spell);
}

void check_assign_expr_val(const AssignExpr* e, Value val) {
    check_assign_expr_empty(e);
    check_cond_expr_val(e->value, val);
}

static void check_expr_empty(const Expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)1);
    ASSERT_NOT_NULL(e->assign_exprs);
}

void check_expr_id(const Expr* e, const char* spell) {
    check_expr_empty(e);
    check_assign_expr_id(&e->assign_exprs[0], spell);
}

void check_expr_val(const Expr* e, Value val) {
    check_expr_empty(e);
    check_assign_expr_val(&e->assign_exprs[0], val);
}

