#ifndef PARSER_TEST_UTIL_H
#define PARSER_TEST_UTIL_H

#include "frontend/parser/parser.h"

void check_int_value(IntValue got, IntValue expected);
void check_float_value(FloatValue got, FloatValue expected);
void check_identifier(Identifier* id, const char* spell);

void check_primary_expr_id(const PrimaryExpr* e, const char* spell);
void check_primary_expr_int(const PrimaryExpr* e, IntValue val);
void check_primary_expr_float(const PrimaryExpr* e, FloatValue val);

void check_postfix_expr_id(const PostfixExpr* e, const char* spell);
void check_postfix_expr_int(const PostfixExpr* e, IntValue val);
void check_postfix_expr_float(const PostfixExpr* e, FloatValue val);

void check_unary_expr_id(const UnaryExpr* e, const char* spell);
void check_unary_expr_int(const UnaryExpr* e, IntValue val);
void check_unary_expr_float(const UnaryExpr* e, FloatValue val);

void check_cast_expr_id(const CastExpr* e, const char* spell);
void check_cast_expr_int(const CastExpr* e, IntValue val);
void check_cast_expr_float(const CastExpr* e, FloatValue val);

void check_shift_expr_id(const ShiftExpr* e, const char* spell);
void check_shift_expr_int(const ShiftExpr* e, IntValue val);
void check_shift_expr_float(const ShiftExpr* e, FloatValue val);

void check_cond_expr_id(const CondExpr* e, const char* spell);
void check_cond_expr_int(const CondExpr* e, IntValue val);
void check_cond_expr_float(const CondExpr* e, FloatValue val);

void check_const_expr_id(const ConstExpr* e, const char* spell);
void check_const_expr_int(const ConstExpr* e, IntValue val);
void check_const_expr_float(const ConstExpr* e, FloatValue val);

void check_assign_expr_id(const AssignExpr* e, const char* spell);
void check_assign_expr_int(const AssignExpr* e, IntValue val);
void check_assign_expr_float(const AssignExpr* e, FloatValue val);

void check_expr_id(const Expr* e, const char* spell);
void check_expr_int(const Expr* e, IntValue val);
void check_expr_float(const Expr* e, FloatValue val);

#endif
