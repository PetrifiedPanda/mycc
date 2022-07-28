#include "parser_test_util.h"

#include "testing/asserts.h"

#include "../test_helpers.h"

void check_identifier(struct identifier* id, const char* spell) {
    if (spell != NULL) {
        ASSERT_STR(id->spelling, spell);
    } else {
        ASSERT_NULL(id);
    }
}

void check_primary_expr_id_or_const(struct primary_expr* e,
                                    const char* spell,
                                    enum token_type type) {
    enum primary_expr_type expected_type = type == IDENTIFIER
                                               ? PRIMARY_EXPR_IDENTIFIER
                                               : PRIMARY_EXPR_CONSTANT;
    ASSERT(e->type == expected_type);
    if (type == IDENTIFIER) {
        ASSERT_NOT_NULL(e->identifier);
        ASSERT_STR(e->identifier->spelling, spell);
    } else {
        ASSERT_TOKEN_TYPE(e->constant.type, type);
        ASSERT_STR(e->constant.spelling, spell);
    }
}

void check_postfix_expr_id_or_const(struct postfix_expr* e,
                                    const char* spell,
                                    enum token_type type) {
    ASSERT(e->is_primary);
    check_primary_expr_id_or_const(e->primary, spell, type);
}

void check_unary_expr_id_or_const(struct unary_expr* unary,
                                  const char* spell,
                                  enum token_type type) {
    ASSERT_SIZE_T(unary->len, (size_t)0);
    ASSERT_NULL(unary->operators_before);

    ASSERT(unary->type == UNARY_POSTFIX);
    check_postfix_expr_id_or_const(unary->postfix, spell, type);
}

void check_cast_expr_id_or_const(struct cast_expr* expr,
                                 const char* spell,
                                 enum token_type type) {
    ASSERT_SIZE_T(expr->len, (size_t)0);
    ASSERT_NULL(expr->type_names);

    check_unary_expr_id_or_const(expr->rhs, spell, type);
}

void check_shift_expr_id_or_const(struct shift_expr* expr,
                                  const char* spell,
                                  enum token_type type) {
    const size_t zero = (size_t)0;
    ASSERT_SIZE_T(expr->len, (size_t)0);
    ASSERT_NOT_NULL(expr->lhs);
    ASSERT_SIZE_T(expr->lhs->len, zero);
    ASSERT_NOT_NULL(expr->lhs->lhs);
    ASSERT_SIZE_T(expr->lhs->lhs->len, zero);
    ASSERT_NOT_NULL(expr->lhs->lhs->lhs);
    check_cast_expr_id_or_const(expr->lhs->lhs->lhs, spell, type);
}

void check_cond_expr_id_or_const(struct cond_expr* expr,
                                 const char* spell,
                                 enum token_type type) {
    const size_t one = (size_t)1;
    const size_t zero = (size_t)0;
    ASSERT_SIZE_T(expr->len, zero);
    ASSERT_SIZE_T(expr->last_else->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->len, one);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->len, one);
    ASSERT_SIZE_T(
        expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs->len,
        one);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                      ->eq_exprs->len,
                  zero);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                      ->eq_exprs->lhs->len,
                  zero);
    ASSERT_SIZE_T(expr->last_else->log_ands->or_exprs->xor_exprs->and_exprs
                      ->eq_exprs->lhs->lhs->len,
                  zero);
    check_shift_expr_id_or_const(expr->last_else->log_ands->or_exprs->xor_exprs
                                     ->and_exprs->eq_exprs->lhs->lhs,
                                 spell,
                                 type);
}

void check_const_expr_id_or_const(struct const_expr* expr,
                                  const char* spell,
                                  enum token_type type) {
    ASSERT_SIZE_T(expr->expr.len, (size_t)0);
    ASSERT_NOT_NULL(expr->expr.last_else);
    check_cond_expr_id_or_const(&expr->expr, spell, type);
}

void check_assign_expr_id_or_const(struct assign_expr* expr,
                                   const char* spell,
                                   enum token_type type) {
    ASSERT_SIZE_T(expr->len, (size_t)0);
    ASSERT_NULL(expr->assign_chain);
    ASSERT_NOT_NULL(expr->value);
    check_cond_expr_id_or_const(expr->value, spell, type);
}

void check_expr_id_or_const(struct expr* expr,
                            const char* spell,
                            enum token_type type) {
    ASSERT_SIZE_T(expr->len, (size_t)1);
    ASSERT_NOT_NULL(expr->assign_exprs);
    check_assign_expr_id_or_const(&expr->assign_exprs[0], spell, type);
}
