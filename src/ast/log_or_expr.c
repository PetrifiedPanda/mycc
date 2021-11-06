#include "ast/log_or_expr.h"

#include <stdlib.h>
#include <assert.h>

LogOrExpr* create_log_or_expr(LogOrExpr* log_or, LogAndExpr* log_and) {
    assert(log_and);
    LogOrExpr* res = malloc(sizeof(LogOrExpr));
    if (res) {
        res->log_or = log_or;
        res->log_and = log_and;
    }
    return res;
}

static void free_children(LogOrExpr* e) {
    if (e->log_or != NULL) {
        free_log_or_expr(e->log_or);
    }
    free_log_and_expr(e->log_and);
}

void free_log_or_expr(LogOrExpr* e) {
    free_children(e);
    free(e);
}

