#include "parser_test_util.h"

#include "util/annotations.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

void check_value(struct value got, struct value expected) {
    ASSERT_VALUE_TYPE(got.type, expected.type);
    if (value_is_int(got.type)) {
        ASSERT_INTMAX_T(got.int_val, expected.int_val);
    } else if (value_is_uint(got.type)) {
        ASSERT_UINTMAX_T(got.uint_val, expected.uint_val);
    } else {
        ASSERT_LONG_DOUBLE(got.float_val, expected.float_val, 0.0001);
    }
}

void check_identifier(struct identifier* id, const char* spell) {
    if (spell != NULL) {
        ASSERT_STR(id->spelling, spell);
    } else {
        ASSERT_NULL(id);
    }
}

void check_primary_expr_id(const struct primary_expr* e, const char* spell) {
    ASSERT(e->type == PRIMARY_EXPR_IDENTIFIER);
    ASSERT_NOT_NULL(e->identifier);
    ASSERT_STR(e->identifier->spelling, spell);
}

void check_primary_expr_const(const struct primary_expr* e, struct value val) {
    ASSERT(e->type == PRIMARY_EXPR_CONSTANT);
    ASSERT(e->constant.type != CONSTANT_ENUM);

    if (value_is_float(val.type)) {
        ASSERT(e->constant.type == CONSTANT_FLOAT);
    } else {
        ASSERT(e->constant.type == CONSTANT_INT);
    }
    check_value(e->constant.val, val);
}

void check_postfix_expr_id(const struct postfix_expr* e, const char* spell) {
    ASSERT(e->is_primary);
    check_primary_expr_id(e->primary, spell);
}

void check_postfix_expr_const(const struct postfix_expr* e, struct value val) {
    ASSERT(e->is_primary);
    check_primary_expr_const(e->primary, val);
}

static void check_unary_expr_empty(const struct unary_expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->ops_before);

    ASSERT(e->type == UNARY_POSTFIX);
}

void check_unary_expr_id(const struct unary_expr* e, const char* spell) {
    check_unary_expr_empty(e);

    check_postfix_expr_id(e->postfix, spell);
}

void check_unary_expr_const(const struct unary_expr* e, struct value val) {
    check_unary_expr_empty(e);

    check_postfix_expr_const(e->postfix, val);
}

static void check_cast_expr_empty(const struct cast_expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)0);
    ASSERT_NULL(e->type_names);
}

void check_cast_expr_id(const struct cast_expr* e, const char* spell) {
    check_cast_expr_empty(e);
    check_unary_expr_id(e->rhs, spell);
}

void check_cast_expr_const(const struct cast_expr* e, struct value val) {
    check_cast_expr_empty(e);
    check_unary_expr_const(e->rhs, val);
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

void check_shift_expr_const(const struct shift_expr* e, struct value val) {
    check_shift_expr_empty(e);
    check_cast_expr_const(e->lhs->lhs->lhs, val);
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

void check_cond_expr_const(const struct cond_expr* e, struct value val) {
    check_cond_expr_empty(e);
    check_shift_expr_const(e->last_else->log_ands->or_exprs->xor_exprs
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

void check_const_expr_const(const struct const_expr* e, struct value val) {
    check_const_expr_empty(e);
    check_cond_expr_const(&e->expr, val);
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

void check_assign_expr_const(const struct assign_expr* e, struct value val) {
    check_assign_expr_empty(e);
    check_cond_expr_const(e->value, val);
}

static void check_expr_empty(const struct expr* e) {
    ASSERT_SIZE_T(e->len, (size_t)1);
    ASSERT_NOT_NULL(e->assign_exprs);
}

void check_expr_id(const struct expr* e, const char* spell) {
    check_expr_empty(e);
    check_assign_expr_id(&e->assign_exprs[0], spell);
}

void check_expr_const(const struct expr* e, struct value val) {
    check_expr_empty(e);
    check_assign_expr_const(&e->assign_exprs[0], val);
}

