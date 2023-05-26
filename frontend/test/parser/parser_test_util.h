#ifndef PARSER_TEST_UTIL_H
#define PARSER_TEST_UTIL_H

#include "frontend/parser/parser.h"

void check_value(Value got, Value expected);
void check_identifier(Identifier* id, const char* spell);

void check_primary_expr_id(const PrimaryExpr* e, const char* spell);
void check_primary_expr_val(const PrimaryExpr* e, Value val);

void check_postfix_expr_id(const PostfixExpr* e, const char* spell);
void check_postfix_expr_val(const PostfixExpr* e, Value val);

void check_unary_expr_id(const UnaryExpr* e, const char* spell);
void check_unary_expr_val(const UnaryExpr* e, Value val);

void check_cast_expr_id(const CastExpr* e, const char* spell);
void check_cast_expr_val(const CastExpr* e, Value val);

void check_shift_expr_id(const ShiftExpr* e, const char* spell);
void check_shift_expr_val(const ShiftExpr* e, Value val);

void check_cond_expr_id(const CondExpr* e, const char* spell);
void check_cond_expr_val(const CondExpr* e, Value val);

void check_const_expr_id(const ConstExpr* e, const char* spell);
void check_const_expr_val(const ConstExpr* e, Value val);

void check_assign_expr_id(const AssignExpr* e, const char* spell);
void check_assign_expr_val(const AssignExpr* e, Value val);

void check_expr_id(const Expr* e, const char* spell);
void check_expr_val(const Expr* e, Value val);

#endif
