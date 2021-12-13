#ifndef EQ_EXPR_H
#define EQ_EXPR_H

#include <stddef.h>

#include "token_type.h"

struct rel_expr;

struct rel_expr_and_op { 
    enum token_type eq_op;
    struct rel_expr* rhs;
};

struct eq_expr {
    struct rel_expr* lhs;
    size_t len;
    struct rel_expr_and_op* eq_chain; 
};

void init_eq_expr(struct eq_expr* res, struct rel_expr* lhs, struct rel_expr_and_op* eq_chain, size_t len);

struct eq_expr* create_eq_expr(struct rel_expr* lhs, struct rel_expr_and_op* eq_chain, size_t len);

void free_eq_expr_children(struct eq_expr* e);

void free_eq_expr(struct eq_expr* e);

#include "ast/rel_expr.h"

#endif

