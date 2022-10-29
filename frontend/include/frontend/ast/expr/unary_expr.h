#ifndef UNARY_EXPR_H
#define UNARY_EXPR_H

#include <stddef.h>

#include "frontend/parser/parser_state.h"

#include "frontend/ast/ast_node_info.h"

struct postfix_expr;
struct cast_expr;
struct type_name;

enum unary_expr_type {
    UNARY_POSTFIX,
    UNARY_ADDRESSOF,
    UNARY_DEREF,
    UNARY_PLUS,
    UNARY_MINUS,
    UNARY_BNOT,
    UNARY_NOT,
    UNARY_SIZEOF_TYPE,
    UNARY_ALIGNOF,
};

enum unary_expr_op {
    UNARY_OP_INC,
    UNARY_OP_DEC,
    UNARY_OP_SIZEOF,
};

struct unary_expr {
    struct ast_node_info info;
    size_t len;
    enum unary_expr_op* ops_before; 
    enum unary_expr_type type;
    union {
        struct postfix_expr* postfix;
        struct cast_expr* cast_expr;
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
 * @param start_bracket_loc location of the bracket before the type name
 *
 * @return struct unary_expr* unary expression created with the given parameters
 *         NULL on fail This does not free any of the parameters
 */
struct unary_expr* parse_unary_expr_type_name(struct parser_state* s,
                                              enum unary_expr_op* ops_before,
                                              size_t len,
                                              struct type_name* type_name,
                                              struct source_loc start_bracket_loc);

void free_unary_expr_children(struct unary_expr* u);
void free_unary_expr(struct unary_expr* u);

#include "frontend/ast/type_name.h"

#include "postfix_expr.h"
#include "cast_expr.h"

#endif

