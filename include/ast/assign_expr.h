#ifndef ASSIGN_EXPR_H
#define ASSIGN_EXPR_H

#include <stddef.h>

#include "token_type.h"

struct unary_expr;
struct cond_expr;

struct unary_and_op {
    struct unary_expr* unary;
    enum token_type assign_op;
};

struct assign_expr {
    size_t len;
    struct unary_and_op* assign_chain;
    struct cond_expr* value;
};

void init_assign_expr(struct assign_expr* res, struct unary_and_op* assign_chain, size_t len, struct cond_expr* value);
struct assign_expr* create_assign_expr(struct unary_and_op* assign_chain, size_t len, struct cond_expr* value);

void free_assign_expr_children(struct assign_expr* e);
void free_assign_expr(struct assign_expr* e);

#include "ast/cond_expr.h"
#include "ast/unary_expr.h"

#endif

