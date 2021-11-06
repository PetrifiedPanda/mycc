#ifndef LOG_AND_EXPR_H
#define LOG_AND_EXPR_H

#include <stddef.h>

typedef struct OrExpr OrExpr;

typedef struct LogAndExpr {
    size_t len;
    OrExpr* or_exprs;
} LogAndExpr;

LogAndExpr* create_log_and_expr(OrExpr* or_exprs, size_t len);

void free_log_and_expr(LogAndExpr* e);

#include "ast/or_expr.h"

#endif

