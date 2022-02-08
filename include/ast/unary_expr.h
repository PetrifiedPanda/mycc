#ifndef UNARY_EXPR_H
#define UNARY_EXPR_H

#include <stddef.h>

#include "token_type.h"

#include "parser/parser_state.h"

struct postfix_expr;
struct cast_expr;
struct type_name;

enum unary_expr_type {
    UNARY_POSTFIX,
    UNARY_UNARY_OP,
    UNARY_SIZEOF_TYPE,
    UNARY_ALIGNOF_TYPE
};

struct unary_expr {
    size_t len;
    enum token_type* operators_before; // only SIZEOF INC_OP or DEC_OP
    enum unary_expr_type type;
    union {
        struct postfix_expr* postfix;
        struct {
            enum token_type unary_op;
            struct cast_expr* cast_expr;
        };
        struct type_name* type_name;
    };
};

struct unary_expr* parse_unary_expr(struct parser_state* s);

/**
 *
 * @param s current state
 * @param ops_before array of len tokens
 * @param len length of ops_before
 * @param type_name the already parsed type_name, with which this starts
 * @return struct unary_expr* unary expression created with the given parameters
 * NULL on fail This does not free any of the parameters
 */
struct unary_expr* parse_unary_expr_type_name(struct parser_state* s,
                                              enum token_type* ops_before,
                                              size_t len,
                                              struct type_name* type_name);

void free_unary_expr_children(struct unary_expr* u);
void free_unary_expr(struct unary_expr* u);

#include "ast/postfix_expr.h"
#include "ast/cast_expr.h"
#include "ast/type_name.h"

#endif
