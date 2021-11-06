#ifndef LOG_OR_EXPR_H
#define LOG_OR_EXPR_H

typedef struct LogAndExpr LogAndExpr;

typedef struct LogOrExpr {
    struct LogOrExpr* log_or;
    LogAndExpr* log_and;
} LogOrExpr;

LogOrExpr* create_log_or_expr(LogOrExpr* log_or, LogAndExpr* log_and);

void free_log_or_expr(LogOrExpr* e);

#include "ast/log_and_expr.h"

#endif

