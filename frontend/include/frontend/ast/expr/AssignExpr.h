#ifndef ASSIGN_EXPR_H
#define ASSIGN_EXPR_H

#include <stddef.h>

#include "frontend/parser/ParserState.h"

typedef struct UnaryExpr UnaryExpr;
typedef struct CondExpr CondExpr;

typedef enum {
    ASSIGN_EXPR_ASSIGN,
    ASSIGN_EXPR_MUL,
    ASSIGN_EXPR_DIV,
    ASSIGN_EXPR_MOD,
    ASSIGN_EXPR_ADD,
    ASSIGN_EXPR_SUB,
    ASSIGN_EXPR_LSHIFT,
    ASSIGN_EXPR_RSHIFT,
    ASSIGN_EXPR_AND,
    ASSIGN_EXPR_XOR,
    ASSIGN_EXPR_OR,
} AssignExprOp;

typedef struct {
    UnaryExpr* unary;
    AssignExprOp op;
} UnaryAndOp;

typedef struct AssignExpr {
    size_t len;
    UnaryAndOp* assign_chain;
    CondExpr* value;
} AssignExpr;

bool parse_assign_expr_inplace(ParserState* s, AssignExpr* res);
AssignExpr* parse_assign_expr(ParserState* s);

void free_assign_expr_children(AssignExpr* e);
void free_assign_expr(AssignExpr* e);

#include "CondExpr.h"
#include "UnaryExpr.h"

#endif

