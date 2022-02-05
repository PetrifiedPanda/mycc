#ifndef PARSER_TEST_UTIL_H
#define PARSER_TEST_UTIL_H

#include "parser/parser.h"

void check_identifier(struct identifier* id, const char* spell);

void check_primary_expr_id_or_const(struct primary_expr* e, const char* spell, enum token_type type);
void check_postfix_expr_id_or_const(struct postfix_expr* e, const char* spell, enum token_type type);
void check_unary_expr_id_or_const(struct unary_expr* unary, const char* spell, enum token_type type);
void check_cast_expr_id_or_const(struct cast_expr* expr, const char* spell, enum token_type type);
void check_shift_expr_id_or_const(struct shift_expr* expr, const char* spell, enum token_type type);
void check_cond_expr_id_or_const(struct cond_expr* expr, const char* spell, enum token_type type);
void check_const_expr_id_or_const(struct const_expr* expr, const char* spell, enum token_type type);
void check_assign_expr_id_or_const(struct assign_expr* expr, const char* spell, enum token_type type);
void check_expr_id_or_const(struct expr* expr, const char* spell, enum token_type type);

#endif
