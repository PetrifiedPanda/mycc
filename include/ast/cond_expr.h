#ifndef COND_EXPR_H
#define COND_EXPR_H

#include <stddef.h>

#include "parser/parser_state.h"

struct log_or_expr;
struct expr;

struct log_or_and_expr {
    struct log_or_expr* log_or;
    struct expr* expr;
};

struct cond_expr {
    size_t len;
    struct log_or_and_expr* conditionals;
    struct log_or_expr* last_else;
};

bool parse_cond_expr_inplace(struct parser_state* s, struct cond_expr* res);

struct cast_expr;

struct cond_expr* parse_cond_expr_cast(struct parser_state* s, struct cast_expr* start);

void free_cond_expr_children(struct cond_expr* e);
void free_cond_expr(struct cond_expr* e);

#include "ast/expr.h"
#include "ast/log_or_expr.h"
#include "ast/cast_expr.h"

#endif

