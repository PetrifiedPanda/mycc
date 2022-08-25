#ifndef ASSIGN_EXPR_H
#define ASSIGN_EXPR_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

struct unary_expr;
struct cond_expr;

enum assign_expr_op {
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
};

struct unary_and_op {
    struct unary_expr* unary;
    enum assign_expr_op op;
};

struct assign_expr {
    size_t len;
    struct unary_and_op* assign_chain;
    struct cond_expr* value;
};

bool parse_assign_expr_inplace(struct parser_state* s, struct assign_expr* res);
struct assign_expr* parse_assign_expr(struct parser_state* s);

void free_assign_expr_children(struct assign_expr* e);
void free_assign_expr(struct assign_expr* e);

#include "cond_expr.h"
#include "unary_expr.h"

#endif

