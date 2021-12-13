#ifndef PRIMARY_EXPR_H
#define PRIMARY_EXPR_H

#include "token_type.h"

#include "ast/constant.h"
#include "ast/string_literal.h"

struct expr;
struct identifier;

enum primary_expr_type {
    PRIMARY_EXPR_IDENTIFIER,
    PRIMARY_EXPR_CONSTANT,
    PRIMARY_EXPR_STRING_LITERAL,
    PRIMARY_EXPR_BRACKET
};

struct primary_expr {
    enum primary_expr_type type;
    union {
        struct constant constant;
        struct string_literal literal;
        struct identifier* identifier;
        struct expr* bracket_expr;
    };
};

struct primary_expr* create_primary_expr_constant(struct constant constant);

struct primary_expr* create_primary_expr_string(struct string_literal literal);

struct primary_expr* create_primary_expr_identifier(struct identifier* identifier);

struct primary_expr* create_primary_expr_bracket(struct expr* bracket_expr);

void free_primary_expr(struct primary_expr* bracket_expr);

#include "ast/expr.h"
#include "ast/identifier.h"

#endif

