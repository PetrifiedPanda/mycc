#ifndef LOG_OR_EXPR_H
#define LOG_OR_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct LogAndExpr LogAndExpr;

typedef struct LogOrExpr {
    size_t len;
    LogAndExpr* log_ands;
} LogOrExpr;

LogOrExpr* parse_log_or_expr(ParserState* s);

typedef struct CastExpr CastExpr;

LogOrExpr* parse_log_or_expr_cast(ParserState* s, CastExpr* start);

void LogOrExpr_free(LogOrExpr* e);

#include "LogAndExpr.h"
#include "CastExpr.h"

#endif

