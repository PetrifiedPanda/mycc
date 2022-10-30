#ifndef PARSER_TEST_UTIL_H
#define PARSER_TEST_UTIL_H

#include "frontend/parser/parser.h"

void check_int_value(struct int_value got, struct int_value expected);
void check_float_value(struct float_value got, struct float_value expected);
void check_identifier(struct identifier* id, const char* spell);

void check_primary_expr_id(const struct primary_expr* e, const char* spell);
void check_primary_expr_int(const struct primary_expr* e, struct int_value val);
void check_primary_expr_float(const struct primary_expr* e,
                              struct float_value val);

void check_postfix_expr_id(const struct postfix_expr* e, const char* spell);
void check_postfix_expr_int(const struct postfix_expr* e, struct int_value val);
void check_postfix_expr_float(const struct postfix_expr* e,
                              struct float_value val);

void check_unary_expr_id(const struct unary_expr* e, const char* spell);
void check_unary_expr_int(const struct unary_expr* e, struct int_value val);
void check_unary_expr_float(const struct unary_expr* e, struct float_value val);

void check_cast_expr_id(const struct cast_expr* e, const char* spell);
void check_cast_expr_int(const struct cast_expr* e, struct int_value val);
void check_cast_expr_float(const struct cast_expr* e, struct float_value val);

void check_shift_expr_id(const struct shift_expr* e, const char* spell);
void check_shift_expr_int(const struct shift_expr* e, struct int_value val);
void check_shift_expr_float(const struct shift_expr* e, struct float_value val);

void check_cond_expr_id(const struct cond_expr* e, const char* spell);
void check_cond_expr_int(const struct cond_expr* e, struct int_value val);
void check_cond_expr_float(const struct cond_expr* e, struct float_value val);

void check_const_expr_id(const struct const_expr* e, const char* spell);
void check_const_expr_int(const struct const_expr* e, struct int_value val);
void check_const_expr_float(const struct const_expr* e, struct float_value val);

void check_assign_expr_id(const struct assign_expr* e, const char* spell);
void check_assign_expr_int(const struct assign_expr* e, struct int_value val);
void check_assign_expr_float(const struct assign_expr* e,
                             struct float_value val);

void check_expr_id(const struct expr* e, const char* spell);
void check_expr_int(const struct expr* e, struct int_value val);
void check_expr_float(const struct expr* e, struct float_value val);

#endif
