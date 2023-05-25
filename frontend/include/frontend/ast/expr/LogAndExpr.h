#ifndef LOG_AND_EXPR_H
#define LOG_AND_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct OrExpr OrExpr;

typedef struct LogAndExpr {
    size_t len;
    OrExpr* or_exprs;
} LogAndExpr;

bool parse_log_and_expr_inplace(ParserState* s, LogAndExpr* res);

typedef struct CastExpr CastExpr;

LogAndExpr* parse_log_and_expr_cast(ParserState* s, CastExpr* start);

void free_log_and_expr_children(LogAndExpr* e);

#include "OrExpr.h"
#include "CastExpr.h"

#endif
