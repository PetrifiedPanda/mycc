#ifndef MYCC_FRONTEND_AST_EXPR_H
#define MYCC_FRONTEND_AST_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

#include "frontend/ast/AstNodeInfo.h"
#include "frontend/ast/StringLiteralNode.h"

typedef struct AssignExpr AssignExpr;

typedef struct Expr {
    uint32_t len;
    AssignExpr* assign_exprs;
} Expr;

bool parse_expr_inplace(ParserState* s, Expr* res);

void Expr_free_children(Expr* expr);

#endif

