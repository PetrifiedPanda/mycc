#include "parser_test_util.h"

#include "util/macro_util.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

void check_int_value(struct int_value got, struct int_value expected) {
    ASSERT_INT_VALUE_KIND(got.kind, expected.kind);
    if (int_value_is_signed(got.kind)) {
        ASSERT_INTMAX_T(got.int_val, expected.int_val);
    } else {
        ASSERT_UINTMAX_T(got.uint_val, expected.uint_val);
    }
}

void check_float_value(struct float_value got, struct float_value expected) {
    ASSERT_FLOAT_VALUE_KIND(got.kind, expected.kind);
    ASSERT_DOUBLE(got.val, expected.val, 0.0001);
}

void check_identifier(struct identifier* id, const char* spell) {
    if (spell != NULL) {
        ASSERT_STR(str_get_data(&id->spelling), spell);
    } else {
        ASSERT_NULL(id);
    }
}

void check_primary_expr_id(const struct primary_expr* e, const char* spell) {
    ASSERT(e->kind == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_NOT_NULL(e->identifier);
    ASSERT_STR(str_get_data(&e->identifier->spelling), spell);
}

void check_primary_expr_int(const struct primary_expr* e,
                            struct int_value val) {
    ASSERT(e->kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(e->constant.kind != CONSTANT_ENUM);

    ASSERT(e->constant.kind == CONSTANT_INT);
    check_int_value(e->constant.int_val, val);
}

void check_primary_expr_float(const struct primary_expr* e,
                              struct float_value val) {
    ASSERT(e->kind == PRIMARY_EXPR_CONSTANT);
    ASSERT(e->constant.kind != CONSTANT_ENUM);

    ASSERT(e->constant.kind == CONSTANT_FLOAT);
    check_float_value(e->constant.float_val, val);
}

void check_postfix_expr_id(const struct postfix_expr* e, const char* spell) {
    ASSERT(e->is_primary);
    check_primary_expr_id(e->primary, spell);
}

void check_postfix_expr_int(const struct postfix_expr* e,
                            struct int_value val) {
    ASSERT(e->is_primary);
    check_primary_expr_int(e->primary, val);
}

void check_postfix_expr_float(const struct postfix_expr* e,
                              struct float_value val) {
    ASSERT(e->is_primary);
    check_primary_expr_float(e->primary, val);
}

static void check_unary_expr_empty(const struct unary_expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->ops_before);

    ASSERT(e->kind == UNARY_POSTFIX);
}

void check_unary_expr_id(const struct unary_expr* e, const char* spell) {
    check_unary_expr_empty(e);

    check_postfix_expr_id(e->postfix, spell);
}

void check_unary_expr_int(const struct unary_expr* e, struct int_value val) {
    check_unary_expr_empty(e);

    check_postfix_expr_int(e->postfix, val);
}

void check_unary_expr_float(const struct unary_expr* e,
                            struct float_value val) {
    check_unary_expr_empty(e);

    check_postfix_expr_float(e->postfix, val);
}

static void check_cast_expr_empty(const struct cast_expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->type_names);
}

void check_cast_expr_id(const struct cast_expr* e, const char* spell) {
    check_cast_expr_empty(e);
    check_unary_expr_id(e->rhs, spell);
}

void check_cast_expr_int(const struct cast_expr* e, struct int_value val) {
    check_cast_expr_empty(e);
    check_unary_expr_int(e->rhs, val);
}

void check_cast_expr_float(const struct cast_expr* e, struct float_value val) {
    check_cast_expr_empty(e);
    check_unary_expr_float(e->rhs, val);
}

static void check_shift_expr_empty(const struct shift_expr* e) {
    const size_t zero = (size_t)0;
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NOT_NULL(e->lhs);
    ASSERT_SIZE_T(e->lhs->len, zero);
    ASSERT_NOT_NULL(e->lhs->lhs);
    ASSERT_SIZE_T(e->lhs->lhs->len, zero);
    ASSERT_NOT_NULL(e->lhs->lhs->lhs);
}

void check_shift_expr_id(const struct shift_expr* e, const char* spell) {
    check_shift_expr_empty(e);
    check_cast_expr_id(e->lhs->lhs->lhs, spell);
}

void check_shift_expr_int(const struct shift_expr* e, struct int_value val) {
    check_shift_expr_empty(e);
    check_cast_expr_int(e->lhs->lhs->lhs, val);
}

void check_shift_expr_float(const struct shift_expr* e,
                            struct float_value val) {
    check_shift_expr_empty(e);
    check_cast_expr_float(e->lhs->lhs->lhs, val);
}

static void check_cond_expr_empty(const struct cond_expr* e) {
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

void check_cond_expr_id(const struct cond_expr* e, const char* spell) {
    check_cond_expr_empty(e);
    check_shift_expr_id(e->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                            ->eq_exprs->lhs->lhs,
                        spell);
}

void check_cond_expr_int(const struct cond_expr* e, struct int_value val) {
    check_cond_expr_empty(e);
    check_shift_expr_int(e->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                             ->eq_exprs->lhs->lhs,
                         val);
}

void check_cond_expr_float(const struct cond_expr* e, struct float_value val) {
    check_cond_expr_empty(e);
    check_shift_expr_float(e->last_else->log_ands->or_exprs->xor_exprs
                               ->and_exprs->eq_exprs->lhs->lhs,
                           val);
}

static void check_const_expr_empty(const struct const_expr* e) {
    ASSERT_SIZE_T(e->expr.len, (size_t)0);
    ASSERT_NOT_NULL(e->expr.last_else);
}

void check_const_expr_id(const struct const_expr* e, const char* spell) {
    check_const_expr_empty(e);
    check_cond_expr_id(&e->expr, spell);
}

void check_const_expr_int(const struct const_expr* e, struct int_value val) {
    check_const_expr_empty(e);
    check_cond_expr_int(&e->expr, val);
}

void check_const_expr_float(const struct const_expr* e,
                            struct float_value val) {
    check_const_expr_empty(e);
    check_cond_expr_float(&e->expr, val);
}

static void check_assign_expr_empty(const struct assign_expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->assign_chain);
    ASSERT_NOT_NULL(e->value);
}

void check_assign_expr_id(const struct assign_expr* e, const char* spell) {
    check_assign_expr_empty(e);
    check_cond_expr_id(e->value, spell);
}

void check_assign_expr_int(const struct assign_expr* e, struct int_value val) {
    check_assign_expr_empty(e);
    check_cond_expr_int(e->value, val);
}

void check_assign_expr_float(const struct assign_expr* e,
                             struct float_value val) {
    check_assign_expr_empty(e);
    check_cond_expr_float(e->value, val);
}

static void check_expr_empty(const struct expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)1);
    ASSERT_NOT_NULL(e->assign_exprs);
}

void check_expr_id(const struct expr* e, const char* spell) {
    check_expr_empty(e);
    check_assign_expr_id(&e->assign_exprs[0], spell);
}

void check_expr_int(const struct expr* e, struct int_value val) {
    check_expr_empty(e);
    check_assign_expr_int(&e->assign_exprs[0], val);
}

void check_expr_float(const struct expr* e, struct float_value val) {
    check_expr_empty(e);
    check_assign_expr_float(&e->assign_exprs[0], val);
}

