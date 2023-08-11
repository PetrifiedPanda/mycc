#ifndef PARSER_TEST_UTIL_H
#define PARSER_TEST_UTIL_H

#include "frontend/parser/parser.h"

void check_value(Value got, Value expected);
void check_identifier(const Identifier* id, Str spell, const TokenArr* arr);

void check_primary_expr_id(const PrimaryExpr* e, Str spell, const TokenArr* arr);
void check_primary_expr_val(const PrimaryExpr* e, Value val, const TokenArr* arr);

void check_postfix_expr_id(const PostfixExpr* e, Str spell, const TokenArr* arr);
void check_postfix_expr_val(const PostfixExpr* e, Value val, const TokenArr* arr);

void check_unary_expr_id(const UnaryExpr* e, Str spell, const TokenArr* arr);
void check_unary_expr_val(const UnaryExpr* e, Value val, const TokenArr* arr);

void check_cast_expr_id(const CastExpr* e, Str spell, const TokenArr* arr);
void check_cast_expr_val(const CastExpr* e, Value val, const TokenArr* arr);

void check_shift_expr_id(const ShiftExpr* e, Str spell, const TokenArr* arr);
void check_shift_expr_val(const ShiftExpr* e, Value val, const TokenArr* arr);

void check_cond_expr_id(const CondExpr* e, Str spell, const TokenArr* arr);
void check_cond_expr_val(const CondExpr* e, Value val, const TokenArr* arr);

void check_const_expr_id(const ConstExpr* e, Str spell, const TokenArr* arr);
void check_const_expr_val(const ConstExpr* e, Value val, const TokenArr* arr);

void check_assign_expr_id(const AssignExpr* e, Str spell, const TokenArr* arr);
void check_assign_expr_val(const AssignExpr* e, Value val, const TokenArr* arr);

void check_expr_id(const Expr* e, Str spell, const TokenArr* arr);
void check_expr_val(const Expr* e, Value val, const TokenArr* arr);

#endif
