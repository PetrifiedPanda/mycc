#ifndef PRIMARY_EXPR_H
#define PRIMARY_EXPR_H

#include "frontend/token_type.h"

#include "constant.h"
#include "string_constant.h"

#include "frontend/parser/parser_state.h"

struct expr;
struct identifier;
struct generic_sel;

enum primary_expr_type {
    PRIMARY_EXPR_IDENTIFIER,
    PRIMARY_EXPR_CONSTANT,
    PRIMARY_EXPR_STRING_LITERAL,
    PRIMARY_EXPR_BRACKET,
    PRIMARY_EXPR_GENERIC
};

struct primary_expr {
    enum primary_expr_type type;
    union {
        struct constant constant;
        struct string_constant string;
        struct identifier* identifier;
        struct expr* bracket_expr;
        struct generic_sel* generic;
    };
};

struct primary_expr* parse_primary_expr(struct parser_state* s);

void free_primary_expr(struct primary_expr* bracket_expr);

#include "expr.h"
#include "generic_sel.h"

#include "frontend/ast/identifier.h"

#endif
