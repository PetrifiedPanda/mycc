#ifndef AND_EXPR_H
#define AND_EXPR_H

#include <stddef.h>

typedef struct EqExpr EqExpr;

typedef struct AndExpr {
    EqExpr* eq_exprs;
    size_t len;
} AndExpr;

AndExpr* create_and_expr(EqExpr* eq_exprs, size_t len);

void free_and_expr_children(AndExpr* e);

void free_and_expr(AndExpr* e);

#include "ast/eq_expr.h"

#endif