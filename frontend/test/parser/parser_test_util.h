#ifndef PARSER_TEST_UTIL_H
#define PARSER_TEST_UTIL_H

#include "frontend/parser/parser.h"

void check_value(struct value got, struct value expected);
void check_identifier(struct identifier* id, const char* spell);

void check_primary_expr_id(const struct primary_expr* e, const char* spell);
void check_primary_expr_const(const struct primary_expr* e, struct value val);

void check_postfix_expr_id(const struct postfix_expr* e, const char* spell);
void check_postfix_expr_const(const struct postfix_expr* e, struct value val);

void check_unary_expr_id(const struct unary_expr* e, const char* spell);
void check_unary_expr_const(const struct unary_expr* e, struct value val);

void check_cast_expr_id(const struct cast_expr* e, const char* spell);
void check_cast_expr_const(const struct cast_expr* e, struct value val);

void check_shift_expr_id(const struct shift_expr* e, const char* spell);
void check_shift_expr_const(const struct shift_expr* e, struct value val);

void check_cond_expr_id(const struct cond_expr* e, const char* spell);
void check_cond_expr_const(const struct cond_expr* e, struct value val);

void check_const_expr_id(const struct const_expr* e, const char* spell);
void check_const_expr_const(const struct const_expr* e, struct value val);

void check_assign_expr_id(const struct assign_expr* e, const char* spell);
void check_assign_expr_const(const struct assign_expr* e, struct value val);

void check_expr_id(const struct expr* e, const char* spell);
void check_expr_const(const struct expr* e, struct value val);

#endif
